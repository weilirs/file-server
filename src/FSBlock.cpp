#include "FSBlock.h"
#include <cstring>

Block::operator DirentBlock &(void)
{
    return *(DirentBlock *)(this);
}

Block::operator Inode &(void)
{
    return *(Inode *)(this);
}

void Dirent::initialize(const std::string_view &_name, uint32_t _block)
{
    _name.copy(name, FS_MAXFILENAME);
    name[_name.length()] = '\0';
    inode_block = _block;
}

bool Dirent::valid(void) const
{
    return inode_block != 0;
}

BlockIterator::BlockIterator(uint32_t _block_num) : blockIndex(_block_num)
{
}

void BlockIterator::load(void)
{
    disk_readblock(blockIndex, &block);
}

void BlockIterator::commit(void)
{
    disk_writeblock(blockIndex, &block);
}

void BlockIterator::copy(const BlockIterator &iter)
{
    blockIndex = iter.blockIndex;
    std::memcpy(block.data(), iter.block.data(), FS_BLOCKSIZE);
}

void DirentBlock::initialize(void)
{
    for (size_t i = 0; i < size(); ++i)
    {
        at(i).inode_block = 0;
    }
}

std::pair<size_t, bool> DirentBlock::find(const std::string_view &entry) const
{
    std::pair<size_t, bool> result{FS_DIRENTRIES, false};
    size_t vacancy = FS_DIRENTRIES;
    for (size_t i = 0; i < FS_DIRENTRIES && !result.second; i++)
    {
        const Dirent &dirent = at(i);
        if (!dirent.valid())
        {
            vacancy = (vacancy == FS_DIRENTRIES) ? (i) : (vacancy);
        }
        else if (entry == dirent.name)
        {
            result.first = i;
            result.second = true;
        }
    }
    result.first = result.second ? result.first : vacancy;
    return result;
}

size_t DirentBlock::validEntries(void) const
{
    size_t result = 0;
    for (size_t i = 0; i < FS_DIRENTRIES; i++)
    {
        if (at(i).valid())
        {
            result++;
        }
    }
    return result;
}

SearchResult::SearchResult() : direntBlockIndex(FS_MAXFILEBLOCKS), direntIndex(FS_DIRENTRIES), found(false) {}

bool SearchResult::valid(void) const
{
    return found || direntIndex < FS_DIRENTRIES;
}

Inode::Inode() : fs_inode()
{
    type = 'f';
    owner[0] = '\0';
    size = 0;
}

void Inode::initialize(const std::string_view &user, char _type)
{
    type = _type;
    user.copy(owner, FS_MAXUSERNAME);
    owner[user.length()] = '\0';
    size = 0;
}

SearchResult Inode::find(BlockIterator &direntCache, const std::string_view &entry) const
{
    if (type != 'd')
    {
        throw std::runtime_error("permission denied");
    }
    SearchResult result;
    bool vacancyFound = false;
    for (size_t i = 0; i < size && !result.found; ++i)
    {
        BlockIterator iterator(blocks[i]);
        iterator.load();
        std::pair<size_t, bool> temp = iterator.block.as<DirentBlock>().find(entry);
        if ((!vacancyFound && !temp.second && temp.first != FS_DIRENTRIES) || temp.second)
        {
            result.direntBlockIndex = i;
            result.direntIndex = temp.first;
            result.found = temp.second;
            direntCache.copy(iterator);
            vacancyFound = !temp.second;
        }
    }
    return result;
}

bool Inode::verify(const std::string_view &user) const
{
    if (owner[0] != '\0' && user != owner)
    {
        throw std::runtime_error("permission denied");
    }
    return true;
}

bool Inode::verify(const std::string_view &user, char _type) const
{
    verify(user);
    if (type != _type)
    {
        throw std::runtime_error("permission denied");
    }
    return true;
}
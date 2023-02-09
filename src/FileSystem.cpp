#include "FileSystem.h"
#include <deque>
#include <utility>

LockMap::LockMap() : locks(), lock_for_lock() {}

std::shared_mutex &LockMap::operator[](size_t index)
{
    std::unique_lock<std::mutex> lock_for_lock_wrap(lock_for_lock);
    return locks[index];
}

void LockMap::erase(uint32_t index)
{
    std::unique_lock<std::mutex> lock_for_lock_wrap(lock_for_lock);
    locks.erase(index);
}

std::pair<std::unordered_map<uint32_t, std::shared_mutex>::iterator, bool> LockMap::try_emplace(uint32_t index)
{
    std::unique_lock<std::mutex> lock_for_lock_wrap(lock_for_lock);
    return locks.try_emplace(index);
}

template <typename Lock>
FileSystem::Iterator<Lock>::Iterator(LockMap &_locks) : locks(_locks), lock(), inode(), dataBlock(), result()
{
}

template <typename Lock>
void FileSystem::Iterator<Lock>::_load_and_check(const std::string_view &user, char type)
{
    inode.load();
    Inode &inode_block = inode.block;
    inode_block.verify(user, type);
}

template <typename Lock>
void FileSystem::Iterator<Lock>::_step(const std::string_view &user, const std::string_view &pathToken)
{
    _load_and_check(user, 'd');
    result = inode.block.as<Inode>().find(dataBlock, pathToken);
    if (!result.found)
    {
        throw std::runtime_error("directory not found");
    }
    inode.blockIndex = dataBlock.block.as<DirentBlock>().at(result.direntIndex).inode_block;
}

template <typename Lock>
void FileSystem::Iterator<Lock>::step(std::shared_lock<std::shared_mutex> &pathLock, const std::string_view &user, const std::string_view &pathToken)
{
    _step(user, pathToken);
    std::shared_lock<std::shared_mutex> nextPathLock(locks[inode.blockIndex]);
    std::swap(pathLock, nextPathLock);
}

template <typename Lock>
void FileSystem::Iterator<Lock>::locate(const std::string_view &user, const std::vector<Request::SafePathToken> &pathTokens, bool dir)
{
    size_t num_steps = dir ? (pathTokens.size() - 1) : (pathTokens.size());
    if (num_steps > 0)
    {
        std::shared_lock<std::shared_mutex> pathLock(locks[0]);
        inode.blockIndex = 0;
        for (size_t i = 0; i + 1 < num_steps; i++)
        {
            step(pathLock, user, pathTokens[i]);
        }
        _step(user, pathTokens[num_steps - 1]);
        Lock dummy(locks[inode.blockIndex]);
        std::swap(lock, dummy);
    }
    else
    {
        // don't change this, it is because of scope
        Lock dummy(locks[inode.blockIndex]);
        std::swap(lock, dummy);
    }
    _load_and_check(user, dir ? ('d') : ('f'));
    Inode &final_inode = inode.block;
    if (dir)
    {
        result = final_inode.find(dataBlock, pathTokens.back());
    }
}

std::unordered_set<uint32_t> FileSystem::traverseInitialize(void)
{
    std::unordered_set<uint32_t> usedBlocks;
    std::deque<uint32_t> traversalContainer;
    traversalContainer.push_back(0);
    while (!traversalContainer.empty())
    {
        BlockIterator iter(traversalContainer.front());
        traversalContainer.pop_front();
        usedBlocks.insert(iter.blockIndex);
        locks.try_emplace(iter.blockIndex);
        iter.load();
        Inode &cursor = iter.block;
        for (size_t i = 0; i < cursor.size; i++)
        {
            usedBlocks.insert(cursor.blocks[i]);
            if (cursor.type == 'd')
            {
                BlockIterator direntBlock(cursor.blocks[i]);
                direntBlock.load();
                DirentBlock &direntries = direntBlock.block;
                for (size_t j = 0; j < FS_DIRENTRIES; j++)
                {
                    if (direntries[j].valid())
                        traversalContainer.push_back(direntries[j].inode_block);
                }
            }
        }
    }
    return usedBlocks;
}

FileSystem::FileSystem() : locks(), freeBlocks(FS_DISKSIZE, traverseInitialize()) {}

void FileSystem::createEntry(const Request &request)
{
    Iterator<std::unique_lock<std::shared_mutex>> parent_dir(locks);
    parent_dir.locate(request.owner, request.pathTokens, true);
    Inode &parent_dir_inode = parent_dir.inode.block;
    bool parent_dir_dirty = false;
    BlockResource potential_new_block;
    if (!parent_dir.result.valid() && parent_dir_inode.size < FS_MAXFILEBLOCKS)
    {
        // create new dirent block
        BlockResource new_dirent(freeBlocks);
        parent_dir.dataBlock.blockIndex = new_dirent;
        std::swap(potential_new_block, new_dirent);
        parent_dir.dataBlock.block.as<DirentBlock>().initialize();
        parent_dir_inode.blocks[parent_dir_inode.size] = parent_dir.dataBlock.blockIndex;
        parent_dir.result.direntBlockIndex = parent_dir_inode.size++;
        parent_dir.result.direntIndex = 0;
        parent_dir_dirty = true;
    }
    else if (parent_dir.result.found || !parent_dir.result.valid())
    {
        throw std::runtime_error("cannot create");
    }
    // create new inode
    BlockResource new_inode(freeBlocks);
    BlockIterator createdInode(new_inode);
    createdInode.block.as<Inode>().initialize(request.owner, request.type.get());
    createdInode.commit();
    locks.try_emplace(createdInode.blockIndex);
    // commit data block
    parent_dir.dataBlock.block.as<DirentBlock>()
        .at(parent_dir.result.direntIndex)
        .initialize(request.pathTokens.back(), createdInode.blockIndex);
    parent_dir.dataBlock.commit();
    // commit inode if modified
    if (parent_dir_dirty)
    {
        parent_dir.inode.commit();
    }
    potential_new_block.confirm();
    new_inode.confirm();
}

void FileSystem::deleteEntry(const Request &request)
{
    Iterator<std::unique_lock<std::shared_mutex>> parent_dir(locks);
    parent_dir.locate(request.owner, request.pathTokens, true);
    if (!parent_dir.result.found)
    {
        throw std::runtime_error("target not found");
    }
    Inode &parent_dir_inode = parent_dir.inode.block;
    DirentBlock &parent_dirent = parent_dir.dataBlock.block;
    uint32_t target_inode = parent_dirent.at(parent_dir.result.direntIndex).inode_block;
    {
        std::unique_lock<std::shared_mutex> _(locks[target_inode]);
    }
    BlockIterator target(target_inode);
    target.load();
    Inode &target_inode_block = target.block;
    target_inode_block.verify(request.owner);
    if (target_inode_block.type == 'd' && target_inode_block.size)
    {
        throw std::runtime_error("cannot delete");
    }
    parent_dirent.at(parent_dir.result.direntIndex).inode_block = 0;
    if (parent_dirent.validEntries() == 0)
    {
        for (size_t i = parent_dir.result.direntBlockIndex; i + 1 < parent_dir_inode.size; ++i)
        {
            parent_dir_inode.blocks[i] = parent_dir_inode.blocks[i + 1];
        }
        --parent_dir_inode.size;
        parent_dir.inode.commit();
        freeBlocks.deallocate(parent_dir.dataBlock.blockIndex);
    }
    else
    {
        parent_dir.dataBlock.commit();
    }
    {
        std::unique_lock<std::shared_mutex> dummy;
        std::swap(parent_dir.lock, dummy);
    }
    freeBlocks.deallocate(target.blockIndex);
    locks.erase(target.blockIndex);
    for (size_t i = 0; i < target_inode_block.size && target_inode_block.type == 'f'; i++)
    {
        freeBlocks.deallocate(target_inode_block.blocks[i]);
    }
}

void FileSystem::readFileBlock(const Request &request, std::unique_ptr<BlockData> &ptr)
{
    Iterator<std::shared_lock<std::shared_mutex>> file(locks);
    file.locate(request.owner, request.pathTokens);
    const Inode &file_inode = file.inode.block;
    if (request.block.get() >= file_inode.size)
    {
        throw std::runtime_error("reading invalid block");
    }
    disk_readblock(file_inode.blocks[request.block.get()], ptr.get());
}

void FileSystem::writeFileBlock(const Request &request, const std::unique_ptr<BlockData> &ptr)
{
    Iterator<std::unique_lock<std::shared_mutex>> file(locks);
    file.locate(request.owner, request.pathTokens);
    Inode &file_inode = file.inode.block;
    bool file_inode_dirty = false;
    BlockResource potential_new_block;
    if (request.block.get() > file_inode.size)
    {
        throw std::runtime_error("cannot write this block");
    }
    else if (request.block.get() == file_inode.size)
    {
        // create a new data block
        BlockResource new_block(freeBlocks);
        file_inode.blocks[file_inode.size++] = new_block;
        std::swap(potential_new_block, new_block);
        file_inode_dirty = true;
    }
    disk_writeblock(file_inode.blocks[request.block.get()], ptr.get());
    if (file_inode_dirty)
    {
        file.inode.commit();
    }
    potential_new_block.confirm();
}

template class FileSystem::Iterator<std::shared_lock<std::shared_mutex>>;
template class FileSystem::Iterator<std::unique_lock<std::shared_mutex>>;
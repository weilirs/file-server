#pragma once

#include "fs_server.h"
#include <array>

using BlockData = std::array<char, FS_BLOCKSIZE>;

class DirentBlock;
class Inode;

class Block : public BlockData
{
public:
    template <typename T>
    T &as(void)
    {
        return *((T *)(this));
    }

    operator DirentBlock &(void);
    operator Inode &(void);
};

class BlockIterator
{
public:
    Block block;
    uint32_t blockIndex;

    BlockIterator(uint32_t _block_num = 0);
    void load(void);
    void commit(void);
    void copy(const BlockIterator &iter);
};

class Dirent : public fs_direntry
{
public:
    void initialize(const std::string_view &_name, uint32_t _block);
    bool valid(void) const;
};

using DirentBlockData = std::array<Dirent, FS_DIRENTRIES>;

class DirentBlock : public DirentBlockData
{
public:
    void initialize(void);
    // returns vacancy position if not found
    std::pair<size_t, bool> find(const std::string_view &entry) const;
    size_t validEntries(void) const;
};

class SearchResult
{
public:
    size_t direntBlockIndex;
    size_t direntIndex;
    bool found;

    SearchResult();
    // is that block cache valid
    bool valid(void) const;
};

class Inode : public fs_inode
{
public:
    Inode();
    void initialize(const std::string_view &user, char _type);
    // returns vacancy position if not found
    SearchResult find(BlockIterator &direntCache, const std::string_view &entry) const;
    bool verify(const std::string_view &user) const;
    bool verify(const std::string_view &user, char _type) const;
};
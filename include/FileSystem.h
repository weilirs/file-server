#pragma once

#include "fs_server.h"
#include "FSBlock.h"
#include "BlockAllocator.h"
#include "Request.h"
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

class LockMap
{
protected:
    std::unordered_map<uint32_t, std::shared_mutex> locks;
    std::mutex lock_for_lock;

public:
    LockMap();
    // make hash map thread safe
    std::shared_mutex &operator[](size_t index);
    void erase(uint32_t index);
    std::pair<std::unordered_map<uint32_t, std::shared_mutex>::iterator, bool> try_emplace(uint32_t index);
};

class FileSystem
{
protected:
    LockMap locks;
    BlockAllocator freeBlocks;

    std::unordered_set<uint32_t> traverseInitialize(void);

public:
    template <typename Lock>
    class Iterator
    {
    protected:
        LockMap &locks;
        void _load_and_check(const std::string_view &user, char file_type);
        void _step(const std::string_view &user, const std::string_view &pathToken);
        void step(std::shared_lock<std::shared_mutex> &pathLock, const std::string_view &user, const std::string_view &pathToken);

    public:
        Lock lock;
        BlockIterator inode;
        BlockIterator dataBlock;
        SearchResult result;

        Iterator(LockMap &_locks);
        void locate(const std::string_view &user, const std::vector<Request::SafePathToken> &pathTokens, bool dir = false);
    };

    FileSystem();

    void createEntry(const Request &request);

    void deleteEntry(const Request &request);

    void readFileBlock(const Request &request, std::unique_ptr<BlockData> &ptr);

    void writeFileBlock(const Request &request, const std::unique_ptr<BlockData> &ptr);
};

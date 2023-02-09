#pragma once

#include <shared_mutex>
#include <cstdint>
#include <vector>
#include <numeric>
#include <cassert>
#include <unordered_set>

class BlockAllocator
{
protected:
    std::vector<uint32_t> resources;
    std::shared_mutex lock;

public:
    BlockAllocator(std::size_t num_resources);
    BlockAllocator(std::size_t num_resources, const std::unordered_set<uint32_t> &preallocated);
    BlockAllocator(std::size_t num_resources, std::unordered_set<uint32_t> &&preallocated);
    uint32_t allocate(void);
    void deallocate(uint32_t resource);
    std::size_t remainingResources(void);
    bool empty(void);
};

class BlockResource
{
protected:
    BlockAllocator *allocator;
    uint32_t block;
    bool used;

public:
    BlockResource();
    BlockResource(BlockAllocator &_allocator);
    BlockResource(BlockAllocator &_allocator, uint32_t _allocated);
    BlockResource(BlockResource &&moved_resource);
    BlockResource(const BlockResource &) = delete;
    uint32_t resource(void) const;
    void confirm(void);
    operator uint32_t(void) const;
    BlockResource &operator=(BlockResource &&moved_resource) = default;
    ~BlockResource();
};
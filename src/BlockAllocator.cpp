#include "BlockAllocator.h"
#include "fs_server.h"
#include <stdexcept>

BlockResource::BlockResource() : allocator(nullptr), block(), used(false)
{
}

BlockResource::BlockResource(BlockAllocator &_allocator) : allocator(&_allocator), block(), used(false)
{
    block = allocator->allocate();
}

BlockResource::BlockResource(BlockAllocator &_allocator, uint32_t _allocated) : allocator(&_allocator), block(_allocated), used(false)
{
}

BlockResource::BlockResource(BlockResource &&moved_resource) : allocator(moved_resource.allocator), block(moved_resource.block), used(moved_resource.used)
{
    moved_resource.confirm();
}

uint32_t BlockResource::resource(void) const
{
    return block;
}

void BlockResource::confirm(void)
{
    if (allocator)
    {
        used = true;
    }
}

BlockResource::operator uint32_t(void) const
{
    return resource();
}

BlockResource::~BlockResource()
{
    if (!used && allocator)
    {
        allocator->deallocate(resource());
    }
}

BlockAllocator::BlockAllocator(std::size_t num_resources) : resources(num_resources), lock()
{
    std::iota(resources.rbegin(), resources.rend(), 0);
}

BlockAllocator::BlockAllocator(std::size_t num_resources, const std::unordered_set<uint32_t> &preallocated) : resources(), lock()
{
    resources.reserve(num_resources);
    for (std::size_t i = 0; i < num_resources; ++i)
    {
        if (preallocated.find(i) == preallocated.cend())
        {
            resources.push_back(i);
        }
    }
}

BlockAllocator::BlockAllocator(std::size_t num_resources, std::unordered_set<uint32_t> &&preallocated) : resources(), lock()
{
    std::unordered_set<uint32_t> dummy(preallocated);
    resources.reserve(num_resources);
    for (std::size_t i = 0; i < num_resources; ++i)
    {
        if (dummy.find(i) == dummy.cend())
        {
            resources.push_back(i);
        }
    }
}

uint32_t BlockAllocator::allocate(void)
{
    std::unique_lock<std::shared_mutex> allocatorLock(lock);
    if (resources.empty())
    {
        throw std::runtime_error("no free blocks");
    }
    uint32_t resource = resources.back();
    resources.pop_back();
#ifdef DEBUG
    std::unique_lock<std::mutex> outputLock(cout_lock);
    std::clog << "allocated block: " << resource << std::endl;
#endif
    return resource;
}

void BlockAllocator::deallocate(uint32_t resource)
{
    std::unique_lock<std::shared_mutex> allocatorLock(lock);
    resources.push_back(resource);
#ifdef DEBUG
    std::unique_lock<std::mutex> outputLock(cout_lock);
    std::clog << "deallocated block: " << resource << std::endl;
#endif
}

std::size_t BlockAllocator::remainingResources(void)
{
    std::shared_lock<std::shared_mutex> allocatorLock(lock);
    return resources.size();
}

bool BlockAllocator::empty(void)
{
    std::shared_lock<std::shared_mutex> allocatorLock(lock);
    return resources.empty();
}
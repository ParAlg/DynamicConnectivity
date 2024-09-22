#ifndef DC_ALLOC
#define DC_ALLOC
#include <cassert>
#include <cstring>
#include <iostream>
#include <utility>
#include <vector>
template <typename T> class type_allocator {
  static inline constexpr size_t default_list_bytes = (1 << 18);

public:
  type_allocator(size_t block_count = default_list_bytes / sizeof(T))
      : block_size(sizeof(T)), block_count(block_count), freeList(nullptr),
        allocated_blocks(0), used_blocks(0) {
    addNewPool(block_count); // Create the initial pool
  }

  ~type_allocator() {
    // Free all allocated memory pools
    for (void *pool : pools) {
      operator delete(pool);
    }
  }

  template <typename... Args> T *create(Args &&...args) {
    if (!freeList)
      addNewPool(block_count);
    used_blocks++;
    void *block = freeList;
    freeList = *reinterpret_cast<void **>(freeList);
    return new (block) T(std::forward<Args>(args)...);
  }

  T *alloc() {
    if (!freeList) {
      // No free blocks available, create a new pool
      addNewPool(block_count);
    }

    used_blocks++;
    // Remove the first block from the freelist
    void *block = freeList;
    freeList =
        *reinterpret_cast<void **>(freeList); // Move to the next free block
    return reinterpret_cast<T *>(block);
  }

  void free(T *block) {
    used_blocks--;
    // Insert the block back into the freelist
    block->~T();
    *reinterpret_cast<void **>(block) = freeList;
    freeList = block;
  }
  // void destroy(T* ptr) {
  //   assert(ptr != nullptr);
  //   ptr->~T();
  //   free(ptr);
  // }
  void stats() {
    std::cout << "block size = " << sizeof(T) << ", allocated "
              << allocated_blocks << " blocks, used " << used_blocks
              << " blocks, which is " << used_blocks * block_size << " bytes\n";
  }
  size_t used_mem() { return sizeof(T) * used_blocks; }

private:
  size_t block_size;
  size_t block_count;
  void *freeList;            // Pointer to the first free block
  std::vector<void *> pools; // Keep track of all allocated pools
  size_t allocated_blocks;
  size_t used_blocks;

  void addNewPool(size_t block_count) {
    // Allocate a new pool of memory
    void *newPool = operator new(block_size * block_count);
    // memset(newPool, 0, block_count * block_size);
    pools.push_back(newPool);
    allocated_blocks += block_count;
    // Link all blocks in the new pool into the freelist directly
    char *block = static_cast<char *>(newPool);
    for (size_t i = 0; i < block_count - 1; ++i) {
      *reinterpret_cast<void **>(block) = block + block_size;
      block += block_size;
    }
    *reinterpret_cast<void **>(block) =
        freeList;       // Last block points to the previous freelist head
    freeList = newPool; // Update the freelist to the start of the new pool
  }
};
#endif
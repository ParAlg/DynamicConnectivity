#ifndef EDGE_SET_H
#define EDGE_SET_H
#include "alloc.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>
template <typename T, uint32_t B = 8,
          typename Allocator = type_allocator<std::array<T, B>>>
class DynamicArray {
private:
  using EBlock = std::array<T, B>;
  EBlock **blocks;
  T block_capacity;
  T used_blocks;
  T num_elements;
  // index of last elements (*blocks[used_blocks-1])[(num_elements-1)%B]

  //  resize the block array when increasing its capacity
  void resize(T new_capacity) {
    EBlock **new_block = reinterpret_cast<EBlock **>(
        ::operator new(new_capacity * sizeof(EBlock *)));
    memset(new_block, 0, new_capacity * sizeof(EBlock *));
    memcpy(new_block, blocks, used_blocks * sizeof(EBlock *));
    delete[] blocks;
    blocks = new_block;
    block_capacity = new_capacity;
  }

  // adjust capacity based on density, full to double, less then 0.5 to shrink
  void adjust_capacity() {
    if (used_blocks == block_capacity)
      resize(block_capacity * 2);
    else if (used_blocks < block_capacity / 2 && block_capacity > 1)
      resize(block_capacity / 2);
  }

public:
  static Allocator *EBallocator;
  DynamicArray(uint32_t init_blocks = 1)
      : block_capacity(init_blocks), num_elements(0), used_blocks(0) {
    blocks = new EBlock *[block_capacity];
  }
  ~DynamicArray() {
    for (T i = 0; i < used_blocks; i++) {
      EBallocator->free(blocks[i]);
    }
    delete[] blocks;
  }

  // insert an element at the end of the array
  void insert(const T &element) {
    if (num_elements % B == 0) { // used blocks are all full
      adjust_capacity();         // check if there are unused blocks
      used_blocks++;
      blocks[used_blocks - 1] = EBallocator->alloc();
    }
    num_elements++;
    (*blocks[used_blocks - 1])[(num_elements - 1) % B] = element;
  }

  // delete the last element from the array
  T pop() {
    assert(num_elements != 0);
    T e = (*blocks[used_blocks - 1])[(num_elements - 1) % B];
    num_elements--;
    if (num_elements % B == 0) {
      EBallocator->free(blocks[used_blocks - 1]);
      blocks[used_blocks - 1] = nullptr;
      used_blocks--;
    }
    adjust_capacity(); // Adjust capacity after deletion
    return e;
  }

  T tail() {
    assert(num_elements != 0);
    return (*blocks[used_blocks - 1])[(num_elements - 1) % B];
  }

  T get_size() const { return num_elements; }
  bool is_empty() const { return num_elements == 0; }
  T get_capacity() const { return block_capacity; }

  // print all elements
  void print_all() const {
    for (T i = 0; i < used_blocks; i++) {
      for (T j = 0; j < B; j++)
        if (i * B + j < num_elements)
          std::cout << (*blocks[i])[j] << std::endl;
    }
  }

  // operations on a specific location
  T at(T ith) { return (*blocks[(ith - 1) / B])[(ith - 1) % B]; }
  void remove(T ith) {
    std::swap((*blocks[(ith - 1) / B])[(ith - 1) % B],
              (*blocks[used_blocks - 1])[(num_elements - 1) % B]);
    pop();
  }
  void mem_stat() { EBallocator->stats(); }
};
template <typename T, uint32_t B, typename Allocator>
Allocator *DynamicArray<T, B, Allocator>::EBallocator = nullptr;
#endif
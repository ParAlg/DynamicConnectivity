#ifndef EDGE_SET_H
#define EDGE_SET_H
#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <parlay/alloc.h>
#include <parlay/parallel.h>
template <typename T, uint32_t B = 8> class DynamicArray {
private:
  using EBlock = std::array<T, B>;
  using EBallocator = parlay::type_allocator<EBlock>;

  EBlock **blocks;
  T block_capacity;
  T used_blocks;
  T num_elements;
  // index of last elements (*blocks[used_blocks-1])[(num_elements-1)%B]

  //  resize the block array when increasing its capacity
  void resize(T new_capacity) {
    EBlock **new_block = new EBlock *[new_capacity];
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
  DynamicArray() : block_capacity(1), num_elements(0), used_blocks(0) {
    blocks = new EBlock *[block_capacity];
  }
  ~DynamicArray() {
    for (T i = 0; i < used_blocks; i++) {
      EBallocator::free(blocks[i]);
    }
    delete[] blocks;
  }

  // insert an element at the end of the array
  void insert(const T &element) {
    if (num_elements % B == 0) { // used blocks are all full
      adjust_capacity();         // check if there are unused blocks
      used_blocks++;
      blocks[used_blocks - 1] = EBallocator::alloc();
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
      EBallocator::free(blocks[used_blocks - 1]);
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
  void mem_stat() { EBallocator::print_stats(); }
};
// template <typename T> class DynamicArray {
// private:
//   struct EBlock {
//     T X[8];
//   };
//   using EBallocator = parlay::type_allocator<EBlock>;
//   T num_blocks;
//   T num_ele;
//   EBlock **blocks;

// public:
//   DynamicArray() : num_blocks(0), num_ele(0), blocks(nullptr) {}
//   ~DynamicArray() {
//     for (T i = 0; i < num_blocks; i++)
//       EBallocator::free(blocks[i]);
//   }
//   void insert(const T &element) {
//     if (num_ele % 8 == 0 || num_blocks == 0)
//       blocks[num_blocks++] = (EBlock *)EBallocator::alloc();
//     num_ele++;
//     blocks[num_blocks - 1]->X[num_ele - 8 * num_blocks - 1] = element;
//   }
//   // pop the last one, if the last block is empty, collect it.
//   T pop() {
//     if (num_blocks == 0 || num_ele == 0) {
//       std::cout << "over pop\n";
//       std::abort();
//     }
//     T e = blocks[num_blocks - 1]->X[num_ele - 8 * num_blocks - 1];
//     num_ele--;
//     if (num_ele % num_blocks == 0) {
//       EBallocator::free(blocks[num_blocks - 1]);
//       num_blocks--;
//     }
//     return e;
//   }
//   T fetch() { return blocks[num_blocks - 1]->X[num_ele - 8 * num_blocks - 1];
//   }
// };
#endif
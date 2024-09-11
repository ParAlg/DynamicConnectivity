#ifndef GRAPH
#define GRAPH
#include "alloc.h"
#include "edgeset.hpp"
#include <array>
#include <cstdint>
using vertex = uint32_t;
using edge_set = DynamicArray<uint32_t, 32>;
using EBlock = std::array<uint32_t, 32>;
template <> inline type_allocator<EBlock> *edge_set::EBallocator = nullptr;
#endif
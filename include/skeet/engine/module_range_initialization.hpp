#pragma once
#include "native_data_image.hpp"
#include <array>
#include <span>

namespace skeet::engine {
struct ModuleAddressRange {
    std::uint32_t low;
    std::uint32_t high;
};
static_assert(sizeof(ModuleAddressRange) == 8);

#if defined(_M_IX86) || defined(__i386__)
using ModuleRangeInitialize = void(__cdecl*)();
using ModuleRangeGet = ModuleAddressRange*(__cdecl*)();
#else
using ModuleRangeInitialize = void (*)();
using ModuleRangeGet = ModuleAddressRange* (*)();
#endif

struct ModuleRangeRowCells {
    std::uint32_t initialize;
    std::uint32_t get;
    std::uint32_t indirect;
};
// Exact static rows at43467A10. Each nonzero entry identifies a data cell,
// rather than a function or the address of a range object.
std::span<const ModuleRangeRowCells, 4> module_range_row_cells() noexcept;

struct ModuleRangeInitializationRow {
    ModuleRangeInitialize const* initialize;
    ModuleRangeGet const* get;
    ModuleAddressRange** const* indirect;
};
struct ModuleRangeInitializationBindings {
    std::array<ModuleRangeInitializationRow, 4> rows;
    const std::byte* const* client_module;
};

// Borrows the exact current cells, including their sentinel values. Active
// function cells must have been populated with genuine functions from verified
// bindings for the loaded engine build; this helper resolves no code address.
ModuleRangeInitializationBindings native_module_range_bindings(NativeDataImage&);

// Complete4334DA95. For each active row, initialize its external range table,
// then obtain its object through the getter or the indirect holder. Widen only
// the first pair whose low endpoint still equals the current client base+1000.
// Sentinel1 skips a row. Active zero callbacks/holders are invalid native input
// and are reported as invalid bindings instead of being silently skipped.
void initialize_module_ranges(const ModuleRangeInitializationBindings&);
} // namespace skeet::engine

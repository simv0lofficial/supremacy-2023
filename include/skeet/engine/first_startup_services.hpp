#pragma once
#include "native_data_image.hpp"
#include "native_service_table.hpp"
#include <span>

namespace skeet::engine {
struct FirstStartupServiceSlot {
    std::uint32_t address;
    std::uint32_t initial_hash;
    const char* name;
};
std::span<const FirstStartupServiceSlot, 9> first_startup_service_slots() noexcept;

// All imports are genuine already-loaded Windows entry points. This context
// borrows source-owned data; no executable input image or VM is required.
struct FirstStartupServiceBindings {
    NativeDataImage& data;
    LoadedModuleEnvironment modules;
    NativeImageEnvironment images;
    ImageDirectoryEntry image_directory_entry;
    NativeServiceTerminateProcess terminate_process;
};

// Source operation434822C8 ->434EA5D4. The second load-order module's base
// selects the original77BF0000 branch. It creates the temporary Nt-service
// table and converts the nine current hash cells in their actual write order.
// The startup scheduler ignores the native scalar return. This conversion is
// one-shot: a repeated call consumes the then-current cells, just as native.
void initialize_first_startup_services(const FirstStartupServiceBindings&);
} // namespace skeet::engine

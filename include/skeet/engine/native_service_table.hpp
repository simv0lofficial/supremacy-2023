#pragma once
#include "loaded_modules.hpp"
#include "pe_exports.hpp"
#include "forwarded_exports.hpp"
#include <array>

namespace skeet::engine {
struct NativeServiceRecord {
    std::uint32_t name_hash;
    std::uint32_t function_rva;
    std::uint32_t service_number;
};
struct NativeServiceTable {
    std::uint32_t count;
    std::array<NativeServiceRecord, 1024> records;
};
static_assert(sizeof(NativeServiceRecord) == 12);
static_assert(sizeof(NativeServiceTable) == 0x3004);

#if defined(_M_IX86) || defined(__i386__)
using NativeServiceTerminateProcess = std::int32_t(__stdcall*)(void*, std::int32_t);
#else
using NativeServiceTerminateProcess = std::int32_t (*)(void*, std::int32_t);
#endif

// Complete ordinary433E9D60, called by the first startup VM on its matched
// ntdll-base branch. The valid loader contains its executable and ntdll as the
// first two InLoadOrder entries. Mapped export data must remain readable.
// Writes only count and the populated records; unused records are preserved.
// Resolves no syscall target and executes no system-service instruction.
void initialize_native_service_table(NativeServiceTable&, const LoaderData&, NativeImageEnvironment,
                                     ImageDirectoryEntry);
// Complete4331F3B6: first matching hash wins; a missing/FFFFFFFF entry calls
// the genuine ZwTerminateProcess(current process,-10). Only if that import
// returns does the original returnFFFFFFFF. Successful results retain16 bits.
std::uint32_t resolve_native_service_number(const NativeServiceTable&, std::uint32_t hash,
                                            NativeServiceTerminateProcess);
} // namespace skeet::engine

#pragma once
#include "skeet/engine/loaded_modules.hpp"

namespace skeet::engine {
struct ExportAnsiString {
    std::uint16_t length;
    std::uint16_t maximum_length;
    const char* buffer;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(ExportAnsiString) == 8);
#endif

#if defined(_M_IX86) || defined(__i386__)
using ImageDirectoryEntry = std::byte*(__stdcall*)(std::byte*, std::uint8_t, std::uint16_t,
                                                   std::uint32_t*);
using InitializeExportString = void(__stdcall*)(ExportAnsiString*, const char*);
using ExportOrdinalFromString = std::int32_t(__stdcall*)(const char*, std::uint32_t,
                                                         std::uint32_t*);
#else
using ImageDirectoryEntry = std::byte* (*)(std::byte*, std::uint8_t, std::uint16_t, std::uint32_t*);
using InitializeExportString = void (*)(ExportAnsiString*, const char*);
using ExportOrdinalFromString = std::int32_t (*)(const char*, std::uint32_t, std::uint32_t*);
#endif

struct ForwardedExportEnvironment {
    LoadedModuleEnvironment modules;
    ImageDirectoryEntry image_directory_entry;
    InitializeExportString initialize_string;
    ExportOrdinalFromString ordinal_from_string;
};
ForwardedExportEnvironment native_forwarded_export_environment();

// Native 0x43320B0A and 0x433EC666, including named and ordinal forwarders.
// The caller supplies valid mapped PE data. Forwarded modules must already
// be loaded; this resolver does not load them or change their names.
std::byte* resolve_forwarded_export(std::byte* module, std::uint32_t hash, std::uint32_t ordinal,
                                    bool salted_hash, bool lock_loader,
                                    const ForwardedExportEnvironment& environment);
}

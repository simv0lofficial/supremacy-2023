#pragma once
#include "hooks.hpp"
#include "rtti.hpp"
#include <cstddef>

namespace skeet::engine {
// MEMORY_BASIC_INFORMATION prefix consumed by native433AB42B. The query is
// the actual ZwQueryVirtualMemory service, reached through4335BD9A in native.
struct HookMemoryRegion {
    void* base_address;
    void* allocation_base;
    std::uint32_t allocation_protection;
    std::size_t region_size;
    std::uint32_t state, protection, type;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(HookMemoryRegion) == 0x1c);
static_assert(offsetof(HookMemoryRegion, protection) == 0x14);
#endif
#if defined(_M_IX86) || defined(__i386__)
using HookQueryMemory = std::int32_t(__stdcall*)(void*, const void*, std::uint32_t,
                                                 HookMemoryRegion*, std::size_t, std::size_t*);
#else
using HookQueryMemory = std::int32_t (*)(void*, const void*, std::uint32_t, HookMemoryRegion*,
                                         std::size_t, std::size_t*);
#endif
struct HookDescriptorSystem {
    HookQueryMemory query_memory;
};
HookDescriptorSystem native_hook_descriptor_system();

// Complete433AB42B..433AB4AA. Probe the executable prefix, allocate the native
// descriptor backlink/RTTI/table shape, then publish original+8/replacement+10.
// Failed probes with no prefix and allocation failure preserve both pointers.
// Existing replacement storage is not freed by this producer.
void prepare_vtable_hook(VtableHookMetadata& metadata, void** original_table, JobMemory memory,
                         HookDescriptorSystem system);
// Complete433D2DC1. Atomically clear only replacement_table, then free the
// captured allocation beginning two pointers before it; original_table stays.
void clear_vtable_hook(VtableHookMetadata& metadata, JobMemory memory);
// Source composition of the captured module/type/base recipe and the actual
// native producer. Replacement binding is performed after this whole call.
void prepare_rtti_vtable_hook(VtableHookMetadata& metadata, std::byte* module,
                              std::uint32_t type_hash, std::uint32_t base_hash,
                              NativeImageEnvironment images, JobMemory memory,
                              HookDescriptorSystem system);

// Production import owner for the native RTTI and descriptor entries. Binding
// their source addresses does not select VM ordering or rewrite its operands.
class HookDescriptorRuntime {
public:
    HookDescriptorRuntime(JobMemory memory, HookDescriptorSystem system,
                          NativeImageEnvironment images)
        : memory_(memory), system_(system), images_(images) {}
    HookDescriptorRuntime(const HookDescriptorRuntime&) = delete;
    HookDescriptorRuntime& operator=(const HookDescriptorRuntime&) = delete;
    void prepare(VtableHookMetadata& metadata, void** original_table) const {
        prepare_vtable_hook(metadata, original_table, memory_, system_);
    }
    void clear(VtableHookMetadata& metadata) const {
        clear_vtable_hook(metadata, memory_);
    }
    NativeImageEnvironment images() const noexcept {
        return images_;
    }

private:
    JobMemory memory_;
    HookDescriptorSystem system_;
    NativeImageEnvironment images_;
};
void bind_hook_descriptor_runtime(HookDescriptorRuntime& runtime) noexcept;
#if defined(_M_IX86) || defined(__i386__)
void __fastcall prepare_vtable_hook_native(VtableHookMetadata*, void* ignored_edx,
                                           void** original_table);
void __fastcall clear_vtable_hook_native(VtableHookMetadata*, void* ignored_edx);
const std::byte* __fastcall find_pe_section_native(const std::byte* nt_header, std::uint32_t hash);
RttiTypeDescriptor* __fastcall find_rtti_type_native(std::byte* module, std::uint32_t hash);
void** __fastcall find_rtti_vtable_native(std::byte* module, RttiTypeDescriptor*,
                                          std::uint32_t base_hash);
void** __fastcall find_rtti_vtable_at_offset_native(std::byte* module, RttiTypeDescriptor*,
                                                    std::uint32_t offset);
#else
void prepare_vtable_hook_native(VtableHookMetadata*, void* ignored_edx, void** original_table);
void clear_vtable_hook_native(VtableHookMetadata*, void* ignored_edx);
const std::byte* find_pe_section_native(const std::byte* nt_header, std::uint32_t hash);
RttiTypeDescriptor* find_rtti_type_native(std::byte* module, std::uint32_t hash);
void** find_rtti_vtable_native(std::byte* module, RttiTypeDescriptor*, std::uint32_t base_hash);
void** find_rtti_vtable_at_offset_native(std::byte* module, RttiTypeDescriptor*,
                                         std::uint32_t offset);
#endif
}

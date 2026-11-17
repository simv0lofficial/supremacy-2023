#pragma once
#include "pe_exports.hpp"

namespace skeet::engine {
struct RttiClassHierarchy;
struct RttiTypeDescriptor {
    void* vtable;
    void* spare;
    char name[1];
};
struct RttiBaseClass {
    RttiTypeDescriptor* type;
    std::uint32_t contained_bases;
    std::int32_t member_displacement, virtual_base_displacement, virtual_displacement;
    std::uint32_t attributes;
    RttiClassHierarchy* hierarchy;
};
struct RttiClassHierarchy {
    std::uint32_t signature, attributes, count;
    RttiBaseClass** bases;
};
struct RttiObjectLocator {
    std::uint32_t signature, offset, constructor_displacement;
    RttiTypeDescriptor* type;
    RttiClassHierarchy* hierarchy;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(RttiTypeDescriptor, name) == 8);
static_assert(offsetof(RttiObjectLocator, type) == 0xc);
static_assert(offsetof(RttiObjectLocator, hierarchy) == 0x10);
static_assert(offsetof(RttiBaseClass, member_displacement) == 8);
static_assert(offsetof(RttiBaseClass, hierarchy) == 0x18);
static_assert(offsetof(RttiClassHierarchy, bases) == 0xc);
#endif

// Complete433A1085: section names are copied from their eight-byte PE field
// and compared with the native ASCII-lowercase salted FNV hash4334DEFF.
const std::byte* find_pe_section(const std::byte* nt_header, std::uint32_t name_hash);
// Complete433B305B: scan .data for aligned .?AV names, returning name-8.
RttiTypeDescriptor* find_rtti_type(std::byte* module, std::uint32_t type_hash,
                                   NativeImageEnvironment images);
// Complete4332CE3A. A nonzero base hash selects its real member displacement
// through the native hierarchy/32-locator collection; zero selects offset0.
void** find_rtti_vtable(std::byte* module, RttiTypeDescriptor* type, std::uint32_t base_hash,
                        NativeImageEnvironment images);
// Complete433EC4BD: direct locator-offset lookup, requiring cdOffset0.
void** find_rtti_vtable_at_offset(std::byte* module, RttiTypeDescriptor* type, std::uint32_t offset,
                                  NativeImageEnvironment images);
// Composition of the recovered type and vtable finders. The module, type hash
// and base hash are the actual first three captured descriptor recipe values.
void** resolve_rtti_vtable(std::byte* module, std::uint32_t type_hash, std::uint32_t base_hash,
                           NativeImageEnvironment images);
}

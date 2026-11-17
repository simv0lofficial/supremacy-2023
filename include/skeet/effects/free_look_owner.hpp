#pragma once
#include "free_look_view.hpp"

namespace skeet::client {
class NativeStartupSources;
}
namespace skeet::effects {

class FreeLookOwner;
struct FreeLookOwnerBindings {
    FreeLookOwnerState& state; //one canonical command/view-capture state
    FreeLookViewBindings& view;
    FreeLookOwner*& active; //actual43475A64 owner slot, shared by all consumers
};

inline constexpr std::size_t native_free_look_owner_allocation_size = 0x38;
std::size_t free_look_owner_size() noexcept;

// Entire ordinary inline constructor43484A3A..43484A65. The factory
// prefix43484A2B requests0x38 from43387448; caller supplies its aligned storage
// with at least free_look_owner_size() bytes. Lifecycle registration precedes
// update registration; both precede their concrete table assignments. Only the
// three delta words are initialized. active/base/capture and the primary's
// second word retain their incoming values. Configuration has a separate CRT.
// Publication belongs to the enclosing factory after this constructor.
FreeLookOwner& construct_free_look_owner(void* storage, engine::OwnerRegistryBinding lifecycle,
                                         engine::OwnerRegistryBinding updates,
                                         FreeLookOwnerBindings);

// Whole43484A2B. The x86 allocator requests0x38 with HEAP_ZERO_MEMORY; the
// borrowed canonical state receives that zero-initialization only on success.
// mem_int3.h:516 records43484A69 MOV[43475A64],EBX: publish the new source
// owner after construction, or null from the ordinary43484A67 failure branch.
FreeLookOwner* create_free_look_owner(engine::OwnerRegistryBinding lifecycle,
                                      engine::OwnerRegistryBinding updates, FreeLookOwnerBindings);

// Whole43406BFF ->433837D0, including clearing43475A64 for a null object.
// The memory boundary is the genuine process-heap/free binding; the storage
// carries43387448's alignment displacement at object-8.
void destroy_free_look_owner(FreeLookOwner*, FreeLookOwner*& active, engine::JobMemory);

FreeLookOwnerState& free_look_owner_state(FreeLookOwner&) noexcept;
FreeLookViewRuntime& free_look_view_runtime(FreeLookOwner&) noexcept;
network::UpdateObserver& free_look_view_endpoint(FreeLookOwner&) noexcept;
void* free_look_lifecycle_object(FreeLookOwner&) noexcept;

struct FreeLookInitialization {
    engine::OwnerRegistryBinding lifecycle;
    engine::OwnerRegistryBinding updates;
    FreeLookOwnerBindings bindings;
};
// The original factory is a parameterless initializer, not an owner-returning
// engine method. This source adapter discards the convenience helper result;
// the existing startup graph supplies the invocation order.
void bind_free_look_initializer(client::NativeStartupSources&, FreeLookInitialization&);

} //namespace skeet::effects

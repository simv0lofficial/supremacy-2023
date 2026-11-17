#pragma once
#include "skeet/engine/jobs.hpp"
#include <cstdint>
#include <memory>

namespace skeet::engine {
class NativeDataImage;
}
namespace skeet::ui {
class IconCache;
}
namespace skeet::client {
class NativeStartupSources;
struct ExitCallbacks;

// Canonical image storage for eleven terminal pointer-buffer finalizers and
// the two ordinary icon/entity-slot initializers. This context and its owners
// remain alive until the common ExitCallbacks pass has completed.
struct StaticCrtInitialization {
    std::shared_ptr<engine::NativeDataImage> data;
    ExitCallbacks& callbacks;
    engine::JobMemory memory;
    ui::IconCache& icons;
    // The native constructors store volatile incoming ECX in these otherwise
    // unconsumed words. Callers supply explicit source initialization inputs;
    // these fields are not decoded values or captured-register substitutes.
    std::uint32_t flags_constructor_word;
    std::uint32_t keys_constructor_word;

    void initialize_icon_cache(); //433689B6
    void initialize_entity_slots(); //433B0BCE
    void finalize_pointer_storage(std::uint32_t native_finalizer) const;
    void (*source_finalizer(std::uint32_t native_finalizer))();
};

// These registrations invoke no finalizer and retain the scheduler's order.
// Every callback names a concrete source body; no native identifier is called.
void bind_static_crt_initializers(NativeStartupSources&, StaticCrtInitialization&);
} //namespace skeet::client

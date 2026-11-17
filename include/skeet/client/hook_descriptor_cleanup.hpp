#pragma once
#include "skeet/engine/hook_descriptor_storage.hpp"
#include "skeet/engine/hook_descriptors.hpp"

namespace skeet::client {
struct ExitCallbacks;
class NativeStartupSources;

// The26 native descriptor finalizers use the same retained image cells as
// the source hook families. Keep this binding alive through ExitCallbacks::run.
class HookDescriptorCleanupContext {
public:
    HookDescriptorCleanupContext(ExitCallbacks& callbacks,
                                 engine::HookDescriptorRuntime& descriptors,
                                 engine::HookDescriptorStorage& storage)
        : callbacks(callbacks), descriptors(descriptors), storage(storage) {}
    HookDescriptorCleanupContext(const HookDescriptorCleanupContext&) = delete;
    HookDescriptorCleanupContext& operator=(const HookDescriptorCleanupContext&) = delete;
    HookDescriptorCleanupContext(HookDescriptorCleanupContext&&) = delete;
    HookDescriptorCleanupContext& operator=(HookDescriptorCleanupContext&&) = delete;

    ExitCallbacks& callbacks;
    engine::HookDescriptorRuntime& descriptors;
    engine::HookDescriptorStorage& storage;
    void (*source_finalizer(std::uint32_t native_address))();
};

// Whole ordinary wrappers at operands0C4..128. Registration only; invocation
// order remains the live VM operand scheduler's order.
void bind_hook_descriptor_initializers(NativeStartupSources&, HookDescriptorCleanupContext&);
}

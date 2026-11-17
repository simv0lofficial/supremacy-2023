#pragma once

namespace skeet::client {
struct ExitCallbacks;
class NativeStartupSources;
class StaticCleanupRuntime;

// Borrow the same 512-slot exit registry used by ShutdownRuntime and the
// concrete static-storage cleanup owner. Both owners and this binding object
// must remain at stable addresses through startup and ExitCallbacks::run().
struct ExitRegistrationContext {
    ExitCallbacks& callbacks;
    StaticCleanupRuntime& cleanup;
    ExitRegistrationContext(ExitCallbacks& callbacks, StaticCleanupRuntime& cleanup) noexcept
        : callbacks(callbacks), cleanup(cleanup) {}
    ExitRegistrationContext(const ExitRegistrationContext&) = delete;
    ExitRegistrationContext& operator=(const ExitRegistrationContext&) = delete;
    ExitRegistrationContext(ExitRegistrationContext&&) = delete;
    ExitRegistrationContext& operator=(ExitRegistrationContext&&) = delete;
};

// The fixed family contains nineteen ordinary entries at operands 008..050.
// Bind an entry only when its concrete finalizer's terminal storage owner is
// available; other entries remain explicitly unmapped in NativeStartupSources.
// Execution order remains the live metadata scheduler's order.
// Operand 004 / protected entry 434822C8 receives no source mapping here.
void bind_exit_initializers(NativeStartupSources&, ExitRegistrationContext&);
}

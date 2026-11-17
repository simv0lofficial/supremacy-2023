#pragma once
#include <memory>

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class NativeStartupSources;
}
namespace skeet::engine {
class HookRuntimeOwner;
}
namespace skeet::effects {
struct PlayerOpacityBindings;
}
namespace skeet::animation {
class RuntimeOwner;
class FixAnimationsRuntime;
class FixAnimationBackupRuntime;
class FixAnimationBackupCleanupContext;

// Construct after the canonical engine/tier0/client-state publications and
// RuntimeOwner. The opacity dependency is the actual renderer's live binding;
// its entity projection must be refreshed by that owner at each transaction.
// The supplied cleanup context was constructed before CRT and already owns
// the registration lifetime for the same process backup map.
// This owner creates no renderer, entity snapshot, hook list or second map.
// All borrowed owners must outlive it; keep it alive through exit callbacks.
class FixRuntimeOwner {
public:
    FixRuntimeOwner(RuntimeOwner&, client::ConfigurationRuntimeOwner&, engine::HookRuntimeOwner&,
                    const effects::PlayerOpacityBindings&, FixAnimationBackupCleanupContext&);
    ~FixRuntimeOwner();
    FixRuntimeOwner(const FixRuntimeOwner&) = delete;
    FixRuntimeOwner& operator=(const FixRuntimeOwner&) = delete;
    FixRuntimeOwner(FixRuntimeOwner&&) = delete;
    FixRuntimeOwner& operator=(FixRuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    RuntimeOwner& bones_owner() const noexcept;
    FixAnimationsRuntime& animations() noexcept;
    FixAnimationBackupRuntime& backup() noexcept;
    FixAnimationBackupCleanupContext& cleanup() noexcept;
    // Binds the same early433B86B4 context; the startup scheduler preserves registration
    // order and later calls the shared exit list. No finalizer runs here.
    void bind_initializers(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::animation

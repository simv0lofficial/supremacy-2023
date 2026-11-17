#pragma once
#include "fix_animation_backup.hpp"

namespace skeet::client {
struct ExitCallbacks;
class NativeStartupSources;
class ClientRuntimeStorage;
}
namespace skeet::animation {
// The cleanup body consumes only these process map/heap cells. Engine fields,
// pose accumulation, hooks and renderer opacity belong to later producers.
struct FixAnimationBackupMapBindings {
    FixAnimationBackupNode**& buckets;
    std::uint32_t& bucket_count;
    std::uint32_t& size;
    engine::JobMemory memory;
};
// Complete4339D47B over the SAME canonical map used by capture/restore.
// The allocator reference names live43467204, so each node observes changes
// made by the preceding genuine engine/free call. Native leaves the bucket
// pointer/count/growth words intact after releasing their storage.
void cleanup_fix_animation_backups(FixAnimationBackupRuntime&, void* const& allocator);
void cleanup_fix_animation_backups(const FixAnimationBackupMapBindings&, void* const& allocator);

class FixAnimationBackupCleanupContext {
public:
    explicit FixAnimationBackupCleanupContext(client::ClientRuntimeStorage&);
    FixAnimationBackupCleanupContext(client::ExitCallbacks& callbacks,
                                     FixAnimationBackupRuntime& backups,
                                     void* const& allocator) noexcept
        : callbacks(callbacks), allocator(allocator),
          map_{backups.bindings.buckets, backups.bindings.bucket_count, backups.bindings.size,
               backups.bindings.memory} {}
    FixAnimationBackupCleanupContext(const FixAnimationBackupCleanupContext&) = delete;
    FixAnimationBackupCleanupContext& operator=(const FixAnimationBackupCleanupContext&) = delete;
    FixAnimationBackupCleanupContext(FixAnimationBackupCleanupContext&&) = delete;
    FixAnimationBackupCleanupContext& operator=(FixAnimationBackupCleanupContext&&) = delete;

    client::ExitCallbacks& callbacks; //shared512slots/count/mask
    void* const& allocator;
    const FixAnimationBackupMapBindings& map() const noexcept {
        return map_;
    }
    // Late capture/restore must publish into the very map whose finalizer was
    // already registered during CRT. This performs no registration or calls.
    void require_producer(const FixAnimationBackupRuntime&) const;
    void finalize(); //43359C18 ->4339D47B
    void (*source_finalizer(std::uint32_t native_address))();

private:
    FixAnimationBackupMapBindings map_;
};

// Actual initializer433B86B4 at operand0A4 only appends43359C18 through the
// existing433837F5 implementation. The live startup scheduler determines its
// registration order; this binder does not run or append any other finalizer.
// Context, producer runtime and borrowed globals must outliveExitCallbacks::run.
void bind_fix_animation_backup_exit_initializer(client::NativeStartupSources&,
                                                FixAnimationBackupCleanupContext&);
} // namespace skeet::animation

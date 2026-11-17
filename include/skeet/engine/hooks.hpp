#pragma once
#include "jobs.hpp"
#include "unload.hpp"

namespace skeet::engine {
struct VtableHookRegistry {
    VtableHookNode& sentinel; // same sentinel owned by UnloadState
    JobMemory memory; // actual native heap exports
};

// Complete4331E834. The supplied descriptor holds original+8/replacement+0x10;
// the actual installed-vtable allocation points back to it at table[-2]. The
// atomic swap precedes node allocation; native allocation failure is preserved.
VtableHookNode* install_vtable_hook(VtableHookRegistry& registry, void* object,
                                    const VtableHookMetadata& metadata);
// Complete43369692. Searches oldest registration first, unlinks/frees that
// node, then atomically restores installed[-2]->original_table. No match is a
// no-op. This ordering differs from whole-unload restoration and is retained.
void uninstall_vtable_hook(VtableHookRegistry& registry, void* object);
} // namespace skeet::engine

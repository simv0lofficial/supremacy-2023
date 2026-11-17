#pragma once
#include "hook_descriptors.hpp"

namespace skeet::client {
class CrtRuntimeOwner;
class ClientRuntimeStorage;
class PostInitializationOwner;
class VisualRuntimeOwner;
}
namespace skeet::ui {
class StartupRenderer;
}
namespace skeet::script {
class ScriptModulesOwner;
}

namespace skeet::engine {
class NativeDataImage;
// Decode the proven VEngineVGui001 field use and validate the receiver's
// genuine CFocusOverlayPanel RTTI table. No captured object pointer is used.
void* find_build_focus_overlay_receiver(const NativeDataImage&, const VtableHookMetadata&);
// This source address can be bound by the earlier descriptor phase. Its real
// runtime is published after the menu/post-initialization owners exist.
void bind_focus_overlay_first_entry(VtableHookMetadata&) noexcept;
// Complete43416A0F over the actual canonical callable cells and receiver.
void dispatch_focus_first_invocation(NativeDataImage&, void* receiver);

class FocusOverlayHooks {
public:
    FocusOverlayHooks(client::CrtRuntimeOwner&, client::PostInitializationOwner&,
                      ui::StartupRenderer&, client::VisualRuntimeOwner&);
    ~FocusOverlayHooks();
    FocusOverlayHooks(const FocusOverlayHooks&) = delete;
    FocusOverlayHooks& operator=(const FocusOverlayHooks&) = delete;
    FocusOverlayHooks(FocusOverlayHooks&&) = delete;
    FocusOverlayHooks& operator=(FocusOverlayHooks&&) = delete;
    void prepare();
    void install();
    void restore();
    void first_invocation(void* receiver); //whole43416A0F
    void draw(void* receiver); //whole4338A69A
    void initialize(); //whole4341F57F via the actual PostInitializationOwner
    VtableHookMetadata& metadata() const noexcept {
        return metadata_;
    }

private:
    client::CrtRuntimeOwner& crt_;
    client::ClientRuntimeStorage& storage_;
    client::PostInitializationOwner& post_;
    ui::StartupRenderer& renderer_;
    client::VisualRuntimeOwner& visual_;
    script::ScriptModulesOwner& scripts_;
    VtableHookMetadata& metadata_;
    UnloadState& unload_;
    const std::uint32_t& accent_;
    bool installed_ = false;
};
}

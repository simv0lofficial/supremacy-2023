#pragma once
#include "skeet/script/bridge.hpp"
#include "skeet/hud/composite_runtime.hpp"

namespace skeet::ui {
struct RenderQueue;
}
namespace skeet::script {

// Native script-owner render method433870EE. Paint callbacks receive one nil
// argument, and renderer.* sees this frame's queue and indicator cursor.
void dispatch_paint(Bridge&, ui::RenderQueue&, std::int32_t& indicator_cursor);

// Native4336F85E, called between the in-game overlay features and menu windows
// by433AD576. Paint_ui callbacks receive no arguments.
void dispatch_paint_ui(Bridge&, ui::RenderQueue&, std::int32_t& indicator_cursor);

// Native script-owner secondary interface at+10, registered by433D20B5 at
// construction. It borrows the canonical Bridge and HUD callback registry.
class PaintHudHandler final : public hud::HudFrameCallback {
public:
    PaintHudHandler(Bridge&, engine::OwnerRegistryBinding callbacks);
    void publish_table() noexcept; //4347FFFD, after command publication
private:
    Bridge& bridge_;
    static const hud::HudFrameEntry entries_[1];
#if defined(_M_IX86) || defined(__i386__)
    static void __fastcall paint_entry(void*, ui::RenderQueue*, void*, std::int32_t*);
#else
    static void paint_entry(void*, ui::RenderQueue*, void*, std::int32_t*);
#endif
};

} // namespace skeet::script

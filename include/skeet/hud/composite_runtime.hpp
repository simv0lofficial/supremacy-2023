#pragma once
#include "skeet/engine/owner_registry.hpp"
#include "skeet/render_queue.hpp"
#include <array>

namespace skeet::ui {
class RendererContainer;
class MenuWindow;
}
namespace skeet::script {
struct Bridge;
}
namespace skeet::hud {
#if defined(_M_IX86) || defined(__i386__)
using HudFrameEntry = void(__fastcall*)(void*, ui::RenderQueue*, void*, std::int32_t*);
#else
using HudFrameEntry = void (*)(void*, ui::RenderQueue*, void*, std::int32_t*);
#endif
// The one-slot secondary interface registered by433D20B5. This is distinct
// from the lifecycle vector43475B78 and the blur vector43475A84.
struct HudFrameCallback {
    const HudFrameEntry* vtable = nullptr;
};

// Complete433D20B5. Call at the native owner's actual construction point;
// neither this function nor dispatch_hud_frame sorts the borrowed entries.
HudFrameCallback* register_hud_frame(engine::OwnerRegistryBinding, HudFrameCallback*);
//433AD5FC..433AD620. Both endpoints are captured once. Every callback receives
// the same resolved local entity and the address of the same indicator cursor.
void dispatch_hud_frame(const engine::OwnerRegistry&, ui::RenderQueue&, void* local_player,
                        std::int32_t& cursor);

struct CompositeFrameContext {
    ui::RendererContainer* const& renderer; // canonical4347690C slot
    const std::int32_t& blocked; //4347C9FC
    const float& scale; //4346D490
    void* engine; //4346A514, virtual26/12
    void* entity_list; //4346A890, virtual3
    const engine::OwnerRegistry& callbacks; //43475A9C/A0/A4
    script::Bridge& scripts;
    ui::MenuWindow* const& windows; //4347691C
    const std::uint32_t& accent;
};
// Complete433AD576..433AD66C. The renderer reserves/publishes its canonical
// command frame. HUD callbacks need connected+local; paint_ui and transitioning
// menu windows still run after an ordinary disconnected/no-local rejection.
bool draw_composite_frame(const CompositeFrameContext&);
} // namespace skeet::hud

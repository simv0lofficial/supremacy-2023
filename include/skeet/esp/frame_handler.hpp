#pragma once
#include "frame.hpp"
#include "skeet/hud/composite_runtime.hpp"

namespace skeet::esp {
// Constructor43482632 registers its secondary interface at owner+8 through
//433D20B5, then installs vtable43471E04 (slot0 =43408EC9). This concrete
// endpoint joins that same borrowed HUD registry to the full ESP body.
// Construct at the owner's registration point; owner/context outlive callbacks.
class EspFrameHandler final : public hud::HudFrameCallback {
public:
    EspFrameHandler(engine::OwnerRegistryBinding registry, EspFrameOwner& owner,
                    const EspFrameContext& context);

private:
#if defined(_M_IX86) || defined(__i386__)
    static void __fastcall draw(void*, ui::RenderQueue*, void*, std::int32_t*);
#else
    static void draw(void*, ui::RenderQueue*, void*, std::int32_t*);
#endif
    inline static constexpr hud::HudFrameEntry entries_[1]{draw};
    EspFrameOwner& owner_;
    const EspFrameContext& context_;
};
} // namespace skeet::esp

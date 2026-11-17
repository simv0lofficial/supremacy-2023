#pragma once
#include "backend_container.hpp"

namespace skeet::ui {
// Native source global 43468350 is a pointer to the external render bridge,
// whose parameterless frame methods in the working capture are slots 1,10,11,2.
// The supplied earlier image used slots 9/10 for the middle pair. This is not the D3D9
// device vtable. The shutdown word is the native 4347C9FC lifecycle gate.
class FrameRuntime {
public:
    FrameRuntime(RendererContainer& renderer, void* const* render_bridge,
                 const volatile std::uint32_t& shutdown) noexcept
        : renderer_(renderer), bridge_(render_bridge), shutdown_(shutdown) {}
    void render(); // 43384182
private:
    RendererContainer& renderer_;
    void* const* bridge_;
    const volatile std::uint32_t& shutdown_;
};
} // namespace skeet::ui

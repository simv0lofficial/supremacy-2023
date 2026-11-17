#pragma once
#include "skeet/script/bridge.hpp"
#include <cstdint>
#include <utility>
#include <vector>

namespace skeet::ui {
struct RenderQueue;
class RendererBackend;
class RendererContainer;
class ImageRasterizer;
}
namespace skeet::esp {
struct Projection;
}
namespace skeet::effects {
class BlurEffect;
}

namespace skeet::script {
struct RendererBindings {
    ui::RendererContainer* const* renderer = nullptr; // live4347690C slot
    ui::ImageRasterizer* images = nullptr;
    const esp::Projection* projection = nullptr;
    effects::BlurEffect* const* blur = nullptr; // live43475A20 slot, cleared by its lifecycle owner
    const bool* texture_disabled = nullptr; // native43468E58
    const float* indicator_scale = nullptr; // native4346D490, GuiDpiState::active_scale
};

// The closures keep this owner alive through a light-userdata upvalue. The
// active queue and indicator cursor come from Bridge::render_context.
class RendererModule {
public:
    explicit RendererModule(RendererBindings bindings = {}) : bindings_(bindings) {}
    void register_exports(Bridge&);
    int text(void*);
    int measure_text(void*);
    int rectangle(void*);
    int line(void*);
    int gradient(void*);
    int circle(void*);
    int circle_outline(void*);
    int triangle(void*);
    int world_to_screen(void*);
    int indicator(void*);
    int texture(void*);
    int load_svg(void*);
    int load_png(void*);
    int load_jpg(void*);
    int load_rgba(void*);
    int blur(void*);
    void clear_textures() noexcept {
        texture_cache_.clear();
    }

private:
    Bridge* bridge_ = nullptr;
    RendererBindings bindings_;
    std::vector<std::pair<std::uint32_t, std::int32_t>> texture_cache_;
    ui::RenderQueue* queue() const noexcept;
    ui::RendererBackend* renderer_backend() const noexcept;
    bool present(void*, int) const;
    int load_image(void*, int);
    static RendererModule& owner(void*);
    template <int (RendererModule::*Function)(void*)> static int entry(void* state) {
        return (owner(state).*Function)(state);
    }
};
} // namespace skeet::script

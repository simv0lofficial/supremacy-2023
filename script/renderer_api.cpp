#include "skeet/script/renderer_api.hpp"
#include "skeet/render/backend_container.hpp"
#include <array>
#include <stdexcept>

namespace skeet::script {
namespace {
void* (*closure_context)(void*, int) = nullptr;
}
RendererModule& RendererModule::owner(void* state) {
    return *static_cast<RendererModule*>(closure_context(state, -10003));
}
ui::RendererBackend* RendererModule::renderer_backend() const noexcept {
    const auto* slot = bindings_.renderer;
    return slot && *slot ? (*slot)->backend_if_alive() : nullptr;
}
void RendererModule::register_exports(Bridge& bridge) { // 433A0FE3
    if (!bridge.api || !bridge.state)
        throw std::logic_error("Lua renderer requires an initialized state");
    bridge_ = &bridge;
    const auto& api = *bridge.api;
    closure_context = api.touserdata;
    using Export = std::pair<const char*, LuaApi::CFunction>;
    const std::array<Export, 16> exports{
        {{"text", entry<&RendererModule::text>},
         {"measure_text", entry<&RendererModule::measure_text>},
         {"rectangle", entry<&RendererModule::rectangle>},
         {"line", entry<&RendererModule::line>},
         {"gradient", entry<&RendererModule::gradient>},
         {"circle", entry<&RendererModule::circle>},
         {"circle_outline", entry<&RendererModule::circle_outline>},
         {"triangle", entry<&RendererModule::triangle>},
         {"world_to_screen", entry<&RendererModule::world_to_screen>},
         {"indicator", entry<&RendererModule::indicator>},
         {"texture", entry<&RendererModule::texture>},
         {"load_svg", entry<&RendererModule::load_svg>},
         {"load_png", entry<&RendererModule::load_png>},
         {"load_jpg", entry<&RendererModule::load_jpg>},
         {"load_rgba", entry<&RendererModule::load_rgba>},
         {"blur", entry<&RendererModule::blur>}}};
    api.createtable(bridge.state, 0, 16);
    for (const auto& [name, function] : exports) {
        api.pushlightuserdata(bridge.state, this);
        api.pushlightuserdata(bridge.state, &texture_cache_);
        api.pushcclosure(bridge.state, function, 2);
        api.setfield(bridge.state, -2, name);
    }
    api.setfield(bridge.state, -10002, "renderer");
}
} // namespace skeet::script

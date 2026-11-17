#include "renderer_detail.hpp"
#include "skeet/render_queue.hpp"
#include "skeet/esp/projection.hpp"
#include <cmath>

namespace skeet::script {
using namespace renderer_detail;

ui::RenderQueue* RendererModule::queue() const noexcept { // 433CF503
    if (!bridge_ || !bridge_->render_context.cursor)
        return nullptr;
    return static_cast<ui::RenderQueue*>(const_cast<void*>(bridge_->render_context.queue));
}
bool RendererModule::present(void* state, int count) const { // 433E8DC7
    const auto& api = *bridge_->api;
    for (; count; --count)
        if (api.type(state, count) <= 0)
            return false;
    return true;
}
int RendererModule::rectangle(void* state) { // 433A1131
    const auto& api = *bridge_->api;
    if (api.gettop(state) < 8)
        return 0;
    auto* output = queue();
    if (!output || !present(state, 8))
        return 0;
    const auto x = integer(api, state, 1, false), y = integer(api, state, 2, false);
    const auto w = integer(api, state, 3, false), h = integer(api, state, 4, false);
    output->rectangle({x, y, w, h, color(api, state, 5, false)});
    return 0;
}
int RendererModule::line(void* state) { // 433E0750
    const auto& api = *bridge_->api;
    if (api.gettop(state) < 8)
        return 0;
    auto* output = queue();
    if (!output || !present(state, 8))
        return 0;
    const auto x = integer(api, state, 1, false), y = integer(api, state, 2, false);
    const auto x2 = integer(api, state, 3, false), y2 = integer(api, state, 4, false);
    output->line({x, y, x2, y2, color(api, state, 5, false)});
    return 0;
}
int RendererModule::gradient(void* state) { // 43324D95
    const auto& api = *bridge_->api;
    if (api.gettop(state) < 13)
        return 0;
    auto* output = queue();
    if (!output || !present(state, 13))
        return 0;
    const auto x = integer(api, state, 1, true), y = integer(api, state, 2, true);
    const auto w = integer(api, state, 3, true), h = integer(api, state, 4, true);
    const auto first = color(api, state, 5, true), second = color(api, state, 9, true);
    const ui::GradientCommand command{x, y, w, h, first, second};
    if (api.toboolean(state, 13))
        output->horizontal_gradient(command);
    else
        output->vertical_gradient(command);
    return 0;
}
int RendererModule::circle(void* state) { // 433CDC72
    const auto& api = *bridge_->api;
    if (api.gettop(state) < 9)
        return 0;
    auto* output = queue();
    if (!output || !present(state, 9))
        return 0;
    const auto x = integer(api, state, 1, true), y = integer(api, state, 2, true);
    const auto rgba = color(api, state, 3, true);
    const auto radius = static_cast<float>(api.checknumber(state, 7));
    const auto rotation = static_cast<float>(api.checknumber(state, 8));
    const auto part = fraction(static_cast<float>(api.checknumber(state, 9)));
    output->circle(
        {x, y, rgba, radius, static_cast<float>(std::fmod(double(rotation), 360.0)), part});
    return 0;
}
int RendererModule::circle_outline(void* state) { // 433DF72F
    const auto& api = *bridge_->api;
    if (api.gettop(state) < 10)
        return 0;
    auto* output = queue();
    if (!output || !present(state, 10))
        return 0;
    const auto x = integer(api, state, 1, true), y = integer(api, state, 2, true);
    const auto rgba = color(api, state, 3, true);
    const auto radius = static_cast<float>(api.checknumber(state, 7));
    const auto rotation = static_cast<float>(api.checknumber(state, 8));
    const auto part = fraction(static_cast<float>(api.checknumber(state, 9)));
    const auto thickness = static_cast<float>(api.checknumber(state, 10));
    output->circle_outline({x, y, rgba, radius,
                            static_cast<float>(std::fmod(double(rotation), 360.0)), part,
                            thickness});
    return 0;
}
int RendererModule::triangle(void* state) { // 433AE218
    const auto& api = *bridge_->api;
    if (api.gettop(state) < 10)
        return 0;
    auto* output = queue();
    if (!output || !present(state, 10))
        return 0;
    const auto y1 = integer(api, state, 2, true), x1 = integer(api, state, 1, true);
    const auto y2 = integer(api, state, 4, true), x2 = integer(api, state, 3, true);
    const auto y3 = integer(api, state, 6, true), x3 = integer(api, state, 5, true);
    output->triangle({x1, y1, x2, y2, x3, y3, color(api, state, 7, true)});
    return 0;
}
int RendererModule::world_to_screen(void* state) { // 4340C09D
    const auto& api = *bridge_->api;
    if (api.gettop(state) >= 3 && queue()) {
        const auto x = static_cast<float>(api.checknumber(state, 1));
        const auto y = static_cast<float>(api.checknumber(state, 2));
        const auto z = static_cast<float>(api.checknumber(state, 3));
        Point screen{};
        if (bindings_.projection && esp::project_point(*bindings_.projection, {x, y, z}, screen)) {
            api.pushinteger(state, screen.x);
            api.pushinteger(state, screen.y);
            return 2;
        }
    }
    api.pushnil(state);
    api.pushnil(state);
    return 2;
}
} // namespace skeet::script

#include "renderer_detail.hpp"
#include "skeet/script/lua_text.hpp"
#include "skeet/render/backend.hpp"
#include "skeet/esp/indicators.hpp"
#include "skeet/effects/blur.hpp"
#include "skeet/text_metrics.hpp"
#include <array>
#include <bit>
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace skeet::script {
namespace renderer_detail {
std::uint32_t text_flags(const LuaApi& api, void* state, int index) {
    std::uint32_t flags = 0x100;
    if (api.type(state, index) > 0) {
        std::size_t length = 0;
        const auto* value = api.checklstring(state, index, &length);
        for (std::size_t i = 0; value && i < length; ++i) {
            switch (value[i]) {
            case '+':
                flags |= 2;
                break;
            case 'c':
                flags |= 0x10;
                break;
            case '-':
                flags |= 4;
                break;
            case 'r':
                flags |= 0x80;
                break;
            case 'b':
                flags |= 1;
                break;
            case 'd':
                flags &= ~0x100u;
                break;
            }
        }
    }
    return flags;
}
}
using namespace renderer_detail;
namespace {
std::u16string_view convert_text(const char* bytes, std::array<char16_t, 2048>& wide) {
    const auto length =
        MultiByteToWideChar(CP_UTF8, 0, bytes, -1, reinterpret_cast<wchar_t*>(wide.data()), 2048);
    wide.back() = 0;
    return length >= 2 ? std::u16string_view(wide.data(), std::size_t(length - 1))
                       : std::u16string_view{};
}
}
int RendererModule::text(void* state) { // 433CE1D0
    const auto& api = *bridge_->api;
    if (api.gettop(state) < 9)
        return 0;
    auto* output = queue();
    if (!output || !present(state, 6))
        return 0;
    const auto x = integer(api, state, 1, false), y = integer(api, state, 2, false);
    const auto rgba = color(api, state, 3, false);
    auto flags = text_flags(api, state, 7);
    const auto maximum_width = integer(api, state, 8, false);
    std::array<char, 2048> bytes{};
    if (!concatenate_lua(api, state, 9, bytes.data(), bytes.size()))
        return 0;
    std::array<char16_t, 2048> wide{};
    const auto content = convert_text(bytes.data(), wide);
    if (!content.empty()) {
        if (!(flags & 4))
            flags |= 8;
        output->text(x, y, rgba, flags, maximum_width, content);
    }
    return 0;
}
int RendererModule::measure_text(void* state) { // 4332B9BD
    const auto& api = *bridge_->api;
    Point size{};
    if (api.gettop(state) >= 2) {
        if (auto* output = queue()) {
            const auto flags = text_flags(api, state, 1);
            std::array<char, 2048> bytes{};
            if (concatenate_lua(api, state, 2, bytes.data(), bytes.size())) {
                std::array<char16_t, 2048> wide{};
                const auto content = convert_text(bytes.data(), wide);
                if (!content.empty())
                    size = ui::measure_text(*output, content, flags);
            }
        }
    }
    api.pushinteger(state, size.x);
    api.pushinteger(state, size.y);
    return 2;
}
int RendererModule::indicator(void* state) { // 4332B09D
    const auto& api = *bridge_->api;
    if (api.gettop(state) < 5)
        return 0;
    auto* output = queue();
    if (!output || !present(state, 4))
        return 0;
    // Keep the four signed integers for the Lua callback; only the drawing
    // branch packs their low bytes into the native color word.
    const auto red = integer(api, state, 1, true), green = integer(api, state, 2, true);
    const auto blue = integer(api, state, 3, true), alpha = integer(api, state, 4, true);
    std::array<char, 2048> bytes{};
    const auto length = concatenate_lua(api, state, 5, bytes.data(), bytes.size());
    if (!length)
        return 0;
    const std::string_view content(bytes.data(), length);
    if (has_indicator(*bridge_)) {
        dispatch_indicator(*bridge_, bridge_->render_context, content, red, green, blue, alpha);
        return 0;
    }
    auto* renderer = renderer_backend();
    if (!renderer || !bindings_.indicator_scale)
        throw std::logic_error("Lua indicators require their renderer and DPI state");
    auto& cursor = *bridge_->render_context.cursor;
    const auto y =
        std::bit_cast<std::int32_t>(std::uint32_t(renderer->screen_height) - std::uint32_t(cursor));
    std::vector<Point> textures;
    for (const auto& texture : renderer->textures())
        textures.push_back({texture.width, texture.height});
    const auto scale = *bindings_.indicator_scale;
    const auto rgba = (std::uint32_t(red) & 255) | ((std::uint32_t(green) & 255) << 8) |
                      ((std::uint32_t(blue) & 255) << 16) | (std::uint32_t(alpha) << 24);
    esp::draw_indicator(*output, cursor, content, rgba, {scale, renderer->screen_height, textures});
    api.pushinteger(state, y);
    return 1;
}
int RendererModule::blur(void* state) { // 433F3D5D
    const auto& api = *bridge_->api;
    if (api.gettop(state) < 4 || !present(state, 4))
        return 0;
    const auto x = integer(api, state, 1, false), y = integer(api, state, 2, false);
    const auto width = integer(api, state, 3, false), height = integer(api, state, 4, false);
    if (bindings_.blur && *bindings_.blur)
        (*bindings_.blur)->draw(x, y, width, height);
    return 0;
}
} // namespace skeet::script

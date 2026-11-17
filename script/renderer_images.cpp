#include "renderer_detail.hpp"
#include "skeet/render/backend.hpp"
#include "skeet/render/icon_cache.hpp"
#include <cstring>
#include <span>

namespace skeet::script {
using namespace renderer_detail;
namespace {
std::uint32_t image_hash(std::span<const std::byte> bytes) noexcept { // 4333D6A2
    std::uint32_t hash = 0x811c9dc5;
    for (auto byte : bytes)
        hash = (hash ^ std::to_integer<std::uint8_t>(byte)) * 0x01000193u;
    return hash ^ 0x92ca5df1u;
}
std::uint32_t swap_pixel(std::uint32_t pixel) noexcept {
    return (pixel & 0xff00ff00u) | ((pixel & 255) << 16) | ((pixel >> 16) & 255);
}
}
int RendererModule::load_svg(void* state) {
    return load_image(state, 0);
} // 433B0BEC
int RendererModule::load_png(void* state) {
    return load_image(state, 1);
} // 4341F09D
int RendererModule::load_jpg(void* state) {
    return load_image(state, 2);
} // 4336BA16
int RendererModule::load_rgba(void* state) {
    return load_image(state, -1);
} // 4339DF43
int RendererModule::load_image(void* state, int format) { // shared4335BE80..4335C0D2
    const auto& api = *bridge_->api;
    auto* renderer = renderer_backend();
    if (!renderer || api.gettop(state) < 3 || !present(state, 3) || texture_cache_.size() > 10000)
        return 0;
    std::size_t length = 0;
    const auto* data = api.checklstring(state, 1, &length);
    if (!data || !length)
        return 0;
    auto width = std::uint32_t(integer(api, state, 2, true));
    auto height = std::uint32_t(integer(api, state, 3, true));
    if (width > 65535 || height > 65535)
        return 0;
    const auto bytes = std::as_bytes(std::span(data, length));
    const auto key = image_hash(bytes) + width + height;
    for (const auto& [stored, texture] : texture_cache_)
        if (stored == key) {
            api.pushinteger(state, texture);
            return 1;
        }
    std::vector<std::uint32_t> pixels;
    if (format == -1) {
        if (width * height * 4 != length)
            return 0;
        // The native RGBA branch swaps the supplied Lua string in place.
        auto* mutable_data = const_cast<char*>(data);
        for (std::size_t offset = 0; offset < length; offset += 4) {
            std::uint32_t pixel;
            std::memcpy(&pixel, mutable_data + offset, 4);
            pixel = swap_pixel(pixel);
            std::memcpy(mutable_data + offset, &pixel, 4);
        }
        pixels.resize(length / 4);
        if (length)
            std::memcpy(pixels.data(), data, length);
    } else {
        if (!bindings_.images)
            return 0;
        auto decoded_width = static_cast<std::int32_t>(width);
        auto decoded_height = static_cast<std::int32_t>(height);
        if (!bindings_.images->decode(static_cast<ui::ImageFormat>(format), bytes, decoded_width,
                                      decoded_height, pixels))
            return 0;
        width = std::uint32_t(decoded_width);
        height = std::uint32_t(decoded_height);
        for (auto& pixel : pixels)
            pixel = swap_pixel(pixel);
    }
    if (pixels.empty())
        return 0;
    const auto texture = renderer->create_texture(0, static_cast<std::int32_t>(width),
                                                  static_cast<std::int32_t>(height),
                                                  std::as_bytes(std::span(pixels)));
    if (texture == UINT32_MAX)
        return 0;
    texture_cache_.emplace_back(key, static_cast<std::int32_t>(texture));
    api.pushinteger(state, static_cast<std::int32_t>(texture));
    return 1;
}
int RendererModule::texture(void* state) { // 4336533D
    if (bindings_.texture_disabled && *bindings_.texture_disabled)
        return 0;
    const auto& api = *bridge_->api;
    const auto top = api.gettop(state);
    if (top < 9)
        return 0;
    auto* output = queue();
    if (!output || !present(state, 9))
        return 0;
    const auto texture = integer(api, state, 1, true);
    if (texture == -1)
        return 0;
    const auto x = integer(api, state, 2, true), y = integer(api, state, 3, true);
    const auto w = integer(api, state, 4, true), h = integer(api, state, 5, true);
    const auto rgba = color(api, state, 6, true);
    std::uint32_t mode = 0;
    if (top >= 10) {
        const auto* flag = api.checklstring(state, 10, nullptr);
        mode = *flag == 'r' ? 2 : *flag == 'f' ? 1 : 0;
    }
    output->texture({x, y, std::uint32_t(texture), rgba, x, y, w, h, mode, 1.0f});
    return 0;
}
} // namespace skeet::script

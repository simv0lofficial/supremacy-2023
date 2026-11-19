// Rendering Subsystem Implementation
#include "skeet/font_cache.hpp"
#include "skeet/font_metrics.hpp"
#include <stdexcept>
#include "skeet/render/backend.hpp"
#include "skeet/render/menu_background.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <limits>
#include <string>
#include <intrin.h>
#include <cstdint>
#include "skeet/render/backend_container.hpp"
#include "skeet/render/font_runtime.hpp"
#include "skeet/core.hpp"
#include <array>
#include <cstdlib>
#include "skeet/render/beam_geometry.hpp"
#include "skeet/engine/interface_call.hpp"
#include <immintrin.h>
#include "skeet/render/device_listener.hpp"
#include <new>
#include "skeet/render/dpi_bridge.hpp"
#include "skeet/menu_window.hpp"
#include <windows.h>
#include <atomic>
#include <unordered_map>
#include <vector>
#include "skeet/render/frame_runtime.hpp"
#include "skeet/render/icon_cache.hpp"
#include "skeet/esp/weapon_icon.hpp"
#include "skeet/menu/rage_weapon_selector.hpp"
#include <span>
#include "skeet/render/menu_window_assets.hpp"
#include "menu_tab_atlas.inc"
#include "skeet/render_queue.hpp"
#include "skeet/render/renderer_assets.hpp"
#include "skeet/render/startup_renderer.hpp"
#include "skeet/client/runtime_storage.hpp"
#include "skeet/engine/loaded_modules.hpp"
#include "skeet/engine/pe_exports.hpp"
#include "skeet/engine/netvars.hpp"
#include "skeet/render/startup_icons.hpp"
#include "skeet/engine/overlay_build.hpp"
#include "skeet/text_metrics.hpp"


// ============================================================================
// Subcomponent: font_cache.cpp
// ============================================================================

namespace skeet::ui {
FontSet& select_fonts(FontBackend& backend, std::uint32_t flags) noexcept { // 0x434172EC
    return (flags & 0x100) || backend.scale == 1.0f ? backend.default_fonts : backend.scaled_fonts;
}
void prepare_glyphs(FontBackend* backend, std::u16string_view text,
                    std::uint32_t flags) { // 0x43342F16
    if (!backend)
        return;
    auto& fonts = select_fonts(*backend, flags);
    const auto index = (flags & 2) ? 2 : (flags & 1) ? 1 : (flags & 4) ? 4 : 0;
    for (const auto character : text) {
        for (auto* link = &fonts[index]; link; link = link->next)
            if (link->cache && link->cache->prepare(link->cache->context, character))
                break;
    }
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: font_metrics.cpp
// ============================================================================

namespace skeet::ui {
std::int32_t font_height(FontBackend& backend, std::uint32_t index) { // 0x4334113A / 0x4336C370
    auto& fonts = backend.scale == 1.0f ? backend.default_fonts : backend.scaled_fonts;
    if (index >= fonts.size())
        throw std::out_of_range("Font metric index exceeds native font roots");
    const auto* cache = fonts[index].cache;
    if (!cache)
        return 0;
    if (!cache->height)
        throw std::logic_error("Font provider has no height callback");
    return cache->height(cache->context);
}
}

// ============================================================================
// Subcomponent: backend.cpp
// ============================================================================

namespace skeet::ui {
namespace {
std::uint32_t native_color(std::uint32_t value) noexcept {
    return (value & 0xff00ff00U) | ((value & 0xff) << 16) | ((value >> 16) & 0xff);
}
std::int32_t add(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) + std::uint32_t(b));
}
std::int32_t subtract(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) - std::uint32_t(b));
}
std::int32_t truncate(float value) noexcept {
    if (!(value >= -2147483648.0f && value < 2147483648.0f))
        return INT32_MIN;
    return static_cast<std::int32_t>(value);
}
template <class T> T read(std::span<const std::byte> bytes, std::size_t offset = 0) {
    if (offset > bytes.size() || sizeof(T) > bytes.size() - offset)
        throw std::invalid_argument("Truncated render command");
    T result;
    std::memcpy(&result, bytes.data() + offset, sizeof(result));
    return result;
}
}

RendererBackend::RendererBackend(IDirect3DDevice9* device) : device_(device) {
    create_menu_background(*this);
}
RendererBackend::~RendererBackend() {
    wait_for_render();
    release_device_objects();
}
void RendererBackend::wait_for_render() {
    // 4340AF6A waits at most one second for the render-thread critical region.
    for (unsigned i = 0; i < 100 && rendering_.load(std::memory_order_acquire); ++i)
        Sleep(10);
    rendering_.store(false, std::memory_order_release);
}

// 433E9B63: dynamic default-pool buffer, 1024 vertices per growth.
HRESULT RendererBackend::lock_buffer() {
    std::uint32_t bytes = capacity_ * sizeof(RenderVertex);
    if (!buffer_ || grow_) {
        if (grow_) {
            grow_ = false;
            bytes += 0x6000;
        }
        IDirect3DVertexBuffer9* next = nullptr;
        const auto result =
            device_->CreateVertexBuffer(bytes, 0x208, 0x142, D3DPOOL_DEFAULT, &next, nullptr);
        if (FAILED(result))
            return result;
        if (buffer_)
            buffer_->Release();
        buffer_ = next;
        capacity_ = bytes / sizeof(RenderVertex);
    }
    const auto result = buffer_->Lock(
        first_ * sizeof(RenderVertex), bytes - first_ * sizeof(RenderVertex),
        reinterpret_cast<void**>(&mapped_), first_ ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD);
    return FAILED(result) ? result : S_OK;
}

// 4339E532: preserve the native flush order on capacity/texture/type changes.
bool RendererBackend::prepare(D3DPRIMITIVETYPE primitive, IDirect3DBaseTexture9* texture,
                              std::uint32_t count, bool connected) {
    if (first_ + count_ + count >= capacity_) {
        grow_ = true;
        flush(true);
    }
    if (texture_ != texture) {
        flush();
        device_->SetTexture(0, texture);
        texture_ = texture;
    }
    if (primitive_ != primitive) {
        flush();
        primitive_ = primitive;
    }
    if (connected_ != connected) {
        flush();
        connected_ = connected;
    }
    if (!mapped_ && FAILED(lock_buffer()))
        return false;
    return true;
}

// Raw unconnected strips retain their original four-vertex format. Rectangle,
// texture and text commands write quads as triangle lists so they can batch.
void RendererBackend::draw(std::uint32_t count, std::uint32_t first) {
    device_->SetStreamSource(0, buffer_, 0, sizeof(RenderVertex));
    if (primitive_ == D3DPT_TRIANGLESTRIP && !connected_) {
        for (std::uint32_t at = 0; at < count; at += 4)
            device_->DrawPrimitive(D3DPT_TRIANGLESTRIP, first + at, 2);
        return;
    }
    auto primitives = count;
    switch (primitive_) {
    case D3DPT_POINTLIST:
        break;
    case D3DPT_LINELIST:
        primitives >>= 1;
        break;
    case D3DPT_LINESTRIP:
        --primitives;
        break;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN:
        primitives -= 2;
        break;
    default:
        primitives /= 3;
        break;
    }
    device_->DrawPrimitive(primitive_, first, primitives);
}

void RendererBackend::flush(bool reset_start) {
    if (mapped_) {
        if (alpha_ != 1.0f) {
            for (std::uint32_t i = 0; i < count_; ++i) {
                auto& color = mapped_[i].color;
                const auto alpha = static_cast<std::uint8_t>(truncate(float(color >> 24) * alpha_));
                color = (color & 0x00ffffff) | (std::uint32_t(alpha) << 24);
            }
        }
        buffer_->Unlock();
        mapped_ = nullptr;
    }
    if (count_) {
        draw(count_, first_);
        first_ += count_;
        count_ = 0;
    }
    if (reset_start || first_ + 2 >= capacity_)
        first_ = 0;
}

void RendererBackend::vertex(float x, float y, std::uint32_t color, float u, float v) {
    mapped_[count_++] = {x, y, depth_, color, u, v};
}
bool RendererBackend::quad(IDirect3DBaseTexture9* texture, float left, float top,
                           float right, float bottom, const std::array<std::uint32_t, 4>& colors,
                           float u0, float v0, float u1, float v1) {
    if (!prepare(D3DPT_TRIANGLELIST, texture, 6, false))
        return false;
    // Keep the original strip's diagonal, winding, colors and UVs. Adjacent
    // quads with the same state now share one DrawPrimitive call.
    vertex(left, top, colors[0], u0, v0);
    vertex(right, top, colors[1], u1, v0);
    vertex(left, bottom, colors[2], u0, v1);
    vertex(left, bottom, colors[2], u0, v1);
    vertex(right, top, colors[1], u1, v0);
    vertex(right, bottom, colors[3], u1, v1);
    return true;
}
bool RendererBackend::append_vertices(D3DPRIMITIVETYPE primitive, IDirect3DBaseTexture9* texture,
                                      std::span<const RenderVertex> vertices, bool connected) {
    if (!prepare(primitive, texture, static_cast<std::uint32_t>(vertices.size()), connected))
        return false;
    if (!vertices.empty())
        std::memcpy(mapped_ + count_, vertices.data(), vertices.size_bytes());
    count_ += static_cast<std::uint32_t>(vertices.size());
    return true;
}

void RendererBackend::rectangle(RectangleCommand c) { // 433E2C3F -> 433E99C8
    const auto color = native_color(c.color);
    const auto right = float(add(c.x, c.width)), bottom = float(add(c.y, c.height));
    quad(nullptr, float(c.x), float(c.y), right, bottom, {color, color, color, color});
}
void RendererBackend::four_color_gradient(FourColorGradientCommand c) { // 433D0FEC
    const auto right = float(add(c.x, c.width)), bottom = float(add(c.y, c.height));
    quad(nullptr, float(c.x), float(c.y), right, bottom,
         {native_color(c.colors[0]), native_color(c.colors[1]), native_color(c.colors[3]),
          native_color(c.colors[2])});
}
void RendererBackend::vertical_gradient(GradientCommand c) { // 43384795
    four_color_gradient({c.x,
                         c.y,
                         c.width,
                         c.height,
                         {c.first_color, c.first_color, c.second_color, c.second_color}});
}
void RendererBackend::horizontal_gradient(GradientCommand c) { // 4333C85C
    four_color_gradient({c.x,
                         c.y,
                         c.width,
                         c.height,
                         {c.first_color, c.second_color, c.second_color, c.first_color}});
}
void RendererBackend::line(LineCommand c) { // 433E7CEE
    if (!prepare(D3DPT_LINELIST, nullptr, 2, false))
        return;
    const auto color = native_color(c.color);
    vertex(float(c.from_x), float(c.from_y), color);
    vertex(float(c.to_x), float(c.to_y), color);
}
void RendererBackend::polyline(std::span<const LinePoint> points, std::uint32_t color) { // 4333088F
    if (!prepare(D3DPT_LINESTRIP, nullptr, static_cast<std::uint32_t>(points.size()), false))
        return;
    color = native_color(color);
    for (const auto& point : points)
        vertex(float(point.x), float(point.y), color);
}
void RendererBackend::triangle(TriangleCommand c) { // 43387AD9
    if (!prepare(D3DPT_TRIANGLELIST, nullptr, 3, false))
        return;
    const auto color = native_color(c.color);
    vertex(float(c.first_x), float(c.first_y), color);
    vertex(float(c.second_x), float(c.second_y), color);
    vertex(float(c.third_x), float(c.third_y), color);
}
void RendererBackend::colored(std::span<const ColoredVertex> vertices, D3DPRIMITIVETYPE type,
                              bool connected) {
    if (!prepare(type, nullptr, static_cast<std::uint32_t>(vertices.size()), connected))
        return;
    for (const auto& value : vertices)
        vertex(value.x, value.y, native_color(value.color));
}
void RendererBackend::triangle_list(std::span<const ColoredVertex> vertices) { // 4334E741
    colored(vertices, D3DPT_TRIANGLELIST, false);
}
void RendererBackend::triangle_strip(std::span<const ColoredVertex> vertices) { // 434008C4
    if (vertices.size() >= 3)
        colored(vertices, D3DPT_TRIANGLESTRIP, true);
}
void RendererBackend::colored_primitives(std::uint32_t type,
                                         std::span<const ColoredVertex> vertices) { // 43337823
    static constexpr D3DPRIMITIVETYPE types[]{D3DPT_POINTLIST,    D3DPT_LINELIST,
                                              D3DPT_TRIANGLELIST, D3DPT_TRIANGLESTRIP,
                                              D3DPT_LINESTRIP,    static_cast<D3DPRIMITIVETYPE>(0),
                                              D3DPT_TRIANGLESTRIP};
    if (type >= 7 || !types[type])
        return;
    while (!vertices.empty()) {
        const auto count = std::min<std::size_t>(capacity_, vertices.size());
        if (count == 0)
            return;
        colored(vertices.first(count), types[type], type == 3);
        vertices = vertices.subspan(count);
    }
}

void RendererBackend::set_bounds(const std::array<std::int32_t, 4>& bounds) { // 4334F405
    flush();
    const RECT rectangle{bounds[0], bounds[1], add(bounds[0], bounds[2]),
                         add(bounds[1], bounds[3])};
    device_->SetScissorRect(&rectangle);
}
std::array<std::int32_t, 4> RendererBackend::get_bounds() const { // 43368A1E
    RECT rectangle{};
    if (FAILED(device_->GetScissorRect(&rectangle)))
        return {};
    return {rectangle.left, rectangle.top, subtract(rectangle.right, rectangle.left),
            subtract(rectangle.bottom, rectangle.top)};
}
bool RendererBackend::is_bounds_reset() const { // 433EB732
    DWORD enabled = 0;
    device_->GetRenderState(D3DRS_SCISSORTESTENABLE, &enabled);
    return enabled != 0;
}
void RendererBackend::reset_bounds(bool enabled) { // 433ED99E
    if (is_bounds_reset() != enabled) {
        flush();
        device_->SetRenderState(D3DRS_SCISSORTESTENABLE, enabled);
    }
}
void RendererBackend::set_alpha(float value) { // 433AEA69
    // Native minimum_number/maximum_number select the numeric operand for NaN.
    value = std::fmax(std::fmin(value, 1.0f), 0.0f);
    if (alpha_ != value)
        flush();
    alpha_ = value;
}
void RendererBackend::set_depth(float value) noexcept {
    depth_ = value;
} // 433EBBA6

void RendererBackend::texture(TextureCommand c) { // 43368EBB -> 433CCC5C
    if (c.texture == UINT32_MAX || c.texture >= textures_.size())
        return;
    const auto& record = textures_[c.texture];
    if (!record.texture)
        return;
    auto width = c.source_width, height = c.source_height;
    if (c.scale != 1.0f) {
        width = truncate(float(width) * c.scale);
        height = truncate(float(height) * c.scale);
    }
    float u0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
    if (c.mode == 2 ||
        (c.mode == 0 && c.source_width > record.width && c.source_height > record.height)) {
        u1 = float(c.source_width) / float(record.width);
        v1 = float(c.source_height) / float(record.height);
    } else if (c.mode != 1 && !(c.mode == 0 && c.source_width == record.width &&
                                c.source_height == record.height)) {
        u0 = float(c.source_x) / float(record.width);
        u1 = float(add(c.source_x, c.source_width)) / float(record.width);
    }
    const auto color = native_color(c.color);
    const auto right = float(add(c.x, width)), bottom = float(add(c.y, height));
    quad(record.texture, float(c.x), float(c.y), right, bottom, {color, color, color, color},
         u0, 0, u1, v1);
}

void RendererBackend::consume(std::span<const std::byte> commands) { // 4338AD28..4338AD51
    reset_bounds(false);
    set_depth(0.5f);
    set_alpha(1.0f);
    for (std::size_t offset = 0; offset < commands.size();) {
        const auto header = read<std::uint32_t>(commands, offset);
        const auto end = header & 0x07ffffffU;
        if (end <= offset + 3 || end > commands.size())
            throw std::invalid_argument("Invalid render command end");
        const auto payload = commands.subspan(offset + 4, end - offset - 4);
        switch (header >> 27) {
        case 0:
            rectangle(read<RectangleCommand>(payload));
            break;
        case 1:
            four_color_gradient(read<FourColorGradientCommand>(payload));
            break;
        case 2:
            vertical_gradient(read<GradientCommand>(payload));
            break;
        case 3:
            horizontal_gradient(read<GradientCommand>(payload));
            break;
        case 4:
            line(read<LineCommand>(payload));
            break;
        case 5: {
            const auto count = read<std::uint32_t>(payload, 4);
            if (std::uint64_t(count) * 8 + 8 > payload.size())
                throw std::invalid_argument("Truncated polyline");
            std::vector<LinePoint> points(count);
            if (count)
                std::memcpy(points.data(), payload.data() + 8, count * 8);
            polyline(points, read<std::uint32_t>(payload));
            break;
        }
        case 6:
            triangle(read<TriangleCommand>(payload));
            break;
        case 7:
        case 8: {
            const auto count = read<std::uint32_t>(payload);
            if (std::uint64_t(count) * 12 + 4 > payload.size())
                throw std::invalid_argument("Truncated colored vertices");
            std::vector<ColoredVertex> values(count);
            if (count)
                std::memcpy(values.data(), payload.data() + 4, count * 12);
            if (header >> 27 == 7)
                triangle_list(values);
            else
                triangle_strip(values);
            break;
        }
        case 9:
            circle(read<CircleCommand>(payload));
            break;
        case 10: {
            const auto count = read<std::uint16_t>(payload, 20);
            if (std::size_t(count) * 2 + 22 > payload.size())
                throw std::invalid_argument("Truncated text");
            std::u16string value(count, u'\0');
            if (count)
                std::memcpy(value.data(), payload.data() + 22, count * 2);
            text(read<std::int32_t>(payload), read<std::int32_t>(payload, 4),
                 read<std::uint32_t>(payload, 8), read<std::uint32_t>(payload, 12),
                 read<std::int32_t>(payload, 16), value);
            break;
        }
        case 11:
            texture(read<TextureCommand>(payload));
            break;
        case 12:
            set_bounds(read<std::array<std::int32_t, 4>>(payload));
            break;
        case 13:
            reset_bounds(read<std::uint8_t>(payload) != 0);
            break;
        case 14:
            set_alpha(read<float>(payload));
            break;
        case 15:
            set_depth(read<float>(payload));
            break;
        case 16:
            circle_outline(read<CircleOutlineCommand>(payload));
            break;
        case 17: {
            const auto count = read<std::uint32_t>(payload, 4);
            if (std::uint64_t(count) * 12 + 8 > payload.size())
                throw std::invalid_argument("Truncated primitive vertices");
            std::vector<ColoredVertex> values(count);
            if (count)
                std::memcpy(values.data(), payload.data() + 8, count * 12);
            colored_primitives(read<std::uint32_t>(payload), values);
            break;
        }
        default:
            throw std::invalid_argument("Unknown render opcode");
        }
        offset = end;
    }
}
bool RendererBackend::render(const RenderQueue& queue) {
    if (queue.commands.empty() || !begin_frame())
        return false;
    consume(queue.commands);
    end_frame();
    return true;
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: backend_circles.cpp
// ============================================================================

namespace skeet::ui {
namespace {
constexpr float tau = 6.283185482025146484375f;
constexpr float radians = 0.01745329238474369049f;
constexpr float step = 0.2094395160675048828125f;
std::uint32_t color(std::uint32_t value) noexcept {
    return (value & 0xff00ff00U) | ((value & 255) << 16) | ((value >> 16) & 255);
}
/* deduplicated helper */
}

// 433E9526: independent 12-degree wedges, capped at 31 triangles. The native
// routine applies alpha here and again when the vertex buffer is flushed.
void RendererBackend::circle(CircleCommand command) {
    if (command.radius < 2.0f || command.radius > 1000.0f || command.fraction <= 0.0f)
        return;
    if (alpha_ != 1.0f)
        command.color =
            (command.color & 0x00ffffffU) |
            (std::uint32_t(std::uint8_t(truncate(float(command.color >> 24) * alpha_))) << 24);
    if (!prepare(D3DPT_TRIANGLELIST, nullptr, 93, true))
        return;
    const auto packed = color(command.color);
    auto angle = command.rotation * radians;
    const auto end = std::min(command.fraction, 1.0f) * tau + angle;
    auto cosine = std::cos(angle), sine = std::sin(angle);
    for (std::int32_t count = 3; count <= 93; count += 3) {
        angle = angle + step;
        vertex(sine * command.radius + float(command.x), cosine * command.radius + float(command.y),
               packed);
        const bool finished = angle >= end;
        if (finished)
            angle = end;
        cosine = std::cos(angle);
        sine = std::sin(angle);
        vertex(sine * command.radius + float(command.x), cosine * command.radius + float(command.y),
               packed);
        vertex(float(command.x), float(command.y), packed);
        if (finished)
            break;
    }
}

// 4334A4D6: connected strip with duplicate first and last vertices. Segment
// rotation uses float recurrence, preserving the original accumulated error.
void RendererBackend::circle_outline(CircleOutlineCommand command) {
    if (command.radius < 2.0f || command.radius > 1000.0f || command.fraction <= 0.0f)
        return;
    const auto arc = std::min(command.fraction, 1.0f) * tau;
    auto segments = truncate(float(std::floor(double(arc / step + 2.0f))));
    if (segments > 0x3fd)
        segments = 0x3fd;
    if (!prepare(D3DPT_TRIANGLESTRIP, nullptr, std::uint32_t(segments) * 2 + 4, true))
        return;
    const auto delta = arc / float(segments);
    const auto dc = std::cos(delta), ds = std::sin(delta);
    auto cosine = std::cos(command.rotation * radians), sine = std::sin(command.rotation * radians);
    const auto inner = command.radius - command.thickness;
    const auto packed = color(command.color);
    const auto cx = float(command.x), cy = float(command.y);
    auto emit = [&](float radius) { vertex(cosine * radius + cx, sine * radius + cy, packed); };
    emit(inner);
    emit(inner);
    emit(command.radius);
    for (std::int32_t i = 0; i < segments; ++i) {
        const auto old_cosine = cosine;
        cosine = cosine * dc - sine * ds;
        sine = sine * dc + old_cosine * ds;
        emit(inner);
        emit(command.radius);
    }
    emit(command.radius);
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: backend_container.cpp
// ============================================================================

namespace skeet::ui {
namespace {
std::atomic_flag publication_lock = ATOMIC_FLAG_INIT; // 43475BC0, 433B32F6/433CCB51
struct PublicationGuard {
    PublicationGuard() {
        while (publication_lock.test_and_set(std::memory_order_acquire)) {
        }
    }
    ~PublicationGuard() {
        publication_lock.clear(std::memory_order_release);
    }
};
struct FontExclusiveGuard {
    explicit FontExclusiveGuard(SRWLOCK& value) : lock(&value) {
        AcquireSRWLockExclusive(lock);
    }
    ~FontExclusiveGuard() {
        ReleaseSRWLockExclusive(lock);
    }
    SRWLOCK* lock;
};
void link_font_fallbacks(FontSet& fonts) noexcept {
    // See startup_font_recipes for the seven font slots.
    for (auto& font : fonts)
        font.next = nullptr;
    fonts[0].next = fonts[1].next = fonts[4].next = &fonts[3];
    fonts[3].next = fonts[5].next = &fonts[6];
    fonts[2].next = &fonts[5];
}
std::int32_t font_height(std::int32_t height, float scale) {
    const auto value = float(height) * scale + 0.5f;
    if (!(value >= -2147483648.0f && value < 2147483648.0f))
        return INT32_MIN;
    return static_cast<std::int32_t>(value);
}
}
RendererContainer::RendererContainer(IDirect3DDevice9* device)
    : backend_(std::make_unique<RendererBackend>(device)) {
    backend_->fonts = &fonts;
    for (auto& frame : frames_) {
        frame.queue.fonts = &fonts;
        frame.queue.renderer_ = this;
    }
}
RendererContainer::~RendererContainer() {
    destroy();
}
void RendererContainer::configure_fonts(std::span<const FontDefinition, 7> definitions) {
    FontExclusiveGuard lock(font_lock_);
    for (std::size_t i = 0; i < 7; ++i) {
        font_definitions[i] = definitions[i];
        auto runtime = std::make_unique<FontRuntime>(definitions[i].face, definitions[i].height,
                                                     definitions[i].flags);
        if (!runtime->valid())
            throw std::runtime_error("Native font creation failed");
        default_fonts[i] = std::move(runtime);
        fonts.default_fonts[i].cache = &default_fonts[i]->cache();
    }
    link_font_fallbacks(fonts.default_fonts);
    fonts_configured_ = true;
}
void RendererContainer::set_font_scale(float scale) noexcept {
    requested_scale_.store(scale, std::memory_order_relaxed);
} // 433EA298
float RendererContainer::requested_font_scale() const noexcept {
    return requested_scale_.load(std::memory_order_relaxed);
}
void RendererContainer::refresh_fonts() { // 433C95EA..433C96CB
    const auto requested = requested_font_scale();
    if (fonts.scale == requested)
        return;
    FontExclusiveGuard lock(font_lock_);
    if (requested != 1.0f) {
        if (!fonts_configured_)
            throw std::logic_error("Seven native font definitions are required");
        for (std::size_t i = 0; i < 7; ++i) {
            const auto& definition = font_definitions[i];
            auto runtime = std::make_unique<FontRuntime>(
                definition.face, font_height(definition.height, requested), definition.flags);
            if (!runtime->valid())
                throw std::runtime_error("Native scaled font creation failed");
            scaled_fonts[i] = std::move(runtime);
            auto& link = fonts.scaled_fonts[i];
            link.cache = &scaled_fonts[i]->cache();
            for (auto& gpu : backend_->font_textures_)
                if (gpu.link == &link && gpu.texture) {
                    gpu.texture->Release();
                    gpu.texture = nullptr;
                }
        }
        link_font_fallbacks(fonts.scaled_fonts);
    }
    fonts.scale = requested;
}
RenderQueue* RendererContainer::begin() {
    active_ = nullptr;
    for (auto& frame : frames_) {
        std::uint32_t expected = 0;
        if (frame.state.compare_exchange_strong(expected, 1, std::memory_order_acq_rel)) {
            frame.queue.renderer_ = this;
            frame.queue.commands.clear();
            frame.queue.desaturate = false;
            frame.queue.bounds_reset = false;
            frame.queue.depth = 0.5f;
            active_ = &frame;
            AcquireSRWLockShared(&font_lock_);
            return &frame.queue;
        }
    }
    return nullptr;
}
void RendererContainer::publish() {
    if (!active_)
        return;
    ReleaseSRWLockShared(&font_lock_);
    auto* frame = active_;
    active_ = nullptr;
    auto next = std::make_shared<PublishedFrame>(*frame);
    frame->state.exchange(2, std::memory_order_acq_rel);
    {
        PublicationGuard lock;
        published_.swap(next);
    }
}
bool RendererContainer::draw() {
    std::shared_ptr<PublishedFrame> frame;
    {
        PublicationGuard lock;
        frame = published_;
    }
    if (!frame || !backend_ || frame->frame->queue.commands.empty())
        return false;
    refresh_fonts();
    return backend_->render(frame->frame->queue);
}
void RendererContainer::release_context() { // 433F529D / 4340AF6A
    if (!backend_)
        return;
    backend_->wait_for_render();
    backend_->release_device_objects();
    for (std::size_t i = 7; i != 0; --i)
        scaled_fonts[i - 1].reset();
    for (std::size_t i = 7; i != 0; --i)
        default_fonts[i - 1].reset();
    backend_.reset();
    for (std::size_t i = 7; i != 0; --i)
        std::u16string().swap(font_definitions[i - 1].face);
}
void RendererContainer::destroy() {
    // Native publication guard is released first, then queue storage in the
    // order +4C,+38,+24,+10. Context cleanup follows those four vector frees.
    published_.reset();
    for (std::size_t i = frames_.size(); i != 0; --i)
        std::vector<std::byte>().swap(frames_[i - 1].queue.commands);
    release_context();
    for (auto& frame : frames_)
        frame.queue.renderer_ = nullptr;
    active_ = nullptr;
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: backend_device.cpp
// ============================================================================

namespace skeet::ui {

void RendererBackend::release_device_objects() { // 433C9479
    auto release_font = [&](auto& record) {
        if (record.texture) {
            if (record.link && record.link->cache && record.link->cache->runtime)
                record.link->cache->runtime->mark_dirty();
            record.texture->Release();
            record.texture = nullptr;
        }
    };
    if (fonts) {
        // The native context walks fixed font slots, irrespective of which
        // cache first became dirty and acquired an atlas texture.
        for (auto* set : {&fonts->default_fonts, &fonts->scaled_fonts})
            for (auto& link : *set)
                for (auto& record : font_textures_)
                    if (record.link == &link)
                        release_font(record);
    }
    for (auto& record : font_textures_)
        release_font(record);
    for (auto& record : textures_) {
        if (record.texture)
            record.texture->Release();
        record.texture = nullptr;
    }
    if (buffer_) {
        if (mapped_) {
            buffer_->Unlock();
            mapped_ = nullptr;
        }
        buffer_->Release();
        buffer_ = nullptr;
    }
    if (saved_texture_) {
        saved_texture_->Release();
        saved_texture_ = nullptr;
    }
}

// 4336AFEE: upload through a SYSTEMMEM texture, then UpdateTexture into DEFAULT.
// The original copies the supplied byte count in one memcpy, ignoring Pitch.
HRESULT RendererBackend::upload_texture(std::int32_t width, std::int32_t height,
                                        std::span<const std::byte> pixels,
                                        IDirect3DTexture9** output) {
    if (!output)
        return E_INVALIDARG;
    IDirect3DTexture9* staging = nullptr;
    auto result = device_->CreateTexture(width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM,
                                         &staging, nullptr);
    if (FAILED(result))
        return result;
    IDirect3DTexture9* target = nullptr;
    result = device_->CreateTexture(width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &target,
                                    nullptr);
    if (FAILED(result)) {
        staging->Release();
        return result;
    }
    D3DLOCKED_RECT locked{};
    result = staging->LockRect(0, &locked, nullptr, 0);
    if (SUCCEEDED(result)) {
        if (!pixels.empty())
            std::memcpy(locked.pBits, pixels.data(), pixels.size());
        result = staging->UnlockRect(0);
        if (SUCCEEDED(result))
            result = device_->UpdateTexture(staging, target);
    }
    if (SUCCEEDED(result))
        *output = target;
    else
        target->Release();
    staging->Release();
    return result;
}
std::uint32_t RendererBackend::create_texture(std::uint32_t key, std::int32_t width,
                                              std::int32_t height,
                                              std::span<const std::byte> pixels) { // 4336B39C
    TextureRecord record;
    record.key = key;
    record.width = width;
    record.height = height;
    record.pixels.assign(pixels.begin(), pixels.end());
    record.timestamp = static_cast<std::uint32_t>(__rdtsc());
    textures_.push_back(std::move(record));
    return static_cast<std::uint32_t>(textures_.size() - 1);
}
void RendererBackend::update_texture(std::uint32_t index, std::int32_t width, std::int32_t height,
                                     std::span<const std::byte> pixels) { // 4335BC40
    if (index == UINT32_MAX || index >= textures_.size())
        return;
    auto& record = textures_[index];
    record.width = width;
    record.height = height;
    record.pixels.assign(pixels.begin(), pixels.end());
    record.timestamp = static_cast<std::uint32_t>(__rdtsc());
    record.dirty = true;
}
std::int32_t RendererBackend::find_texture(std::uint32_t key) const noexcept { // 4331E088
    for (std::size_t i = 0; i < textures_.size(); ++i)
        if (textures_[i].key == key)
            return static_cast<std::int32_t>(i);
    return -1;
}
void RendererBackend::texture_size(std::uint32_t index, std::int32_t* width,
                                   std::int32_t* height) const noexcept { // 433D7503
    if (!width || !height)
        return;
    *width = 0;
    *height = 0;
    if (index != UINT32_MAX && index < textures_.size()) {
        *width = textures_[index].width;
        *height = textures_[index].height;
    }
}
void RendererBackend::upload_textures() { // 433C973D..433C97BA
    for (auto& record : textures_) {
        if (record.dirty) {
            record.dirty = false;
            if (record.texture)
                record.texture->Release();
            record.texture = nullptr;
        } else if (record.texture)
            continue;
        if (!record.pixels.empty()) {
            IDirect3DTexture9* next = nullptr;
            if (SUCCEEDED(upload_texture(record.width, record.height, record.pixels, &next)))
                record.texture = next;
        }
    }
}

bool RendererBackend::begin_frame() { // 433C95CE, D3D setup after cache uploads
    upload_fonts();
    upload_textures();
    D3DDEVICE_CREATION_PARAMETERS creation{};
    if (SUCCEEDED(device_->GetCreationParameters(&creation))) {
        RECT rectangle{};
        if (GetClientRect(creation.hFocusWindow, &rectangle)) {
            screen_width = rectangle.right - rectangle.left;
            screen_height = rectangle.bottom - rectangle.top;
        }
    }
    if (!screen_width || !screen_height)
        return false;
    rendering_.store(true, std::memory_order_release);
    D3DCAPS9 caps{};
    device_->GetDeviceCaps(&caps);
    if (caps.AlphaCmpCaps & 0x10) {
        device_->SetRenderState(static_cast<D3DRENDERSTATETYPE>(27), 1);
        device_->SetRenderState(static_cast<D3DRENDERSTATETYPE>(24), 1);
        device_->SetRenderState(static_cast<D3DRENDERSTATETYPE>(15), 1);
        device_->SetRenderState(static_cast<D3DRENDERSTATETYPE>(25), 5);
    }
    constexpr DWORD actual_states[][2]{
        {7, 1},    {8, 3},   {9, 2},   {14, 1},  {19, 5},  {20, 6},  {22, 1},  {23, 4},  {28, 0},
        {29, 0},   {48, 0},  {52, 0},  {128, 0}, {136, 1}, {137, 0}, {145, 1}, {151, 0}, {161, 1},
        {168, 15}, {171, 1}, {176, 1}, {174, 1}, {184, 0}, {206, 1}, {207, 2}, {208, 6}, {209, 5}};
    for (const auto& state : actual_states)
        device_->SetRenderState(static_cast<D3DRENDERSTATETYPE>(state[0]), state[1]);
    constexpr DWORD texture_states[][3]{{0, 4, 4}, {0, 5, 2},  {0, 6, 0},  {0, 1, 4}, {0, 2, 2},
                                        {0, 3, 0}, {0, 11, 0}, {0, 24, 0}, {1, 1, 1}, {1, 4, 1}};
    for (const auto& state : texture_states)
        device_->SetTextureStageState(state[0], static_cast<D3DTEXTURESTAGESTATETYPE>(state[1]),
                                      state[2]);
    const DWORD sampler_states[][3]{
        {0, 6, 2}, {0, 5, 2}, {0, 9, 0}, {0, 8, 0}, {0, 10, caps.MaxAnisotropy},
        {0, 1, 1}, {0, 2, 1}, {1, 1, 1}, {1, 2, 1}};
    for (const auto& state : sampler_states)
        device_->SetSamplerState(state[0], static_cast<D3DSAMPLERSTATETYPE>(state[1]), state[2]);
    if (SUCCEEDED(device_->GetFVF(&saved_fvf_)) && saved_fvf_ != 0x142)
        device_->SetFVF(0x142);
    D3DMATRIX identity{};
    identity._11 = identity._22 = identity._33 = identity._44 = 1.0f;
    device_->SetTransform(D3DTS_WORLD, &identity);
    device_->SetTransform(D3DTS_VIEW, &identity);
    D3DVIEWPORT9 viewport{};
    device_->GetViewport(&viewport);
    const auto left = float(viewport.X) + 0.5f, right = float(viewport.X + viewport.Width) + 0.5f;
    const auto top = float(viewport.Y) + 0.5f, bottom = float(viewport.Y + viewport.Height) + 0.5f;
    const auto x = 1.0f / (right - left), y = 1.0f / (top - bottom);
    D3DMATRIX projection{};
    projection._11 = x + x;
    projection._22 = y + y;
    projection._33 = 1.0f;
    projection._41 = -(left + right) * x;
    projection._42 = -(bottom + top) * y;
    projection._43 = -0.0f;
    projection._44 = 1.0f;
    device_->SetTransform(D3DTS_PROJECTION, &projection);
    set_depth(0.5f);
    if (FAILED(device_->GetTexture(0, &saved_texture_)))
        saved_texture_ = nullptr;
    device_->SetTexture(0, nullptr);
    texture_ = nullptr;
    return true;
}
void RendererBackend::end_frame() { // 4337EF22
    flush(true);
    if (saved_fvf_ != 0x142)
        device_->SetFVF(saved_fvf_);
    device_->SetTexture(0, saved_texture_);
    texture_ = saved_texture_;
    if (saved_texture_) {
        saved_texture_->Release();
        saved_texture_ = nullptr;
    }
    rendering_.store(false, std::memory_order_release);
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: backend_text.cpp
// ============================================================================

namespace skeet::ui {
namespace {
std::int32_t add_text(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) + std::uint32_t(b));
}
std::int32_t subtract_text(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) - std::uint32_t(b));
}
std::uint32_t text_color(std::uint32_t value) noexcept {
    return (value & 0xff00ff00u) | ((value & 255) << 16) | ((value >> 16) & 255);
}
FontLink* image_font(FontLink& root, char16_t c, GlyphImage& out) {
    for (auto* link = &root; link; link = link->next) {
        if (!link->cache)
            continue;
        if (!link->cache->runtime)
            throw std::logic_error("Text drawing requires a concrete font runtime");
        if (link->cache->runtime->glyph(c, out))
            return link;
    }
    return nullptr;
}
Point text_extent(FontLink& root, std::u16string_view text) { // 4338AA26
    const auto height = root.cache ? root.cache->runtime->height() : 0;
    Point result{0, height};
    std::int32_t width = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == u'\n') {
            width = 0;
            result.y = add_text(result.y, height);
        } else if (text[i] == u'\a' && text.size() - i > 8)
            i += 8;
        else {
            GlyphImage glyph;
            if (image_font(root, text[i], glyph)) {
                width = add_text(width, add_text(glyph.advance_first, glyph.advance_second));
                if (width > result.x)
                    result.x = width;
            }
        }
    }
    return result;
}
}

const RendererBackend::FontTexture* RendererBackend::font_texture(const FontLink& link) const noexcept {
    const auto at = std::find_if(font_textures_.begin(), font_textures_.end(),
                                 [&](const auto& record) { return record.link == &link; });
    return at == font_textures_.end() || !at->texture ? nullptr : &*at;
}
void RendererBackend::upload_fonts() { // 433C96CF..433C973A / 4334A887
    if (!fonts)
        return;
    auto upload = [&](FontSet& set) {
        for (auto& root : set) {
            if (!root.cache)
                continue;
            auto* font = root.cache->runtime;
            if (!font)
                throw std::logic_error("Font uploading requires a concrete font runtime");
            if (!font->consume_dirty())
                continue;
            auto at = std::find_if(font_textures_.begin(), font_textures_.end(),
                                   [&](const auto& record) { return record.link == &root; });
            if (at == font_textures_.end()) {
                font_textures_.push_back({&root, nullptr});
                at = font_textures_.end() - 1;
            }
            const auto atlas_read = font->lock_atlas();
            const auto atlas = atlas_read.view();
            IDirect3DTexture9* uploaded = nullptr;
            if (SUCCEEDED(upload_texture(atlas.width, atlas.height, std::as_bytes(atlas.pixels),
                                         &uploaded))) {
                if (at->texture)
                    at->texture->Release();
                at->texture = uploaded;
                at->width = atlas.width;
                at->height = atlas.height;
            } else {
                font->mark_dirty();
            }
        }
    };
    upload(fonts->default_fonts);
    if (fonts->scale != 1.0f)
        upload(fonts->scaled_fonts);
}

void RendererBackend::text(std::int32_t x, std::int32_t y, std::uint32_t color, std::uint32_t flags,
                           std::int32_t maximum_width,
                           std::u16string_view content) { // 4339D0B8 / 4334173D
    if (!fonts)
        return;
    auto& roots = select_fonts(*fonts, flags);
    const auto index = (flags & 2) ? 2 : (flags & 1) ? 1 : (flags & 4) ? 4 : 0;
    auto& root = roots[index];
    if (!root.cache)
        return;
    if (!root.cache->runtime)
        throw std::logic_error("Text drawing requires a concrete font runtime");
    if ((flags & 0x104) == 4 && fonts->scale != 1.0f)
        flags |= 8;
    if (flags & 0x90) {
        auto size = text_extent(root, content);
        if (maximum_width && size.x > maximum_width)
            size.x = maximum_width;
        if (flags & 0x10) {
            x = subtract_text(x, size.x / 2);
            y = subtract_text(y, size.y / 2);
        } else if (flags & 0x80)
            x = subtract_text(x, size.x);
    }
    auto pass = [&](std::int32_t origin_x, std::int32_t origin_y, std::uint32_t argb,
                    bool shadow) { // 4332667C
        auto pen_x = origin_x, pen_y = origin_y;
        const auto limit = maximum_width ? add_text(origin_x, maximum_width)
                                         : std::numeric_limits<std::int32_t>::max();
        const auto line_height = root.cache->runtime->height();
        for (std::size_t i = 0; i < content.size(); ++i) {
            const auto character = content[i];
            if (character == u'\n') {
                pen_x = origin_x;
                pen_y = add_text(pen_y, line_height);
            } else if (character == u'\a' && content.size() - i > 8) {
                if (!shadow) {
                    char digits[9]{};
                    for (std::size_t j = 0; j < 8; ++j)
                        digits[j] = static_cast<char>(content[i + 1 + j]);
                    const auto rgba = static_cast<std::uint32_t>(std::strtoul(digits, nullptr, 16));
                    argb = (rgba >> 8) | (rgba << 24);
                }
                i += 8;
            } else {
                GlyphImage glyph;
                auto* font = image_font(root, character, glyph);
                if (!font)
                    continue;
                auto* texture = font_texture(*font);
                if (!texture || glyph.right > float(texture->width) ||
                    glyph.bottom > float(texture->height))
                    continue;
                const auto draw_x = add_text(pen_x, glyph.advance_first);
                const auto next = add_text(draw_x, glyph.advance_second);
                if (next >= limit)
                    return;
                const auto left = float(draw_x), top = float(pen_y);
                const auto right = (left + glyph.right) - glyph.left;
                const auto bottom = (top + glyph.bottom) - glyph.top;
                // The producer may have grown the CPU atlas after upload.
                const auto width = float(texture->width), height = float(texture->height);
                if (quad(texture->texture, left, top, right, bottom, {argb, argb, argb, argb},
                         glyph.left / width, glyph.top / height, glyph.right / width,
                         glyph.bottom / height))
                    pen_x = next;
            }
        }
    };
    if (flags & 8) {
        const auto shadow_alpha = std::min(color >> 24, 128u);
        pass(add_text(x, 1), add_text(y, 1), (shadow_alpha << 24) | 0x010101u, true);
    } else if (flags & 0x20) {
        const auto outline_color = (color & 0xff000000u) | 0x0c0c0cu;
        constexpr std::array<Point, 4> offsets{{{1, 1}, {-1, -1}, {-1, 1}, {1, -1}}};
        for (const auto offset : offsets)
            pass(add_text(x, offset.x), add_text(y, offset.y), outline_color, true);
    }
    pass(x, y, text_color(color), false);
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: beam_geometry.cpp
// ============================================================================

namespace skeet::render {
namespace {
using esp::Vector3;
float add(float a, float b) noexcept {
    return _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
float sub(float a, float b) noexcept {
    return _mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
float mul(float a, float b) noexcept {
    return _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
float divide(float a, float b) noexcept {
    return _mm_cvtss_f32(_mm_div_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
Vector3 subtract(Vector3 a, Vector3 b) noexcept {
    return {sub(a.x, b.x), sub(a.y, b.y), sub(a.z, b.z)};
}
Vector3 normalize(Vector3 value) noexcept {
    const auto squared =
        add(add(mul(value.x, value.x), mul(value.y, value.y)), mul(value.z, value.z));
    const auto length = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(squared)));
    const auto reciprocal = divide(1.0f, add(length, 0x1p-23f)); // native 43472108
    return {mul(reciprocal, value.x), mul(reciprocal, value.y), mul(reciprocal, value.z)};
}
Vector3 width_axis(Vector3 view, Vector3 to_previous) noexcept {
    return normalize({sub(mul(view.z, to_previous.y), mul(view.y, to_previous.z)),
                      sub(mul(view.x, to_previous.z), mul(view.z, to_previous.x)),
                      sub(mul(view.y, to_previous.x), mul(view.x, to_previous.y))});
}
Vector3 tangent_axis(Vector3 view, Vector3 width) noexcept {
    return normalize({sub(mul(width.z, view.y), mul(width.y, view.z)),
                      sub(mul(width.x, view.z), mul(width.z, view.x)),
                      sub(mul(view.x, width.y), mul(view.y, width.x))});
}
Vector3 average(Vector3 previous, Vector3 current) noexcept {
    return normalize({mul(add(previous.x, current.x), 0.5f), mul(add(previous.y, current.y), 0.5f),
                      mul(add(previous.z, current.z), 0.5f)});
}
Vector3 offset(Vector3 position, Vector3 axis, float distance) noexcept {
    return {add(mul(distance, axis.x), position.x), add(mul(distance, axis.y), position.y),
            add(mul(axis.z, distance), position.z)};
}
std::uint32_t bgra(std::uint32_t rgba) noexcept {
    return (rgba & 0xFF00FF00u) | ((rgba & 0x000000FFu) << 16) | ((rgba & 0x00FF0000u) >> 16);
}
void attribute(BeamMeshDescriptor& description, unsigned channel, std::int32_t vertex,
               const void* source, std::size_t size) {
    auto* destination = static_cast<std::byte*>(description.vertex_data[channel]);
    std::memcpy(destination + std::int64_t(description.vertex_layout[channel]) * vertex, source,
                size);
}
}

BeamSegmentBuilder::BeamSegmentBuilder(BeamGeometry& output, std::int32_t segment_count,
                                       Vector3 camera, bool indexed) noexcept
    : output_(output), camera_(camera), segment_count_(segment_count),
      first_vertex_(std::uint16_t(output.vertices.size())), indexed_(indexed) {
    output_.indexed = indexed;
}
void BeamSegmentBuilder::emit_pair(Vector3 width, Vector3 tangent) {
    const auto positive = mul(pending_.width, 0.5f);
    const auto negative =
        mul(std::bit_cast<float>(std::bit_cast<std::uint32_t>(pending_.width) ^ 0x80000000u), 0.5f);
    const auto color = bgra(pending_.rgba);
    output_.vertices.push_back({offset(pending_.position, width, positive),
                                color,
                                {0.0f, pending_.texcoord},
                                {0.0f, pending_.texcoord},
                                width,
                                tangent});
    output_.vertices.push_back({offset(pending_.position, width, negative),
                                color,
                                {1.0f, pending_.texcoord},
                                {1.0f, pending_.texcoord},
                                width,
                                tangent});
}
void BeamSegmentBuilder::append_quad() {
    const auto end =
        std::uint16_t(std::uint32_t(first_vertex_) + std::uint32_t(segments_seen_) * 2u);
    const std::array<std::uint16_t, 6> quad{std::uint16_t(end - 4u), std::uint16_t(end - 3u),
                                            std::uint16_t(end - 2u), std::uint16_t(end - 3u),
                                            std::uint16_t(end - 1u), std::uint16_t(end - 2u)};
    output_.indices.insert(output_.indices.end(), quad.begin(), quad.end());
}
void BeamSegmentBuilder::next_segment(const BeamSegment& segment) { // 43320D77..43321CA9
    if (segments_seen_ > 0) {
        const auto view = subtract(pending_.position, camera_);
        const auto current_width = width_axis(view, subtract(pending_.position, segment.position));
        const auto smoothed_width =
            segments_seen_ < 2 ? current_width : average(previous_width_axis_, current_width);
        previous_width_axis_ = current_width;
        emit_pair(smoothed_width, tangent_axis(view, smoothed_width));
        if (indexed_ && segments_seen_ >= 2)
            append_quad();
    }
    pending_ = segment;
    segments_seen_ = std::bit_cast<std::int32_t>(std::uint32_t(segments_seen_) + 1u);
    if (segments_seen_ == segment_count_) {
        const auto view = subtract(pending_.position, camera_);
        emit_pair(previous_width_axis_, tangent_axis(view, previous_width_axis_));
        if (indexed_ && segments_seen_ > 1)
            append_quad();
    }
}
void build_beam_geometry(std::span<const BeamSegment> segments, Vector3 camera,
                         BeamGeometry& output, bool indexed) {
    output.vertices.clear();
    output.indices.clear();
    output.vertices.reserve(segments.size() * 2);
    if (indexed && segments.size() > 1)
        output.indices.reserve((segments.size() - 1) * 6);
    BeamSegmentBuilder builder(output, std::int32_t(segments.size()), camera, indexed);
    for (const auto& segment : segments)
        builder.next_segment(segment);
}
void draw_beam_geometry(void* mesh, const BeamGeometry& geometry) {
    if (geometry.vertices.empty())
        return;
    const auto vertex_count = std::int32_t(geometry.vertices.size());
    const auto index_count =
        geometry.indexed ? std::int32_t(geometry.indices.size()) : vertex_count;
    engine::invoke<void>(mesh, 10, std::int32_t(geometry.indexed ? 2 : 3));
    BeamMeshDescriptor description;
    engine::invoke<void>(mesh, 17, vertex_count, index_count, &description, false);
    for (std::int32_t i = 0; i < vertex_count; ++i) {
        const auto& vertex = geometry.vertices[std::size_t(i)];
        attribute(description, 0, i, &vertex.position, sizeof(vertex.position));
        attribute(description, 4, i, &vertex.bgra, sizeof(vertex.bgra));
        attribute(description, 6, i, vertex.uv0.data(), sizeof(vertex.uv0));
        attribute(description, 7, i, vertex.uv1.data(), sizeof(vertex.uv1));
        attribute(description, 14, i, &vertex.tangent_s, sizeof(vertex.tangent_s));
        attribute(description, 15, i, &vertex.tangent_t, sizeof(vertex.tangent_t));
    }
    std::int32_t indices_written = 0;
    if (description.index_size) {
        for (std::int32_t i = 0; i < index_count; ++i) {
            const auto index =
                geometry.indexed ? geometry.indices[std::size_t(i)] : std::uint16_t(i);
            description.indices[indices_written] =
                std::uint16_t(std::uint32_t(description.first_vertex) + index);
            indices_written = std::bit_cast<std::int32_t>(std::uint32_t(indices_written) +
                                                          std::uint32_t(description.index_size));
        }
    }
    engine::invoke<void>(mesh, 20, vertex_count, indices_written, &description);
    engine::invoke<void>(mesh, 12, std::int32_t(-1), std::int32_t(0));
}

} // namespace skeet::render

// ============================================================================
// Subcomponent: device_listener.cpp
// ============================================================================

namespace skeet::ui {
RendererContainer* RendererDeviceListener::renderer() const noexcept {
    if (const auto* value = std::get_if<RendererContainer*>(&renderer_))
        return *value;
    return *std::get<RendererContainer* const*>(renderer_);
}
bool RendererDeviceListener::follows(RendererContainer* const& renderer) const noexcept {
    const auto* cell = std::get_if<RendererContainer* const*>(&renderer_);
    return cell && *cell == &renderer;
}
void RendererDeviceListener::restore_base_vtable() noexcept {
    const auto binding = renderer_;
    ::new (static_cast<void*>(this)) RendererDeviceListener(binding);
}
void RendererDeviceListener::release_device_objects() {
    // Native global4347690C's +8 context can be zero after renderer teardown.
    if (auto* owner = renderer())
        if (auto* backend = owner->backend_if_alive())
            backend->release_device_objects();
}
void RendererDeviceListener::set_device(IDirect3DDevice9* device, std::uintptr_t, std::uintptr_t) {
    // This is only the borrowed context+C8 assignment: no AddRef or Release.
    if (auto* owner = renderer())
        if (auto* backend = owner->backend_if_alive())
            backend->set_device(device);
}
// The native slot is exactly RET 8; both notifications are intentionally unused.
void RendererDeviceListener::notification(std::uintptr_t, std::uintptr_t) {}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: dpi_bridge.cpp
// ============================================================================

namespace skeet::ui {
RendererDpiBridge::RendererDpiBridge(GuiDpiState& dpi, RendererContainer& renderer) noexcept
    : dpi_(dpi), renderer_(renderer) {
    dpi_.font_resources = {this, refresh_bound_renderer};
}
RendererDpiBridge::~RendererDpiBridge() {
    if (dpi_.font_resources.context == this &&
        dpi_.font_resources.refresh == refresh_bound_renderer)
        dpi_.font_resources = {};
}
void RendererDpiBridge::refresh_bound_renderer(void* context) noexcept {
    static_cast<RendererDpiBridge*>(context)->refresh();
}
void RendererDpiBridge::refresh() noexcept {
    // 4339F0AD publishes4346D490 before front43472A80 slot80 forwards through
    // 4341ACC8 to backend4346F46C slot80 (433EA298, context+100 assignment).
    renderer_.set_font_scale(dpi_.active_scale);
}
}

// ============================================================================
// Subcomponent: font_runtime.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::ui {
namespace {
struct GlyphRecord {
    float left, top, right, bottom;
    std::int32_t first, second;
};
struct SharedGlyphLock {
    explicit SharedGlyphLock(SRWLOCK& lock) noexcept : lock_(lock) {
        AcquireSRWLockShared(&lock_);
    }
    ~SharedGlyphLock() { ReleaseSRWLockShared(&lock_); }
    SRWLOCK& lock_;
};
struct ExclusiveGlyphLock {
    explicit ExclusiveGlyphLock(SRWLOCK& lock) noexcept : lock_(lock) {
        AcquireSRWLockExclusive(&lock_);
    }
    ~ExclusiveGlyphLock() { ReleaseSRWLockExclusive(&lock_); }
    SRWLOCK& lock_;
};
std::uint32_t luminance_pixel(std::uint32_t bgra) noexcept {
    if (!bgra)
        return 0;
    const auto blue = bgra & 255;
    const auto green = (bgra >> 8) & 255;
    const auto red = (bgra >> 16) & 255;
    return 0x00ffffffu | (((green * 183 + red * 54 + blue * 19) >> 8) << 24);
}
GlyphCache* metric_cache(FontBackend& backend, std::uint32_t index) {
    auto& fonts = backend.scale == 1.0f ? backend.default_fonts : backend.scaled_fonts;
    if (index >= fonts.size())
        throw std::out_of_range("Font metric index exceeds native font roots");
    return fonts[index].cache;
}
}

struct FontRuntime::Impl {
    HDC dc = nullptr;
    HFONT font = nullptr;
    HBITMAP bitmap = nullptr;
    std::uint32_t* dib = nullptr;
    std::int32_t line_height = 0, text_ascent = 0, inner_leading = 0, outer_leading = 0;
    std::int32_t dib_width = 0, dib_height = 0;
    std::int32_t atlas_width = 0, atlas_height = 0, x = 0, y = 0;
    std::uint32_t flags = 0;
    bool initialized = false;
    std::array<std::uint32_t, 2048> coverage{};
    std::unordered_map<char16_t, GlyphRecord> records;
    std::vector<std::uint32_t> pixels;
    mutable SRWLOCK lock = SRWLOCK_INIT;
    std::atomic_bool dirty = false;

    ~Impl() {
        if (dc)
            DeleteDC(dc);
        if (bitmap)
            DeleteObject(bitmap);
        if (font)
            DeleteObject(font);
    }
    bool covered(char16_t character) const noexcept {
        const auto c = static_cast<std::uint16_t>(character);
        return (coverage[c >> 5] & (std::uint32_t(1) << (c & 31))) != 0;
    }
    bool failed(char16_t character) noexcept {
        const auto c = static_cast<std::uint16_t>(character);
        coverage[c >> 5] &= ~(std::uint32_t(1) << (c & 31));
        return false;
    }
    void reserve_glyph(std::int32_t width) {
        if (x + width >= atlas_width) {
            x = 0;
            y += line_height;
        }
        if (y + line_height >= atlas_height) {
            const auto height = atlas_height ? atlas_height * 2 : atlas_width;
            pixels.resize(static_cast<std::size_t>(atlas_width) * height);
            atlas_height = height;
        }
    }
    void outline(std::int32_t width) {
        // 4332F89F..4332FA69. Black outline pixels never seed another outline.
        for (std::int32_t py = y; py < y + line_height; ++py) {
            for (std::int32_t px = x; px < x + width; ++px) {
                auto& destination = pixels[static_cast<std::size_t>(py) * atlas_width + px];
                if (destination >> 24)
                    continue;
                for (std::int32_t dx = -1; dx <= 1; ++dx) {
                    for (std::int32_t dy = -1; dy <= 1; ++dy) {
                        if ((!dx && !dy) || px + dx < x || px + dx >= x + width || py + dy < y ||
                            py + dy >= y + line_height)
                            continue;
                        const auto neighbour =
                            pixels[static_cast<std::size_t>(py + dy) * atlas_width + px + dx];
                        if ((neighbour & 255) && (neighbour & 0xff00) && (neighbour & 0xff0000) &&
                            (neighbour & 0xff000000))
                            destination = 0xdc000000;
                    }
                }
            }
        }
    }
    bool create_glyph(char16_t character) { // 4332F22F; batch 433CA690
        if (records.contains(character))
            return true;
        if (!initialized || !covered(character))
            return false;
        ABC abc{};
        const auto c = static_cast<UINT>(character);
        if (flags & 2) {
            if (!GetCharABCWidthsW(dc, c, c, &abc))
                return failed(character);
        } else {
            int width = 0;
            if (!GetCharWidth32W(dc, c, c, &width))
                return failed(character);
            abc.abcB = static_cast<UINT>(width);
        }
        if ((flags & 2) && abc.abcB == 2 && !(flags & 4)) {
            MAT2 matrix{};
            matrix.eM11.value = matrix.eM22.value = 1;
            GLYPHMETRICS gm{};
            const auto length = GetGlyphOutlineW(dc, c, GGO_GRAY8_BITMAP, &gm, 0, nullptr, &matrix);
            if (!length || length == GDI_ERROR || !gm.gmBlackBoxY)
                return failed(character);
            std::vector<std::uint8_t> gray(length);
            const auto written = GetGlyphOutlineW(dc, c, GGO_GRAY8_BITMAP, &gm, length,
                                                  gray.data(), &matrix);
            if (written == GDI_ERROR || written > length || !gm.gmBlackBoxY ||
                !gm.gmBlackBoxX || gm.gmBlackBoxX >= static_cast<UINT>(atlas_width))
                return failed(character);
            const auto pitch = written / gm.gmBlackBoxY;
            if (pitch < gm.gmBlackBoxX)
                return failed(character);
            const auto width = static_cast<std::int32_t>(gm.gmBlackBoxX);
            const auto rows = static_cast<std::int32_t>(
                std::min(gm.gmBlackBoxY, static_cast<UINT>(line_height)));
            const auto offset = std::clamp<std::int32_t>(text_ascent - gm.gmptGlyphOrigin.y,
                                                         0, line_height - rows);
            reserve_glyph(width);
            for (std::int32_t row = 0; row < rows; ++row)
                for (UINT column = 0; column < gm.gmBlackBoxX; ++column) {
                    const auto value = gray[static_cast<std::size_t>(row) * pitch + column];
                    pixels[static_cast<std::size_t>(y + row + offset) *
                               atlas_width +
                           x + column] =
                        value ? 0x00ffffffu | (std::min(std::uint32_t(value) * 4, 255u) << 24) : 0;
                }
            records.emplace(character, GlyphRecord{float(x), float(y), float(x + width),
                                                   float(y + line_height), gm.gmptGlyphOrigin.x,
                                                   gm.gmCellIncX - gm.gmptGlyphOrigin.x});
            x += width;
        } else {
            if (!abc.abcA && !abc.abcB && !abc.abcC)
                return failed(character);
            const std::int32_t border = (flags >> 2) & 1;
            if (abc.abcB >= static_cast<UINT>(atlas_width - border * 2))
                return failed(character);
            const auto width = static_cast<std::int32_t>(abc.abcB) + border * 2;
            reserve_glyph(width);
            std::memset(dib, 0, static_cast<std::size_t>(dib_width) * dib_height * 4);
            if (character != u'\t' && character != u' ') {
                SetBkColor(dc, 0);
                SetBkMode(dc, TRANSPARENT);
                SetTextColor(dc, 0xffffff);
                SetTextAlign(dc, 0);
                const wchar_t text = static_cast<wchar_t>(character);
                const auto success =
                    ExtTextOutW(dc, border - abc.abcA, border, 0, nullptr, &text, 1, nullptr);
                GdiFlush();
                if (!success)
                    return failed(character);
                const auto end_column = std::min(width - border, dib_width);
                for (std::int32_t row = border; row < line_height - border; ++row)
                    for (std::int32_t column = border; column < end_column; ++column)
                        pixels[static_cast<std::size_t>(y + row) * atlas_width + x + column] =
                            luminance_pixel(
                                dib[static_cast<std::size_t>(row) * dib_width + column]);
            }
            if (flags & 4)
                outline(width);
            records.emplace(character, GlyphRecord{float(x), float(y), float(x + width),
                                                   float(y + line_height), abc.abcA,
                                                   static_cast<std::int32_t>(abc.abcB) + abc.abcC});
            x += width;
        }
        dirty.store(true, std::memory_order_seq_cst);
        return true;
    }
};

FontRuntime::FontRuntime(std::u16string_view face, std::int32_t height, std::uint32_t flags)
    : impl_(std::make_unique<Impl>()) { // 4334E194 / 4332302B
    auto& state = *impl_;
    cache_.context = this;
    cache_.prepare = [](void* self, char16_t c) {
        return static_cast<FontRuntime*>(self)->prepare(c);
    };
    cache_.height = [](void* self) { return static_cast<FontRuntime*>(self)->height(); };
    cache_.metrics = [](void* self, char16_t c, GlyphMetrics& out) {
        GlyphImage image;
        if (!static_cast<FontRuntime*>(self)->glyph(c, image))
            return false;
        out = {image.advance_first, image.advance_second};
        return true;
    };
    cache_.internal_leading = [](void* self) {
        return static_cast<FontRuntime*>(self)->internal_leading();
    };
    cache_.ascent = [](void* self) { return static_cast<FontRuntime*>(self)->ascent(); };
    cache_.runtime = this;
    state.dc = CreateCompatibleDC(nullptr);
    if (!state.dc)
        return;
    SetMapMode(state.dc, MM_TEXT);
    SetTextAlign(state.dc, 0);
    LOGFONTW description{};
    description.lfHeight = std::bit_cast<std::int32_t>(0u - static_cast<std::uint32_t>(height));
    description.lfWeight = flags & 1 ? 700 : 400;
    description.lfQuality = flags & 2 ? CLEARTYPE_NATURAL_QUALITY : NONANTIALIASED_QUALITY;
    auto count = face.find(u'\0');
    if (count == std::u16string_view::npos)
        count = face.size();
    // wcscpy_s clears its destination when the complete name does not fit.
    if (count < LF_FACESIZE)
        for (std::size_t i = 0; i < count; ++i)
            description.lfFaceName[i] = static_cast<wchar_t>(face[i]);
    state.font = CreateFontIndirectW(&description);
    if (!state.font)
        return;
    SelectObject(state.dc, state.font);
    const auto coverage_bytes = GetFontUnicodeRanges(state.dc, nullptr);
    if (!coverage_bytes)
        return;
    std::vector<std::uint8_t> coverage(coverage_bytes);
    auto* glyphs = reinterpret_cast<GLYPHSET*>(coverage.data());
    glyphs->cbThis = coverage_bytes;
    if (!GetFontUnicodeRanges(state.dc, glyphs))
        return;
    for (DWORD i = 0; i < glyphs->cRanges; ++i) {
        const auto begin = std::uint32_t(glyphs->ranges[i].wcLow);
        const auto end = begin + glyphs->ranges[i].cGlyphs;
        for (auto c = begin; c < end && c < 65536; ++c)
            state.coverage[c >> 5] |= std::uint32_t(1) << (c & 31);
    }
    TEXTMETRICW metrics{};
    if (!GetTextMetricsW(state.dc, &metrics))
        return;
    state.text_ascent = metrics.tmAscent;
    state.inner_leading = metrics.tmInternalLeading;
    state.outer_leading = metrics.tmExternalLeading;
    state.dib_width = metrics.tmMaxCharWidth + (flags & 4 ? 2 : 0);
    state.line_height = state.dib_height = metrics.tmHeight + (flags & 4 ? 2 : 0);
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = state.dib_width;
    info.bmiHeader.biHeight = -state.dib_height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    state.bitmap = CreateDIBSection(state.dc, &info, DIB_RGB_COLORS,
                                    reinterpret_cast<void**>(&state.dib), nullptr, 0);
    if (!state.bitmap)
        return;
    std::memset(state.dib, 0, static_cast<std::size_t>(state.dib_width) * state.dib_height * 4);
    SelectObject(state.dc, state.bitmap);
    state.atlas_width = height > 60 ? 2048 : height > 20 ? 1024 : height > 8 ? 512 : 256;
    state.flags = flags;
    state.initialized = true;
}
FontRuntime::~FontRuntime() = default;
bool FontRuntime::valid() const noexcept {
    return impl_->initialized;
}
bool FontRuntime::supports(char16_t c) const noexcept {
    SharedGlyphLock lock(impl_->lock);
    return impl_->covered(c);
}
bool FontRuntime::prepare(char16_t c) {
    auto& state = *impl_;
    {
        SharedGlyphLock lock(state.lock);
        if (state.records.contains(c))
            return true;
        if (!state.covered(c))
            return false;
    }
    ExclusiveGlyphLock lock(state.lock);
    return state.create_glyph(c);
}
void FontRuntime::prepare(std::u16string_view text) {
    for (auto c : text)
        prepare(c);
}
bool FontRuntime::glyph(char16_t c, GlyphImage& out) const noexcept { // 43386484
    const auto& state = *impl_;
    SharedGlyphLock lock(state.lock);
    const auto found = state.records.find(c);
    if (found == state.records.end())
        return false;
    const auto& glyph = found->second;
    out = {glyph.left,
           glyph.top,
           glyph.right,
           glyph.bottom,
           glyph.left / float(state.atlas_width),
           glyph.top / float(state.atlas_height),
           glyph.right / float(state.atlas_width),
           glyph.bottom / float(state.atlas_height),
           glyph.first,
           glyph.second};
    return true;
}
FontRuntime::AtlasRead::AtlasRead(const Impl& state) noexcept : state_(state) {
    AcquireSRWLockShared(&state_.lock);
}
FontRuntime::AtlasRead::~AtlasRead() {
    ReleaseSRWLockShared(&state_.lock);
}
GlyphAtlas FontRuntime::AtlasRead::view() const noexcept {
    return {state_.atlas_width, state_.atlas_height, state_.pixels};
}
FontRuntime::AtlasRead FontRuntime::lock_atlas() const noexcept {
    return AtlasRead(*impl_);
}
void FontRuntime::mark_dirty() noexcept {
    impl_->dirty.store(true, std::memory_order_seq_cst);
} // 433E9F4D
bool FontRuntime::consume_dirty() noexcept {
    return impl_->dirty.exchange(false);
} // 43394E61
GlyphCache& FontRuntime::cache() noexcept {
    return cache_;
}
std::int32_t FontRuntime::height() const noexcept {
    return impl_->line_height;
} // 43416FAF
std::int32_t FontRuntime::ascent() const noexcept {
    return impl_->text_ascent;
} // 4340C42F
std::int32_t FontRuntime::internal_leading() const noexcept {
    return impl_->inner_leading;
} // 43329ACD
std::int32_t FontRuntime::external_leading() const noexcept {
    return impl_->outer_leading;
} // 43327E79
std::int32_t font_internal_leading(FontBackend& backend, std::uint32_t index) { // 433CF7C4
    const auto* cache = metric_cache(backend, index);
    if (!cache)
        return 0;
    if (!cache->internal_leading)
        throw std::logic_error("Font provider has no internal-leading accessor");
    return cache->internal_leading(cache->context);
}
std::int32_t font_ascent(FontBackend& backend, std::uint32_t index) { // 4332917E
    const auto* cache = metric_cache(backend, index);
    if (!cache)
        return 0;
    if (!cache->ascent)
        throw std::logic_error("Font provider has no ascent accessor");
    return cache->ascent(cache->context);
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: frame_runtime.cpp
// ============================================================================

namespace skeet::ui {
void FrameRuntime::render() { // 43384182..433841C8
    if (shutdown_ || !bridge_ || !*bridge_)
        return;
    auto* object = *bridge_;
    engine::invoke<void>(object, 1);
    engine::invoke<void>(object, 10);
    if (!shutdown_)
        renderer_.draw();
    engine::invoke<void>(object, 11);
    engine::invoke<void>(object, 2);
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: icon_cache.cpp
// ============================================================================

namespace skeet::ui {
namespace {
std::uint16_t normalized_definition(std::uint16_t definition) noexcept {
    return std::uint16_t(definition - 500) <= 25 ? 42 : definition;
}
std::uint32_t swapped_pixel(std::uint32_t value) noexcept {
    return (value & 0xff00ff00u) | ((value & 255) << 16) | ((value >> 16) & 255);
}
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_IMAGE_MEMBER __thiscall
#else
#define SKEET_IMAGE_MEMBER
#endif
bool SKEET_IMAGE_MEMBER buffer_get_overflow(EngineImageBuffer*, std::int32_t) {
    return false;
} // 43336939
std::int32_t wrap_image(std::uint32_t value) noexcept {
    return std::bit_cast<std::int32_t>(value);
}
bool SKEET_IMAGE_MEMBER buffer_put_overflow(EngineImageBuffer* buffer,
                                            std::int32_t amount) { // 433B36FE
    if (buffer->grow_size < 0) {
        if (!(buffer->flags & 2))
            return false;
        buffer->grow_size = 0; // 43407D76: convert external storage into owned memory.
        if (!buffer->capacity)
            buffer->memory = nullptr;
        else {
            auto* owned = static_cast<std::byte*>(
                engine::invoke<void*>(buffer->allocator, 1, buffer->capacity));
            std::memcpy(owned, buffer->memory, std::uint32_t(buffer->capacity));
            buffer->memory = owned;
        }
    }
    const auto needed = wrap_image(std::uint32_t(buffer->put) - std::uint32_t(buffer->offset) +
                                   std::uint32_t(amount));
    while (buffer->capacity < needed) {
        const auto minimum = wrap_image(std::uint32_t(buffer->capacity) + 1);
        auto next = buffer->capacity;
        if (!buffer->grow_size) {
            if (!next)
                next = 32;
            while (next < minimum)
                next = wrap_image(std::uint32_t(next) * 2);
        } else
            next = wrap_image(std::uint32_t(minimum - 1 - (minimum - 1) % buffer->grow_size) +
                              std::uint32_t(buffer->grow_size));
        if (next < minimum) {
            if (!next && minimum <= -1)
                next = -1;
            else
                do {
                    next = wrap_image(std::uint32_t(next) + std::uint32_t(minimum)) / 2;
                } while (next < minimum);
        }
        buffer->capacity = next;
        buffer->memory =
            buffer->memory
                ? static_cast<std::byte*>(
                      engine::invoke<void*>(buffer->allocator, 3, buffer->memory, next))
                : static_cast<std::byte*>(engine::invoke<void*>(buffer->allocator, 1, next));
    }
    return true;
}

bool invoke_decoder(std::uintptr_t address, ImageFormat format, const std::byte* bytes,
                    std::int32_t length, EngineImageBuffer& buffer, std::int32_t& width,
                    std::int32_t& height) {
    if (!address)
        return false;
#if (defined(__i386__) || defined(_M_IX86)) && (defined(__GNUC__) || defined(__clang__))
    // The native functions use ECX/EDX for input bytes/length, but leave their
    // stack arguments to the caller; __fastcall would pop them a second time.
    const std::uintptr_t arguments[]{reinterpret_cast<std::uintptr_t>(&buffer),
                                     reinterpret_cast<std::uintptr_t>(&width),
                                     reinterpret_cast<std::uintptr_t>(&height), 0xbf800000u, 0};
    std::uint32_t result;
    if (format == ImageFormat::svg) {
        asm volatile(
            "pushl 16(%%esi); pushl 12(%%esi); pushl 8(%%esi); pushl 4(%%esi); pushl (%%esi); call *%%edi; addl $20,%%esp"
            : "=a"(result), "+c"(bytes), "+d"(length)
            : "S"(arguments), "D"(address)
            : "memory", "cc");
    } else {
        asm volatile("pushl 8(%%esi); pushl 4(%%esi); pushl (%%esi); call *%%edi; addl $12,%%esp"
                     : "=a"(result), "+c"(bytes), "+d"(length)
                     : "S"(arguments), "D"(address)
                     : "memory", "cc");
    }
    return (result & 255) != 0;
#elif defined(_M_IX86)
    std::uint8_t result;
    auto* output = &buffer;
    auto* output_width = &width;
    auto* output_height = &height;
    if (format == ImageFormat::svg) {
        __asm {
            push 0
            push 0bf800000h
            push output_height
            push output_width
            push output
            mov ecx, bytes
            mov edx, length
            call address
            add esp, 20
            mov result, al
        }
    } else {
        __asm {
            push output_height
            push output_width
            push output
            mov ecx, bytes
            mov edx, length
            call address
            add esp, 12
            mov result, al
        }
    }
    return result != 0;
#else
    if (format == ImageFormat::svg) {
        using Function = bool (*)(const std::byte*, std::int32_t, EngineImageBuffer*, std::int32_t*,
                                  std::int32_t*, float, bool);
        return reinterpret_cast<Function>(address)(bytes, length, &buffer, &width, &height, -1.0f,
                                                   false);
    }
    using Function =
        bool (*)(const std::byte*, std::int32_t, EngineImageBuffer*, std::int32_t*, std::int32_t*);
    return reinterpret_cast<Function>(address)(bytes, length, &buffer, &width, &height);
#endif
}
void delay_image_job() {
    using Function = LONG(WINAPI*)(BOOLEAN, LARGE_INTEGER*);
    static const auto function = reinterpret_cast<Function>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwDelayExecution"));
    if (!function)
        throw std::logic_error("ZwDelayExecution is not available");
    LARGE_INTEGER interval{};
    interval.QuadPart = -10000;
    function(FALSE, &interval);
}
class ImageJob {
public:
    ImageJob(const ImageDecoderBindings& bindings, ImageFormat format,
             std::span<const std::byte> bytes, std::int32_t width, std::int32_t height,
             std::vector<std::uint32_t>& output)
        : bindings(bindings), format(format), bytes(bytes), width(width), height(height),
          output(output) {}
    virtual void release(std::uint32_t) {} //433C95B7, engine supplies the deleting flag; RET4
    virtual void run() { // 4339F131
        EngineImageBuffer buffer;
        buffer.allocator = bindings.allocator;
        buffer.get_overflow = buffer_get_overflow;
        buffer.put_overflow = buffer_put_overflow;
        const auto size = std::uint32_t(width) * std::uint32_t(height) * 4;
        buffer.capacity = static_cast<std::int32_t>(size + 1);
        buffer.memory =
            static_cast<std::byte*>(engine::invoke<void*>(bindings.allocator, 1, buffer.capacity));
        try {
            const auto decoder = format == ImageFormat::svg   ? bindings.svg
                                 : format == ImageFormat::png ? bindings.png
                                                              : bindings.jpeg;
            if (format == ImageFormat::svg || format == ImageFormat::png ||
                format == ImageFormat::jpeg)
                success =
                    invoke_decoder(decoder, format, bytes.data(),
                                   static_cast<std::int32_t>(bytes.size()), buffer, width, height);
            if (success) {
                const auto count = std::uint32_t(width) * std::uint32_t(height);
                output.resize(count);
                const auto length = count * 4;
                if (!(buffer.error & 2) && buffer.get >= buffer.offset &&
                    std::int64_t(buffer.get) + length <= buffer.maximum_put &&
                    std::uint64_t(buffer.get - buffer.offset) + length <=
                        std::uint32_t(buffer.capacity)) {
                    if (length)
                        std::memcpy(output.data(), buffer.memory + buffer.get - buffer.offset,
                                    length);
                    buffer.get += static_cast<std::int32_t>(length);
                } else
                    buffer.error |= 2;
            }
        } catch (...) {
            if (buffer.grow_size >= 0 && buffer.memory)
                engine::invoke<void>(bindings.allocator, 5, buffer.memory);
            throw;
        }
        if (buffer.grow_size >= 0 && buffer.memory)
            engine::invoke<void>(bindings.allocator, 5, buffer.memory);
    }
    virtual void complete() {
        done.store(true);
    } // 433CEFA0
    const ImageDecoderBindings& bindings;
    ImageFormat format;
    std::span<const std::byte> bytes;
    std::int32_t width, height;
    std::vector<std::uint32_t>& output;
    bool success = false;
    std::atomic_bool done = false;
};
}

bool ImageRasterizer::decode(ImageFormat format, std::span<const std::byte> bytes,
                             std::int32_t& width, std::int32_t& height,
                             std::vector<std::uint32_t>& rgba) { // 4332B8B2
    if (!bindings_.enqueue || !bindings_.execute || !bindings_.png)
        return false;
    const auto renderer = engine::invoke<void*>(bindings_.render_context_source, 11);
    if (!renderer)
        return false;
    const auto context = engine::invoke<void*>(renderer, 29);
    if (!context)
        return false;
    void* pool = nullptr;
    std::memcpy(&pool, static_cast<std::byte*>(context) + 0x88, sizeof(pool));
    if (!pool)
        return false;
    ImageJob job(bindings_, format, bytes, width, height, rgba);
#if defined(_M_IX86) || defined(__i386__)
    using Enqueue = void(__thiscall*)(void*, ImageJob*);
    using Execute = void(__thiscall*)(void*);
#else
    using Enqueue = void (*)(void*, ImageJob*);
    using Execute = void (*)(void*);
#endif
    reinterpret_cast<Enqueue>(bindings_.enqueue)(context, &job);
    while (!job.done.load()) {
        reinterpret_cast<Execute>(bindings_.execute)(pool);
        if (!job.done.load())
            delay_image_job();
    }
    width = job.width;
    height = job.height;
    return job.success;
}

IconCache::IconCache(RendererBackend& renderer, IconBindings bindings)
    : runtime_(std::in_place, renderer, bindings) {
    initialize_native_cache();
}
void IconCache::bind_runtime(RendererBackend& renderer, IconBindings bindings) {
    if (runtime_)
        throw std::logic_error("The canonical icon cache renderer was already bound");
    runtime_.emplace(renderer, bindings);
}
IconCache::Runtime& IconCache::runtime() {
    if (!runtime_)
        throw std::logic_error("Icon engine/renderer bindings have not been acquired");
    return *runtime_;
}
ImageRasterizer& IconCache::image_rasterizer() {
    return runtime().rasterizer;
}
void IconCache::initialize_native_cache() noexcept {
    textures_.fill(-1);
} //433689B6

bool IconCache::path(std::uint16_t definition, std::string& output) { // 4332C460 / 433C09AD
    auto& bindings = runtime().bindings;
    if (!definition)
        output = "materials/panorama/images/icons/ui/gungametrbomb.svg";
    else if (definition == 5)
        output = "materials/panorama/images/icons/ui/plus2.svg";
    else {
        if (!bindings.item_system && bindings.item_system_factory)
            bindings.item_system = bindings.item_system_factory();
        if (!bindings.item_system)
            return false;
        const auto schema = engine::invoke<void*>(bindings.item_system, 0);
        if (!schema)
            return false;
        const auto item = engine::invoke<void*>(schema, 4, definition);
        if (!item)
            return false;
        const char* name = nullptr;
        std::memcpy(&name, static_cast<std::byte*>(item) + 0x1dc, sizeof(name));
        if (!name)
            return false;
        if (const auto underscore = std::strchr(name, '_'))
            name = underscore + 1;
        output = "materials/panorama/images/icons/equipment/";
        output += name;
        output += ".svg";
    }
    if (output.size() >= 512)
        output.resize(511);
    return true;
}
std::vector<std::byte> IconCache::read_game_file(const std::string& path) { // 433D25A8
    auto& bindings = runtime().bindings;
    std::vector<std::byte> bytes;
    auto* file = engine::invoke<void*>(bindings.filesystem, 2, path.c_str(), "rb", "GAME");
    if (!file)
        return bytes;
    try {
        const auto length = engine::invoke<std::uint32_t>(bindings.filesystem, 7, file);
        bytes.resize(length);
        if (!engine::invoke<std::int32_t>(bindings.filesystem, 0, bytes.data(), length, file))
            bytes.clear();
    } catch (...) {
        engine::invoke<void>(bindings.filesystem, 3, file);
        throw;
    }
    engine::invoke<void>(bindings.filesystem, 3, file);
    return bytes;
}
std::int32_t IconCache::load_svg(const std::string& path, std::int32_t height,
                                 std::int32_t existing) { // 433B0C29
    auto bytes = read_game_file(path);
    if (bytes.empty() || !runtime().bindings.images.svg)
        return -1;
    std::int32_t width = 0;
    std::vector<std::uint32_t> rgba;
    if (!runtime().rasterizer.decode(ImageFormat::svg, bytes, width, height, rgba))
        return -1;
    for (auto& pixel : rgba)
        pixel = swapped_pixel(pixel);
    if (existing == -1)
        return static_cast<std::int32_t>(
            runtime().renderer.create_texture(0, width, height, std::as_bytes(std::span(rgba))));
    runtime().renderer.update_texture(std::uint32_t(existing), width, height,
                                      std::as_bytes(std::span(rgba)));
    return existing;
}
std::int32_t IconCache::cached_texture(std::uint16_t definition) const noexcept {
    const auto index = normalized_definition(definition);
    return index < textures_.size() ? textures_[index] : -1;
}
bool IconCache::load(std::uint16_t definition, std::int32_t requested_width,
                     std::int32_t requested_height, CachedIcon& output,
                     bool dimensions) { // 433591C2
    auto& renderer = runtime().renderer;
    (void)requested_width; // Native433591C2 always passes zero width to433B0C29.
    const auto index = normalized_definition(definition);
    if (index >= textures_.size())
        return false;
    auto& texture = textures_[index];
    bool reload = texture == -1;
    if (!reload && requested_height) {
        std::int32_t width = 0, height = 0;
        renderer.texture_size(std::uint32_t(texture), &width, &height);
        reload = height != requested_height;
    }
    if (reload) {
        std::string filename;
        if (!path(index, filename))
            return false;
        texture = load_svg(filename, requested_height, texture);
        if (texture == -1)
            return false;
    }
    output.texture = texture;
    if (dimensions)
        renderer.texture_size(std::uint32_t(texture), &output.width, &output.height);
    return true;
}
esp::WeaponIconProvider IconCache::weapon_provider() noexcept {
    return {this, [](void* context, RenderQueue&, std::uint16_t, std::uint16_t actual,
                     std::int32_t height, esp::WeaponIcon& icon) {
                CachedIcon loaded;
                if (!static_cast<IconCache*>(context)->load(actual, 0, height, loaded))
                    return false;
                icon = {loaded.texture, loaded.width, loaded.height};
                return true;
            }};
}
RageSelectorIconProvider IconCache::selector_provider() noexcept {
    return {this, [](void* context, RenderQueue&, std::uint8_t symbol, Point size,
                     RageSelectorIcon& icon) {
                CachedIcon loaded;
                if (!static_cast<IconCache*>(context)->load(symbol, size.x, size.y, loaded))
                    return false;
                icon = {loaded.texture, {loaded.width, loaded.height}};
                return true;
            }};
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: menu_background.cpp
// ============================================================================

namespace skeet::ui {
std::uint32_t create_menu_background(RendererBackend& renderer) {
    // Original stack array4348F902..4348F96B, copied as64bytes at4348F98C.
    //4341CC03 XORs native storage with its RDTSC key; RendererBackend's texture
    //records already keep decoded bytes for4336AFEE's D3D9 upload.
    static constexpr std::array<std::uint32_t, 16> pixels{
        0xff141414, 0xff141414, 0xff0c0c0c, 0xff141414, 0xff0c0c0c, 0xff141414,
        0xff0c0c0c, 0xff141414, 0xff0c0c0c, 0xff141414, 0xff141414, 0xff141414,
        0xff0c0c0c, 0xff141414, 0xff0c0c0c, 0xff141414,
    };
    return renderer.create_texture(menu_background_key, 4, 4, std::as_bytes(std::span(pixels)));
}
void assign_menu_background(MenuWindow& window, const RendererBackend& renderer) noexcept {
    window.background_texture =
        static_cast<std::uint32_t>(renderer.find_texture(menu_background_key));
}
}

// ============================================================================
// Subcomponent: menu_window_assets.cpp
// ============================================================================

namespace skeet::ui {
namespace {
// Decoded resource25D9, preserved byte-for-byte from skeet/Binary/menu.h.
// The supplied capture resumes43493908 at434801F2 with EAX1 and size12000.
static_assert(sizeof(menu_tab_atlas_pixels) == 384 * 48 * 4);
}
std::uint32_t create_menu_tab_atlas(RendererBackend& renderer) { //434801C9, known resource
    return renderer.create_texture(0x08f6352a, 384, 48,
                                   std::as_bytes(std::span(menu_tab_atlas_pixels)));
}
void initialize_main_menu_assets(MenuWindow& window, RendererBackend& renderer,
                                 MainMenuAssets& assets) { //43485A05
    assets.tab_atlas = create_menu_tab_atlas(renderer);
    assign_menu_background(window, renderer);
    if (assets.tab_atlas == UINT32_MAX)
        return;
    // Native pointer order+B4,+CC,+B8,+BC,+C0,+C4,+C8,+D0.
    constexpr std::array<std::int32_t, 8> source_x{0, 288, 48, 96, 144, 192, 240, 336};
    for (std::size_t i = 0; i < source_x.size(); ++i) {
        auto& tab = *window.tabs.at(i);
        tab.icon = assets.tab_atlas;
        tab.icon_source = {source_x[i], 0, 48, 48};
    }
}
}

// ============================================================================
// Subcomponent: queue_context.cpp
// ============================================================================

namespace skeet::ui {
Point RenderQueue::screen() const {
    // Like native433F49E6, this getter requires the supplied queue's live
    // embedded renderer/context. Standalone command writers have no viewport.
    const auto& context = renderer_->backend();
    return {context.screen_width, context.screen_height};
}
} //namespace skeet::ui

// ============================================================================
// Subcomponent: renderer_assets.cpp
// ============================================================================

namespace skeet::ui {
namespace {
template <class T> T read(const std::byte* pointer) {
    T value;
    std::memcpy(&value, pointer, sizeof value);
    return value;
}
void require(bool value, const char* message) {
    if (!value)
        throw std::runtime_error(message);
}
bool contains(const std::byte* module, const void* pointer, std::size_t size, std::uint32_t rva,
              std::uint32_t length) {
    const auto address = reinterpret_cast<std::uintptr_t>(pointer);
    const auto begin = reinterpret_cast<std::uintptr_t>(module) + rva;
    return address >= begin && size <= length && address - begin <= length - size;
}
bool matches(const std::byte* code, std::span<const int> pattern) {
    for (std::size_t i = 0; i < pattern.size(); ++i)
        if (pattern[i] >= 0 && static_cast<unsigned char>(code[i]) != pattern[i])
            return false;
    return true;
}
std::uintptr_t code_entry(std::byte* panorama, std::uint32_t rva,
                          std::initializer_list<int> pattern) {
    auto* address = panorama + rva;
    require(contains(panorama, address, pattern.size(), 0x1000, 0x1E5C8D) &&
                matches(address, {pattern.begin(), pattern.size()}),
            "Panorama image-codec/job entry does not match the supported build");
    return reinterpret_cast<std::uintptr_t>(address);
}
ImageDecoderBindings decoder_bindings(engine::NativeDataImage& data,
                                      const engine::LoadedModuleEnvironment& modules,
                                      engine::NativeImageEnvironment images) {
    auto* panorama = data.native_cell<std::byte*>(0x43469234);
    require(panorama && read<std::uint16_t>(panorama) == 0x5A4D &&
                read<std::uint32_t>(panorama + 0x3C) == 0x160,
            "Required Panorama image-codec module is not loaded");
    const auto* nt = panorama + 0x160;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == 8 && read<std::uint16_t>(nt + 0x18) == 0x10B &&
                read<std::uint32_t>(nt + 8) == 0x651F4636 &&
                read<std::uint32_t>(nt + 80) == 0x2C6000 &&
                read<std::uint32_t>(nt + 88) == 0x26E92E,
            "Unsupported Panorama image-codec build identity");
    ImageDecoderBindings result;
    // These build RVAs are independently named by the supplied loader's
    // extra() binding phase and the actual working function bodies. They are
    // genuine Panorama library entries, not recovered client implementations.
    result.enqueue =
        code_entry(panorama, 0x9E220,
                   {0x55, 0x8B, 0xEC, 0x51, 0x8B, 0xC1, 0x53, 0x56, 0x57, 0x8B, 0x98, 0x88, 0, 0,
                    0,    0x89, 0x45, 0xFC, 0x8D, 0x4B, 4,    0xFF, 0x15, -1,   -1,   -1,   -1});
    result.execute =
        code_entry(panorama, 0x9A320,
                   {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x10, 0x53, 0x8B, 0xD9, 0x8B, 0x0D, -1, -1,
                    -1,   -1,   0x56, 0x89, 0x5D, 0xF4, 0x8B, 0x81, 0x0C, 0x10, 0,    0});
    result.jpeg = code_entry(panorama, 0xA1B80, {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x20, 0x89, 0x4D,
                                                 0xF4, 0x0F, 0x57, 0xC0, 0x8B, 0x0D, -1,   -1,
                                                 -1,   -1,   0x8B, 0xC2, 0x53, 0x56, 0x0F, 0x11});
    result.png = code_entry(panorama, 0xA3C80,
                            {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x2C, 0x53, 0x56, 0x57, 0x6A, 8,  0x8B,
                             0xF1, 0x8B, 0xFA, 0x6A, 0,    0x56, 0xE8, -1,   -1,   -1,   -1, 0x83});
    result.svg = code_entry(panorama, 0xAB6F0,
                            {0x55, 0x8B, 0xEC, 0x81, 0xEC, 0xB8, 0,    0, 0,    0x53, 0x8B, 0x5D,
                             0x0C, 0x56, 0x57, 0x8B, 0x7D, 0x10, 0x8B, 3, 0x52, 0x89, 0x45, 0xFC});
    auto* tier0 = static_cast<std::byte*>(engine::find_loaded_module(
        modules, engine::loaded_module_name_hash(u"tier0.dll", 9), true, true));
    require(tier0 != nullptr, "Image codecs require the already-loaded tier0.dll");
    auto* slot =
        engine::resolve_pe_export(tier0, engine::netvar_name_hash("g_pMemAlloc"), 0, true, images);
    require(slot != nullptr, "tier0.dll does not export g_pMemAlloc");
    result.allocator = read<void*>(slot);
    require(result.allocator != nullptr, "tier0 g_pMemAlloc is not initialized");
    result.render_context_source = data.native_cell<void*>(0x43468E98);
    require(result.render_context_source != nullptr, "Image codecs require PanoramaUIEngine001");
    return result;
}
IconBindings checked_bindings(client::ClientRuntimeStorage& storage, StartupRenderer& renderer) {
    if (storage.data().native_cell<RendererContainer*>(0x4347690C) != &renderer.renderer() ||
        &renderer.dpi() != &storage.menu_startup_data().dpi)
        throw std::invalid_argument(
            "Renderer assets must borrow the canonical renderer and menu DPI");
    return initialize_build_icon_bindings(storage.data());
}
IconBindings checked_bindings(client::ClientRuntimeStorage& storage, StartupRenderer& renderer,
                              IconBindings bindings) {
    auto& data = storage.data();
    const auto& images = bindings.images;
    auto* const filesystem = data.native_cell<std::byte*>(0x43467F68);
    if (data.native_cell<RendererContainer*>(0x4347690C) != &renderer.renderer() ||
        &renderer.dpi() != &storage.menu_startup_data().dpi || !filesystem ||
        bindings.filesystem != filesystem + 4 ||
        &bindings.item_system != &data.native_cell<void*>(0x43476424) ||
        &bindings.item_system_factory !=
            &data.native_cell<IconBindings::ItemSystemFactory>(0x43468B9C) ||
        !bindings.item_system_factory || !images.render_context_source || !images.allocator ||
        !images.enqueue || !images.execute || !images.svg || !images.png || !images.jpeg ||
        images.render_context_source != data.native_cell<void*>(0x43468E98) ||
        images.allocator != data.native_cell<void*>(0x43467204) ||
        images.enqueue != data.native_cell<std::uintptr_t>(0x4346D67C) ||
        images.execute != data.native_cell<std::uintptr_t>(0x4346D680) ||
        images.jpeg != data.native_cell<std::uintptr_t>(0x4346D684) ||
        images.png != data.native_cell<std::uintptr_t>(0x4346D688) ||
        images.svg != data.native_cell<std::uintptr_t>(0x4346D68C))
        throw std::invalid_argument(
            "Renderer assets require the complete canonical acquired engine bindings");
    return bindings;
}
}
IconBindings initialize_build_icon_bindings(engine::NativeDataImage& data,
                                            const engine::LoadedModuleEnvironment& modules,
                                            engine::NativeImageEnvironment images) {
    auto* filesystem = data.native_cell<std::byte*>(0x43467F68);
    require(filesystem != nullptr, "Renderer icons require VFileSystem017");
    auto& factory = data.native_cell<IconBindings::ItemSystemFactory>(0x43468B9C);
    require(factory != nullptr, "Renderer weapon icons require the actual ItemSystem factory");
    const auto images_bound = decoder_bindings(data, modules, images);
    // Publish genuine acquired functions for the same native-data consumers.
    // ItemSystem remains a live reference shared with4341F57F's publication.
    data.native_cell<void*>(0x43467204) = images_bound.allocator;
    data.native_cell<std::uintptr_t>(0x4346D67C) = images_bound.enqueue;
    data.native_cell<std::uintptr_t>(0x4346D680) = images_bound.execute;
    data.native_cell<std::uintptr_t>(0x4346D684) = images_bound.jpeg;
    data.native_cell<std::uintptr_t>(0x4346D688) = images_bound.png;
    data.native_cell<std::uintptr_t>(0x4346D68C) = images_bound.svg;
    return {filesystem + 4, data.native_cell<void*>(0x43476424), factory, images_bound};
}
IconBindings initialize_build_icon_bindings(engine::NativeDataImage& data) {
    return initialize_build_icon_bindings(data, engine::native_loaded_module_environment(),
                                          engine::native_image_environment());
}
struct RendererAssets::Data {
    client::ClientRuntimeStorage& storage;
    StartupRenderer* startup = nullptr;
    std::optional<RendererDeviceListener> owned_listener;
    RendererDeviceListener* listener;
    IconCache cache;
    StartupIcons icons;
    explicit Data(client::ClientRuntimeStorage& owner)
        : storage(owner),
          owned_listener(std::in_place, owner.data().native_cell<RendererContainer*>(0x4347690C)),
          listener(&*owned_listener), icons{owner.data().native_cell<std::int32_t>(0x43468E3C),
                                            owner.data().native_cell<std::int32_t>(0x4346842C),
                                            owner.data().native_cell<std::int32_t>(0x43467DA8)} {}
    Data(client::ClientRuntimeStorage& owner, StartupRenderer& renderer)
        : Data(owner, renderer, checked_bindings(owner, renderer)) {}
    Data(client::ClientRuntimeStorage& owner, StartupRenderer& renderer, IconBindings acquired)
        : storage(owner), startup(&renderer), listener(&renderer.device_listener()),
          cache(renderer.renderer().backend(), checked_bindings(owner, renderer, acquired)),
          icons{owner.data().native_cell<std::int32_t>(0x43468E3C),
                owner.data().native_cell<std::int32_t>(0x4346842C),
                owner.data().native_cell<std::int32_t>(0x43467DA8)} {}
};
RendererAssets::RendererAssets(client::ClientRuntimeStorage& storage)
    : data_(std::make_unique<Data>(storage)) {}
RendererAssets::RendererAssets(client::ClientRuntimeStorage& storage, StartupRenderer& renderer)
    : data_(std::make_unique<Data>(storage, renderer)) {}
RendererAssets::RendererAssets(client::ClientRuntimeStorage& storage, StartupRenderer& renderer,
                               IconBindings bindings)
    : data_(std::make_unique<Data>(storage, renderer, bindings)) {}
void RendererAssets::bind_renderer(StartupRenderer& renderer) {
    bind_renderer(renderer, checked_bindings(data_->storage, renderer));
}
void RendererAssets::bind_renderer(StartupRenderer& renderer, IconBindings bindings) {
    if (data_->startup)
        throw std::logic_error("The canonical renderer assets were already bound");
    if (&renderer.device_listener() != data_->listener)
        throw std::invalid_argument("Late renderer must borrow the same early device listener");
    const auto acquired = checked_bindings(data_->storage, renderer, bindings);
    data_->cache.bind_runtime(renderer.renderer().backend(), acquired);
    data_->startup = &renderer;
}
bool RendererAssets::renderer_bound() const noexcept {
    return data_->startup != nullptr;
}
RendererAssets::~RendererAssets() = default;
client::ClientRuntimeStorage& RendererAssets::storage() const noexcept {
    return data_->storage;
}
StartupRenderer& RendererAssets::startup_renderer() const {
    if (!data_->startup)
        throw std::logic_error("Renderer assets have not acquired their late renderer");
    return *data_->startup;
}
RendererContainer& RendererAssets::renderer() const {
    return startup_renderer().renderer();
}
RendererDeviceListener& RendererAssets::device_listener() const noexcept {
    return *data_->listener;
}
IconCache& RendererAssets::icon_cache() noexcept {
    return data_->cache;
}
StartupIcons& RendererAssets::startup_icons() noexcept {
    return data_->icons;
}
}

// ============================================================================
// Subcomponent: startup_icons.cpp
// ============================================================================

namespace skeet::ui {
void initialize_startup_icons(IconCache& cache, StartupIcons& icons) { // 433283A5
    icons.grenade =
        cache.load_svg("materials/panorama/images/icons/equipment/hegrenade.svg", 0, -1);
    icons.inferno = cache.load_svg("materials/panorama/images/icons/equipment/inferno.svg", 0, -1);
    icons.bomb = cache.load_svg("materials/panorama/images/icons/ui/bomb_c4.svg", 0, -1);
}
}

// ============================================================================
// Subcomponent: startup_renderer.cpp
// ============================================================================

namespace skeet::ui {
namespace {
constexpr std::array<StartupFontRecipe, 7> initial_fonts{{{u"Verdana", 9, 2},
                                                          {u"Verdana", 9, 3},
                                                          {u"Calibri", 24, 3},
                                                          {u"Segoe UI Symbol", 12, 2},
                                                          {u"Small Fonts", 7, 4},
                                                          {u"Segoe UI Symbol", 24, 2},
                                                          {u"Malgun Gothic", 10, 2}}};
struct Region {
    std::byte* first;
    std::size_t size;
};
struct Image {
    std::byte* base;
    Region text{}, rdata{}, data{};
    static bool contains(Region region, const void* pointer, std::size_t size) noexcept {
        const auto at = reinterpret_cast<std::uintptr_t>(pointer);
        const auto begin = reinterpret_cast<std::uintptr_t>(region.first);
        return at >= begin && size <= region.size && at - begin <= region.size - size;
    }
};
Image checked_image(std::byte* base, std::uint32_t nt_offset, std::uint32_t timestamp,
                    std::uint32_t image_size, std::uint32_t checksum) {
    require(base && read<std::uint16_t>(base) == 0x5A4D,
            "Renderer module is not a loaded DOS image");
    require(read<std::uint32_t>(base + 0x3C) == nt_offset,
            "Unsupported renderer module NT-header location");
    const auto* nt = base + nt_offset;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 20) == 0xE0 && read<std::uint16_t>(nt + 24) == 0x10B,
            "Renderer bindings require the observed PE32 module layout");
    require(read<std::uint32_t>(nt + 8) == timestamp &&
                read<std::uint32_t>(nt + 80) == image_size &&
                read<std::uint32_t>(nt + 88) == checksum,
            "Unsupported renderer module build identity");
    Image result{base};
    const auto count = read<std::uint16_t>(nt + 6);
    require(count == 6, "Unsupported renderer module section count");
    for (std::uint16_t index = 0; index < count; ++index) {
        const auto* section = nt + 0xF8 + index * 40;
        const auto rva = read<std::uint32_t>(section + 12), size = read<std::uint32_t>(section + 8);
        require(rva <= image_size && size <= image_size - rva,
                "Renderer PE section exceeds its loaded image");
        const auto flags = read<std::uint32_t>(section + 36);
        if (!std::memcmp(section, ".text\0\0\0", 8)) {
            require((flags & 0x22000000u) == 0x20000000u,
                    "Renderer text is not executable and nondiscardable");
            result.text = {base + rva, size};
        } else if (!std::memcmp(section, ".rdata\0\0", 8))
            result.rdata = {base + rva, size};
        else if (!std::memcmp(section, ".data\0\0\0", 8))
            result.data = {base + rva, size};
    }
    require(result.text.first && result.rdata.first && result.data.first,
            "Renderer PE sections are missing");
    return result;
}
IDirect3DDevice9** shader_device_slot(const engine::NativeDataImage& data) {
    // ShaderDevice001::IsUsingGraphics: XOR EAX,EAX; CMP [device],EAX;
    // SETNE AL; RET. Decode its live absolute operand, rather than retaining
    // the old module address5BA762C0 or the working capture's6A8262C0.
    const auto image = checked_image(data.native_cell<std::byte*>(0x4346A604), 0x140, 0x63DC1158,
                                     0xCE000, 0x9DC6B);
    auto* object = data.native_cell<void*>(0x4346A33C);
    require(Image::contains(image.data, object, sizeof(void*)),
            "ShaderDevice001 is outside its loaded module data");
    auto** table = *static_cast<void***>(object);
    require(Image::contains(image.rdata, table, 7 * sizeof(void*)),
            "ShaderDevice001 vtable is outside module RTTI data");
    const auto* code = static_cast<const std::byte*>(table[6]);
    static constexpr std::array<int, 12> pattern{0x33, 0xC0, 0x39, 0x05, -1,   -1,
                                                 -1,   -1,   0x0F, 0x95, 0xC0, 0xC3};
    require(Image::contains(image.text, code, pattern.size()) && matches(code, pattern),
            "ShaderDevice001 virtual6 is not the proven graphics-state accessor");
    auto** slot =
        reinterpret_cast<IDirect3DDevice9**>(std::uintptr_t(read<std::uint32_t>(code + 4)));
    require(Image::contains(image.data, slot, sizeof(void*)),
            "Shader device operand is outside module data");
    require(*slot != nullptr, "The game D3D9 device has not been created");
    return slot;
}
void** overlay_bridge_holder(const engine::NativeDataImage& data) {
    auto* module = data.native_cell<std::byte*>(0x43468D08);
    if (!module)
        return nullptr; //43384182 explicitly accepts a missing bridge.
    const auto identity = engine::require_overlay_build_identity(module);
    const auto image =
        checked_image(module, 0x130, identity.timestamp, 0x190000, identity.checksum);
    // CD3D9Renderer's genuine frame producer reads the active renderer slot,
    // tests it, and publishes EBX to that SAME slot before copying+A4/+A8.
    static constexpr std::array<int, 32> pattern{
        0x8B, 0x0D, -1, -1, -1, -1, 0x85, 0xC9, 0x0F, 0x85, -1,   -1, -1, -1, 0xDD, 0xD8,
        0x89, 0x1D, -1, -1, -1, -1, 0xF3, 0x0F, 0x10, 0x83, 0xA4, 0,  0,  0,  0xF3, 0x0F};
    const std::byte* found = nullptr;
    for (std::size_t i = 0; i + pattern.size() <= image.text.size; ++i) {
        const auto* code = image.text.first + i;
        if (!matches(code, pattern))
            continue;
        require(!found, "Active overlay renderer producer is ambiguous");
        found = code;
    }
    require(found, "Active overlay renderer producer was not found");
    const auto address = read<std::uint32_t>(found + 2);
    require(address == read<std::uint32_t>(found + 18),
            "Overlay renderer load/store operands differ");
    auto** holder = reinterpret_cast<void**>(std::uintptr_t(address));
    require(Image::contains(image.data, holder, 12),
            "Overlay renderer holder is outside module data");
    // Its current value is nullable and remains owned by Steam. FrameRuntime
    // rereads the holder each invocation; no captured object is instantiated.
    return holder;
}
std::shared_ptr<engine::NativeDataImage>
checked_owner(std::shared_ptr<engine::NativeDataImage> data, GuiDpiState& dpi) {
    auto& image = engine::require_native_data_image(data);
    if (&dpi.selection != &image.native_cell<std::int32_t>(0x43475A94))
        throw std::invalid_argument("Renderer and menu must share the canonical DPI selection");
    if (image.native_cell<RendererContainer*>(0x4347690C))
        throw std::logic_error("The canonical startup renderer already exists");
    return data;
}
void initialize_window_size(RendererBackend& backend) { //4348F887..4348F8D1
    D3DDEVICE_CREATION_PARAMETERS parameters{};
    (void)backend.device()->GetCreationParameters(&parameters);
    RECT rectangle;
    if (GetClientRect(parameters.hFocusWindow, &rectangle)) {
        backend.screen_width = std::bit_cast<std::int32_t>(std::uint32_t(rectangle.right) -
                                                           std::uint32_t(rectangle.left));
        backend.screen_height = std::bit_cast<std::int32_t>(std::uint32_t(rectangle.bottom) -
                                                            std::uint32_t(rectangle.top));
    } else {
        backend.screen_width = 1024;
        backend.screen_height = 768;
    }
}
}
std::span<const StartupFontRecipe, 7> startup_font_recipes() noexcept {
    return initial_fonts;
}
void initialize_startup_fonts(RendererContainer& renderer) { //font portion4348F2C1..4348FA7D
    std::array<FontDefinition, 7> definitions;
    for (std::size_t i = 0; i < definitions.size(); ++i)
        definitions[i] = {std::u16string(initial_fonts[i].face), initial_fonts[i].height,
                          initial_fonts[i].flags};
    renderer.configure_fonts(definitions);
    // Native makes these edits AFTER all seven default FontRuntime objects.
    // They affect subsequent scaled fonts; the defaults above stay intact.
    renderer.font_definitions[4] = {u"Verdana", 7, 2};
    renderer.font_definitions[6].height = 9;
}
BuildRendererBindings discover_build_renderer_bindings(const engine::NativeDataImage& data) {
    return {shader_device_slot(data), overlay_bridge_holder(data)};
}
StartupRenderer::StartupRenderer(std::shared_ptr<engine::NativeDataImage> data, GuiDpiState& dpi)
    : data_(checked_owner(std::move(data), dpi)), dpi_(dpi),
      bindings_(discover_build_renderer_bindings(*data_)), renderer_(*bindings_.device_slot),
      owned_listener_(std::in_place, renderer_), listener_(&*owned_listener_),
      frame_(renderer_, bindings_.bridge_holder, data_->native_cell<std::uint32_t>(0x4347C9FC)) {
    initialize();
}
StartupRenderer::StartupRenderer(std::shared_ptr<engine::NativeDataImage> data, GuiDpiState& dpi,
                                 RendererDeviceListener& listener)
    : data_(checked_owner(std::move(data), dpi)), dpi_(dpi),
      bindings_(discover_build_renderer_bindings(*data_)), renderer_(*bindings_.device_slot),
      listener_(&listener),
      frame_(renderer_, bindings_.bridge_holder, data_->native_cell<std::uint32_t>(0x4347C9FC)) {
    if (!listener.follows(data_->native_cell<RendererContainer*>(0x4347690C)))
        throw std::invalid_argument(
            "Startup renderer must borrow its canonical early device listener");
    initialize();
}
void StartupRenderer::initialize() {
    initialize_window_size(renderer_.backend());
    initialize_startup_fonts(renderer_);
    data_->native_cell<void**>(0x43468350) = bindings_.bridge_holder;
    data_->native_cell<RendererContainer*>(0x4347690C) = &renderer_;
}
StartupRenderer::~StartupRenderer() {
    // Existing post-initialization/unload owns device-listener registration.
    // This owner must outlive that registration and the main menu bindings.
    auto& current = data_->native_cell<RendererContainer*>(0x4347690C);
    if (current == &renderer_) {
        renderer_.destroy(); //43336C83: destroy the front before clearing it.
        current = nullptr;
    }
}
}

// ============================================================================
// Subcomponent: render_queue.cpp
// ============================================================================

namespace skeet::ui {
static_assert(std::endian::native == std::endian::little);
namespace {
std::int32_t sub(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) - std::uint32_t(b));
}
}

std::uint32_t subtract_color(std::uint32_t color, std::uint32_t amount) noexcept { // 0x433E87BD
    std::uint32_t result = 0;
    for (unsigned shift = 0; shift != 32; shift += 8) {
        const auto a = (color >> shift) & 255;
        const auto b = (amount >> shift) & 255;
        result |= (a > b ? a - b : 0) << shift;
    }
    return result;
}
std::uint32_t add_color(std::uint32_t color, std::uint32_t amount) noexcept { // 0x43383DE6
    std::uint32_t result = 0;
    for (unsigned shift = 0; shift != 32; shift += 8) {
        const auto sum = ((color >> shift) & 255) + ((amount >> shift) & 255);
        result |= (sum < 255 ? sum : 255) << shift;
    }
    return result;
}

// Complete helper 0x433D0F44..0x433D0F91. Alpha is preserved exactly.
std::uint32_t desaturate_color(std::uint32_t color, bool enabled) noexcept {
    if (!enabled)
        return color;
    const auto red = color & 255;
    const auto green = (color >> 8) & 255;
    const auto blue = (color >> 16) & 255;
    const auto intensity = (2 * red + 3 * green + blue) / 6;
    return (color & 0xff000000U) | intensity | (intensity << 8) | (intensity << 16);
}

// 0x433AB0FA. Storage ownership is ordinary C++; the command format is unchanged.
std::byte* RenderQueue::append(std::uint32_t opcode, std::uint32_t payload_size) {
    const auto payload_aligned = (std::uint64_t(payload_size) + 3) & ~std::uint64_t(3);
    const auto offset = commands.size();
    const auto end = offset + 4 + payload_aligned;
    if (opcode > 31 || end > 0x07ffffff)
        throw std::length_error("Render command exceeds its header format");
    commands.resize(static_cast<std::size_t>(end), std::byte{});
    const std::uint32_t header = static_cast<std::uint32_t>(end) | (opcode << 27);
    std::memcpy(commands.data() + offset, &header, sizeof(header));
    return commands.data() + offset + 4;
}

// Complete native writers; field order is the original queue wire format.
void RenderQueue::rectangle(RectangleCommand command) { // 0x433EB76A
    command.color = desaturate_color(command.color, desaturate);
    std::memcpy(append(0, sizeof(command)), &command, sizeof(command));
}
void RenderQueue::rectangle_outline(RectangleCommand c, std::int32_t thickness) { // 0x433C14DB
    rectangle({c.x, c.y, c.width, thickness, c.color});
    rectangle({c.x, c.y, thickness, c.height, c.color});
    rectangle({add(sub(c.x, thickness), c.width), c.y, thickness, c.height, c.color});
    rectangle({c.x, add(sub(c.y, thickness), c.height), c.width, thickness, c.color});
}
void RenderQueue::outlined_rectangle(RectangleCommand c, std::uint32_t border_color,
                                     std::int32_t thickness) { // 0x433F16D1
    rectangle({add(c.x, 1), add(c.y, 1), sub(c.width, 2), sub(c.height, 2), c.color});
    c.color = border_color;
    rectangle_outline(c, thickness);
}
void RenderQueue::four_color_gradient(FourColorGradientCommand command) { // 0x4335BDB5
    for (auto& color : command.colors)
        color = desaturate_color(color, desaturate);
    std::memcpy(append(1, sizeof(command)), &command, sizeof(command));
}
void RenderQueue::vertical_gradient(GradientCommand command) { // 0x433B66C1
    command.first_color = desaturate_color(command.first_color, desaturate);
    command.second_color = desaturate_color(command.second_color, desaturate);
    std::memcpy(append(2, sizeof(command)), &command, sizeof(command));
}
void RenderQueue::horizontal_gradient(GradientCommand command) { // 0x4336AF92
    command.first_color = desaturate_color(command.first_color, desaturate);
    command.second_color = desaturate_color(command.second_color, desaturate);
    std::memcpy(append(3, sizeof(command)), &command, sizeof(command));
}
void RenderQueue::line(LineCommand command) { // 0x433B1863
    command.color = desaturate_color(command.color, desaturate);
    std::memcpy(append(4, sizeof(command)), &command, sizeof(command));
}
void RenderQueue::polyline(std::span<const LinePoint> points, std::uint32_t color) { // 0x43369793
    static_assert(sizeof(LinePoint) == 8);
    if (points.size() > (0x07ffffffU - 20) / sizeof(LinePoint))
        throw std::length_error("Polyline exceeds its command format");
    color = desaturate_color(color, desaturate);
    const auto count = static_cast<std::uint32_t>(points.size());
    auto* payload = append(5, count * sizeof(LinePoint) + 16);
    std::memcpy(payload, &color, sizeof(color));
    std::memcpy(payload + 4, &count, sizeof(count));
    if (!points.empty())
        std::memcpy(payload + 8, points.data(), points.size_bytes());
    // The original reserves eight additional zero bytes after the points.
}
void RenderQueue::triangle(TriangleCommand command) { // 0x433ADAF1
    command.color = desaturate_color(command.color, desaturate);
    std::memcpy(append(6, sizeof(command)), &command, sizeof(command));
}
void RenderQueue::circle(CircleCommand command) { // 0x4338A58B
    command.color = desaturate_color(command.color, desaturate);
    std::memcpy(append(9, sizeof(command)), &command, sizeof(command));
}
void RenderQueue::circle_outline(CircleOutlineCommand command) { // 0x43396E3D
    command.color = desaturate_color(command.color, desaturate);
    std::memcpy(append(16, sizeof(command)), &command, sizeof(command));
}

// Complete texture queue entry 0x433DF2CF..0x433DF33A, native opcode 11.
void RenderQueue::texture(TextureCommand command) {
    command.color = desaturate_color(command.color, desaturate);
    std::memcpy(append(11, sizeof(command)), &command, sizeof(command));
}
void RenderQueue::text(std::int32_t x, std::int32_t y, std::uint32_t color, std::uint32_t flags,
                       std::int32_t maximum_width, std::u16string_view content) { // 0x433E4A54
    if (content.empty())
        return;
    if (content.size() > (0x07ffffffU - 28) / sizeof(char16_t))
        throw std::length_error("Text exceeds its render command format");
    color = desaturate_color(color, desaturate);
    auto* payload = append(10, static_cast<std::uint32_t>(content.size() * 2 + 24));
    prepare_glyphs(fonts, content, flags);
    const std::array<std::uint32_t, 5> header{std::uint32_t(x), std::uint32_t(y), color, flags,
                                              std::uint32_t(maximum_width)};
    std::memcpy(payload, header.data(), sizeof(header));
    const auto length = static_cast<std::uint16_t>(content.size());
    std::memcpy(payload + 20, &length, sizeof(length));
    std::memcpy(payload + 22, content.data(), content.size() * sizeof(char16_t));
}

void RenderQueue::set_bounds(const std::array<std::int32_t, 4>& value) { // 0x4339CB94
    bounds = value;
    std::memcpy(append(12, sizeof(value)), value.data(), sizeof(value));
}
std::array<std::int32_t, 4> RenderQueue::get_bounds() const noexcept { // 0x433F1E34
    return bounds;
}
void RenderQueue::reset_bounds(bool reset) { // 0x4331ECE5
    bounds_reset = reset;
    *append(13, 1) = static_cast<std::byte>(reset);
}
bool RenderQueue::is_bounds_reset() const noexcept { // 0x433F19B9
    return bounds_reset;
}
void RenderQueue::set_alpha(float value) { // 0x433C12DA
    if (alpha != value) {
        alpha = value;
        std::memcpy(append(14, sizeof(value)), &value, sizeof(value));
    }
}
float RenderQueue::get_alpha() const noexcept { // 0x4334A98A
    return alpha;
}
void RenderQueue::set_depth(float value) { // 0x433B360E
    depth = value;
    std::memcpy(append(15, sizeof(value)), &value, sizeof(value));
}
float RenderQueue::get_depth() const noexcept { // 0x433CFD53
    return depth;
}
void RenderQueue::set_desaturation(bool enabled) noexcept { // 0x4338A5FD
    desaturate = enabled;
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: text_metrics.cpp
// ============================================================================

namespace skeet::ui {

Point measure_text(RenderQueue& queue, std::u16string_view content,
                   std::uint32_t flags) { // 0x4341825C / 0x4332B578 / 0x4338AA26
    prepare_glyphs(queue.fonts, content, flags);
    if (!queue.fonts)
        throw std::logic_error("Text measurement requires a font backend");
    auto& fonts = select_fonts(*queue.fonts, flags);
    const auto index = (flags & 2) ? 2 : (flags & 1) ? 1 : (flags & 4) ? 4 : 0; // 0x4332AF73
    auto& root = fonts[index];
    std::int32_t height = 0;
    if (root.cache) {
        if (!root.cache->height)
            throw std::logic_error("Font provider has no height callback");
        height = root.cache->height(root.cache->context);
    }
    Point size{0, height};
    std::int32_t width = 0;
    for (std::size_t i = 0; i < content.size(); ++i) {
        const auto character = content[i];
        if (character == u'\n') {
            width = 0;
            size.y = add(size.y, height);
        } else if (character == u'\a' && content.size() - i > 8) {
            i += 8;
        } else {
            for (auto* link = &root; link; link = link->next) {
                if (!link->cache)
                    continue;
                if (!link->cache->metrics)
                    throw std::logic_error("Font provider has no glyph metrics callback");
                GlyphMetrics metrics;
                if (link->cache->metrics(link->cache->context, character, metrics)) {
                    width = add(width, add(metrics.advance_first, metrics.advance_second));
                    if (width > size.x)
                        size.x = width;
                    break;
                }
            }
        }
    }
    return size;
}
}

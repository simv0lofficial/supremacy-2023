#pragma once
#include "font_cache.hpp"
#include "core.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace skeet::ui {
class RendererContainer;

struct RectangleCommand {
    std::int32_t x, y, width, height;
    std::uint32_t color;
};
struct GradientCommand {
    std::int32_t x, y, width, height;
    std::uint32_t first_color, second_color;
};
struct FourColorGradientCommand {
    std::int32_t x, y, width, height;
    // Native vertex order; corner orientation is not yet recovered.
    std::array<std::uint32_t, 4> colors;
};
struct LinePoint {
    std::int32_t x, y;
};
struct LineCommand {
    std::int32_t from_x, from_y, to_x, to_y;
    std::uint32_t color;
};
struct TriangleCommand {
    std::int32_t first_x, first_y, second_x, second_y, third_x, third_y;
    std::uint32_t color;
};
struct CircleCommand {
    std::int32_t x, y;
    std::uint32_t color;
    float radius, rotation, fraction;
};
struct CircleOutlineCommand {
    std::int32_t x, y;
    std::uint32_t color;
    float radius, rotation, fraction, thickness;
};

struct TextureCommand {
    std::int32_t x, y;
    std::uint32_t texture;
    std::uint32_t color;
    std::int32_t source_x, source_y, source_width, source_height;
    std::uint32_t mode;
    float scale;
};
static_assert(sizeof(TextureCommand) == 40);

std::uint32_t desaturate_color(std::uint32_t color, bool enabled) noexcept;
std::uint32_t subtract_color(std::uint32_t color, std::uint32_t amount) noexcept;
std::uint32_t add_color(std::uint32_t color, std::uint32_t amount) noexcept;

struct RenderQueue {
    std::vector<std::byte> commands;
    bool desaturate = false;
    bool bounds_reset = false;
    std::array<std::int32_t, 4> bounds{};
    float depth = 0.5f;
    float alpha = 1.0f;
    FontBackend* fonts = nullptr;
    Point screen() const; //433B0530 -> 433F49E6, supplied queue's renderer

    // Header: low 27 bits are the absolute end offset, high 5 bits the opcode.
    std::byte* append(std::uint32_t opcode, std::uint32_t payload_size);
    void rectangle(RectangleCommand command);
    void rectangle_outline(RectangleCommand command, std::int32_t thickness);
    void outlined_rectangle(RectangleCommand command, std::uint32_t border_color,
                            std::int32_t thickness);
    void four_color_gradient(FourColorGradientCommand command);
    void vertical_gradient(GradientCommand command);
    void horizontal_gradient(GradientCommand command);
    void line(LineCommand command);
    void polyline(std::span<const LinePoint> points, std::uint32_t color);
    void triangle(TriangleCommand command);
    void circle(CircleCommand command);
    void circle_outline(CircleOutlineCommand command);
    void texture(TextureCommand command);
    void text(std::int32_t x, std::int32_t y, std::uint32_t color, std::uint32_t flags,
              std::int32_t maximum_width, std::u16string_view content);
    void set_bounds(const std::array<std::int32_t, 4>& value);
    std::array<std::int32_t, 4> get_bounds() const noexcept;
    void reset_bounds(bool reset);
    bool is_bounds_reset() const noexcept;
    void set_alpha(float value);
    float get_alpha() const noexcept;
    void set_depth(float value);
    float get_depth() const noexcept;
    void set_desaturation(bool enabled) noexcept;

private:
    friend class RendererContainer;
    // Borrowed from the container that owns this frame. This never follows
    // the current global renderer after a callback changes that global slot.
    RendererContainer* renderer_ = nullptr;
};

} // namespace skeet::ui

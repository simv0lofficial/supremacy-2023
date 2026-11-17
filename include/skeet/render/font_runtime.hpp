#pragma once
#include "skeet/font_cache.hpp"
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>

namespace skeet::ui {
struct GlyphImage {
    float left = 0, top = 0, right = 0, bottom = 0;
    float u0 = 0, v0 = 0, u1 = 0, v1 = 0;
    std::int32_t advance_first = 0, advance_second = 0;
};
struct GlyphAtlas {
    std::int32_t width = 0, height = 0;
    std::span<const std::uint32_t> pixels;
};

// GDI font, Unicode coverage, glyph records and the growing ARGB atlas.
// Flags: 1 bold, 2 ClearType/ABC widths, 4 one-pixel outline.
class FontRuntime {
public:
    FontRuntime(std::u16string_view face, std::int32_t height, std::uint32_t flags);
    ~FontRuntime();
    FontRuntime(const FontRuntime&) = delete;
    FontRuntime& operator=(const FontRuntime&) = delete;
    bool valid() const noexcept;
    bool supports(char16_t character) const noexcept;
    bool prepare(char16_t character);
    void prepare(std::u16string_view text);
    bool glyph(char16_t character, GlyphImage& out) const noexcept;
    GlyphAtlas atlas() const noexcept;
    void mark_dirty() noexcept;
    bool consume_dirty() noexcept;
    GlyphCache& cache() noexcept;
    std::int32_t height() const noexcept;
    std::int32_t ascent() const noexcept;
    std::int32_t internal_leading() const noexcept;
    std::int32_t external_leading() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    GlyphCache cache_{};
};

std::int32_t font_internal_leading(FontBackend& backend, std::uint32_t index);
std::int32_t font_ascent(FontBackend& backend, std::uint32_t index);
} // namespace skeet::ui

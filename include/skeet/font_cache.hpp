#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace skeet::ui {
class FontRuntime;
struct GlyphMetrics {
    std::int32_t advance_first = 0;
    std::int32_t advance_second = 0;
};
struct GlyphCache {
    void* context;
    bool (*prepare)(void*, char16_t);
    std::int32_t (*height)(void*) = nullptr;
    bool (*metrics)(void*, char16_t, GlyphMetrics&) = nullptr;
    std::int32_t (*internal_leading)(void*) = nullptr;
    std::int32_t (*ascent)(void*) = nullptr;
    FontRuntime* runtime = nullptr;
};
struct FontLink {
    GlyphCache* cache = nullptr;
    FontLink* next = nullptr;
};
using FontSet = std::array<FontLink, 7>;
struct FontBackend {
    float scale = 1.0f;
    FontSet default_fonts;
    FontSet scaled_fonts;
};
FontSet& select_fonts(FontBackend& backend, std::uint32_t flags) noexcept;
void prepare_glyphs(FontBackend* backend, std::u16string_view text, std::uint32_t flags);
} // namespace skeet::ui

#pragma once
#include "skeet/core.hpp"
#include "skeet/render_queue.hpp"

namespace skeet::esp {

struct WeaponTextProvider {
    void* context;
    // Engine item-schema lookup (factory -> schema -> definition -> name).
    // A missing interface/definition/name is represented by a null result.
    const char* (*schema_name)(void*, std::int32_t definition);
    // Native localization boundary 0x43397087. Null selects the native
    // schema-token path, which removes its 13-byte #SFUI_WPNHUD_ prefix.
    const char16_t* (*localize)(void*, const char* token);
};
struct WeaponTextBindings {
#if defined(_M_IX86) || defined(__i386__)
    using ItemSystemFactory = void*(__cdecl*)();
#else
    using ItemSystemFactory = void* (*)();
#endif
    void*& item_system; //canonical43476424, shared with post-initialization/icons
    ItemSystemFactory const& item_system_factory; //43468B9C
    void* const& localization; //4346A31C, actual Localize_001
};
// Concrete engine adapter: item-system slot0 -> schema slot4 -> item slot2;
// localization slot11 is the native 0x43397087 lookup through 0x433E533F.
WeaponTextProvider weapon_text_provider(WeaponTextBindings& bindings) noexcept;
// Native 0x433F25A1. Early lookup failure leaves measured_size unchanged.
// Schema labels contain fewer than 64 characters after prefix
// removal/localization; this function rejects violations of that contract.
void draw_weapon_text(ui::RenderQueue& queue, WeaponTextProvider provider, std::int32_t definition,
                      Point position, std::uint32_t color, Point* measured_size = nullptr);

} // namespace skeet::esp

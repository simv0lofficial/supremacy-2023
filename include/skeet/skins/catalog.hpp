#pragma once
#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace skeet::skins {
struct CatalogChoice {
    std::uint32_t value;
    std::u16string name;
};
// Explicit engine schema/localization boundary: no baked-in replacement catalog.
class Catalog {
public:
    virtual ~Catalog() = default;
    virtual bool available() = 0;
    virtual std::uint32_t maximum_paintkit() const = 0;
    virtual std::span<const std::uint16_t> paintkits(std::uint16_t item) = 0;
    virtual std::optional<std::u16string> paintkit_name(std::uint32_t id) = 0;
    virtual std::span<const CatalogChoice> glove_models() = 0;
    virtual std::span<const CatalogChoice> agent_models(std::int32_t team) = 0;
};
class Refresh {
public:
    virtual ~Refresh() = default;
    // 0x4334D9E5 and 0x4346A8A0 scheduling, implemented by the engine adapter.
    virtual void invalidate_weapon_models() = 0;
    virtual float current_time() const = 0;
    virtual void schedule_full_update(float time) = 0;
    // 0x433821B6 queries live team; disconnected/demo/HLTV use team 2.
    virtual std::int32_t agent_team() const = 0;
};
} // namespace skeet::skins

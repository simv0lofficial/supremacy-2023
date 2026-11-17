#include "entity_detail.hpp"
#include <array>
#include <cstring>

namespace skeet::script::entity_detail {
namespace {
struct FieldMatch {
    const engine::DataMapField* field;
    std::uint32_t base;
};
#pragma pack(push, 4)
struct OptimizedField {
    std::array<std::byte, 0x30> other;
    std::uint32_t extent;
    std::array<std::byte, 8> trailing;
};
struct OptimizedVector {
    const OptimizedField* elements;
    std::uint32_t allocation_count, growth_size;
    std::int32_t count;
    std::array<std::byte, 0x20> other;
};
struct OptimizedMap {
    std::array<OptimizedVector, 2> vectors;
};
#pragma pack(pop)
static_assert(sizeof(OptimizedField) == 0x3c);
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(OptimizedVector, count) == 0xc);
static_assert(sizeof(OptimizedVector) == 0x30);
#endif
// Complete433B317E: the CRT comparison is deliberately case insensitive;
// embedded offsets accumulate, while inheritance retains the caller's base.
bool find_field(engine::DataMap* map, const char* name, std::uint32_t base, FieldMatch& out) {
    for (; map; map = map->base) {
        for (std::int32_t i = 0; i < map->field_count; ++i) {
            const auto& field = map->fields[i];
            if (field.name && !_stricmp(field.name, name)) {
                out = {&field, base};
                return true;
            }
            if (field.type == 10 && find_field(field.embedded, name, base + field.offset, out))
                return true;
        }
    }
    return false;
}
}
bool data_map_property(const char* name, engine::DataMap* map, EntityProperty& out) {
    constexpr std::array<std::uint8_t, 30> kinds{8, 1, 4, 2, 2, 0, 9, 10, 9, 0, 8, 0, 8, 11, 8,
                                                 2, 1, 0, 4, 4, 8, 8, 8,  8, 8, 1, 0, 0, 3,  7};
    FieldMatch found{};
    if (!find_field(map, name, 0, found))
        return false;
    const auto type = std::uint32_t(found.field->type);
    if (type >= kinds.size() || kinds[type] == 8)
        return false;
    out.type = kinds[type];
    out.offset = found.base + found.field->offset;
    return true;
}
// Complete433C046C. The engine's optimized map owns two native0x30-byte
// vectors; each flattened descriptor keeps its object extent at +0x30.
std::uint32_t maximum_data_map_offset(engine::DataMap* map) {
    if (!map)
        return 0;
    const auto* optimized = static_cast<const OptimizedMap*>(map->optimized);
    if (!optimized)
        return 0;
    std::uint32_t maximum = 0;
    for (int vector = 0; vector < 2; ++vector) {
        const auto& descriptor = optimized->vectors[vector];
        for (std::int32_t i = 0; i < descriptor.count; ++i) {
            const auto offset = descriptor.elements[i].extent;
            if (offset > maximum)
                maximum = offset;
        }
    }
    return maximum;
}
} // namespace skeet::script::entity_detail

#include "entity_detail.hpp"
#include <array>
#include <cstring>

namespace skeet::script::entity_detail {
// Complete native433FB78D. The graph is the engine-owned receive metadata;
// output retains the native twelve-byte representation and truncation rules.
bool receive_property(const char* name, const engine::RecvTable& table, std::uint32_t base,
                      EntityProperty& out, const engine::RecvVarProxy* byte_proxy,
                      const engine::RecvVarProxy* short_proxy) {
    constexpr std::array<std::uint16_t, 12> sizes{4, 4, 12, 8, 0, 0, 0, 8, 0, 1, 2, 4};
    for (std::int32_t index = 0; index < table.property_count; ++index) {
        const auto& property = table.properties[index];
        const auto offset = base + std::uint32_t(property.offset);
        const bool match = !std::strcmp(name, property.name);
        if (match && property.type != 6) {
            out.type = std::uint8_t(property.type);
            out.offset = offset;
            if (property.type == 4)
                out.element_count = std::uint16_t(std::uint32_t(property.string_buffer_size) > 255u
                                                      ? 255u
                                                      : property.string_buffer_size);
            else if (property.type == 0) {
                if (byte_proxy && *byte_proxy == property.proxy)
                    out.type = 9;
                else if (short_proxy && *short_proxy == property.proxy)
                    out.type = 10;
            }
            return true;
        }
        if (property.type != 6)
            continue;
        const auto* child = property.data_table;
        if (match && child && property.data_table_proxy) {
            if (child->property_count > 1 && property.offset == 0) {
                const auto* extra = child->properties[0].extra_data;
                if (!extra)
                    return false;
                out.type = 254;
                out.element_type = std::uint8_t(child->properties[1].type);
                out.element_stride = read<std::uint8_t>(extra, 0x10);
                out.element_count = read<std::uint8_t>(extra, 0x18);
                out.offset = read<std::uint32_t>(extra, 0x14);
                return true;
            }
            std::int32_t element = 0;
            for (; element < child->property_count; ++element) {
                const auto& candidate = child->properties[element];
                if (!candidate.parent_array_property_name || std::strlen(candidate.name) > 4)
                    break;
            }
            if (element >= child->property_count) {
                out.type = 6;
                out.element_type = std::uint8_t(child->properties[0].type);
                if (child->property_count >= 2)
                    out.element_stride = std::uint8_t(child->properties[1].offset);
                else if (out.element_type < sizes.size())
                    out.element_stride = sizes[out.element_type];
                out.element_count = std::uint8_t(child->property_count);
                out.offset = offset;
                return true;
            }
        }
        if (child->property_count > 0 &&
            receive_property(name, *child, offset, out, byte_proxy, short_proxy))
            return true;
    }
    return false;
}
} // namespace skeet::script::entity_detail

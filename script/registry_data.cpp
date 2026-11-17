#include "skeet/script/registry.hpp"
#include <cstring>
#include <stdexcept>

namespace skeet::script {
namespace {
#include "registry_catalog.inc"

std::uint32_t word(std::span<const std::byte> bytes, std::size_t offset) {
    if (offset > bytes.size() || bytes.size() - offset < 4)
        throw std::out_of_range("Lua registry word exceeds its native buffer");
    std::uint32_t value;
    std::memcpy(&value, bytes.data() + offset, 4);
    return value;
}
std::size_t offset(std::uint32_t address, std::uint32_t base, std::size_t size) {
    const auto relative = std::uint32_t(address - base);
    if (relative >= size)
        throw std::out_of_range("Lua registry pointer exceeds its native buffer");
    return relative;
}
std::string text(std::span<const std::byte> bytes, std::uint32_t address, std::uint32_t base) {
    const auto start = offset(address, base, bytes.size());
    auto end = start;
    while (end < bytes.size() && bytes[end] != std::byte{})
        ++end;
    if (end == bytes.size())
        throw std::out_of_range("Lua registry string is unterminated");
    return {reinterpret_cast<const char*>(bytes.data() + start), end - start};
}
}

std::span<const ModuleDefinition> builtin_registry() noexcept {
    return modules;
}

void decode_native_registry(std::span<const std::byte> encoded, std::span<std::byte> decoded,
                            std::uint32_t relocated_begin) {
    if (encoded.size() != native_registry_size || decoded.size() != native_registry_size)
        throw std::length_error("Lua registry must contain exactly 0xCFC bytes");
    std::uint32_t key = 0xd0e47c1d;
    for (std::size_t index = 4; index < native_registry_size; index += 4) {
        auto value = word(encoded, index) ^ key;
        if (value >= native_registry_begin && value <= native_registry_begin + native_registry_size)
            value += relocated_begin - native_registry_begin;
        std::memcpy(decoded.data() + index, &value, 4);
        if (key & 1)
            key ^= 0xc0000001;
        key >>= 1;
    }
}

std::vector<DecodedModule> inspect_native_registry(std::span<const std::byte> decoded,
                                                   std::uint32_t relocated_begin) {
    if (decoded.size() != native_registry_size)
        throw std::length_error("Lua registry must contain exactly 0xCFC bytes");
    std::vector<DecodedModule> result;
    for (std::size_t record = 0x8fc;; record += 12) {
        const auto name_address = word(decoded, record);
        if (!name_address)
            break;
        auto name = text(decoded, name_address, relocated_begin);
        const auto binder = word(decoded, record + 8);
        const ModuleDefinition* definition = nullptr;
        for (const auto& module : modules)
            if (name == module.name && binder == module.original_binder)
                definition = &module;
        if (!definition)
            throw std::invalid_argument("Unknown native Lua module constructor");
        DecodedModule module{definition->kind, std::move(name), binder, {}};
        auto entry = offset(word(decoded, record + 4), relocated_begin, decoded.size());
        for (;; entry += 8) {
            const auto export_name = word(decoded, entry);
            if (!export_name)
                break;
            module.exports.push_back(
                {text(decoded, export_name, relocated_begin), word(decoded, entry + 4)});
        }
        result.push_back(std::move(module));
    }
    return result;
}
} // namespace skeet::script

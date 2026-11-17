#include "skeet/script/manager.hpp"
#include <bit>

namespace skeet::script {
std::uint32_t script_name_hash(std::u16string_view name) noexcept {
    std::uint32_t hash = 0x811C9DC5;
    for (char16_t c : name) {
        if (!c)
            break;
        hash = (hash ^ c) * 0x1000193u;
    }
    return hash ^ 0x92CA5DF1;
}
std::uint32_t script_name_hash(std::string_view name) noexcept {
    std::uint32_t hash = 0x811C9DC5;
    for (unsigned char c : name) {
        if (!c)
            break;
        hash = (hash ^ c) * 0x1000193u;
    }
    return hash ^ 0x92CA5DF1;
}
std::uint64_t script_file_time(std::uint64_t windows_time) noexcept {
    const auto seconds = std::bit_cast<std::int64_t>(windows_time) / 10000000;
    return std::uint64_t(seconds) - 11644473600ull;
}
std::array<std::uint32_t, 128> startup_script_hashes(std::span<const ScriptRecord> records,
                                                     std::uint32_t excluded) noexcept {
    std::array<std::uint32_t, 128> result{};
    std::size_t index = 0;
    for (const auto& script : records) {
        const bool builtin = !script.name.empty() && script.name.front() == u'*';
        if (bool(script.startup) == builtin)
            continue;
        const auto hash = script_name_hash(script.name) ^ 0x92CA5DF1;
        if (hash == excluded)
            continue;
        result[index++] = hash;
        if (index == result.size())
            break;
    }
    return result;
}
} // namespace skeet::script

#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace skeet::script::ui_values_detail {
inline std::string utf8(std::u16string_view text) { // 43336ED2
    if (text.empty())
        return {};
    const auto length =
        WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(text.data()),
                            static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (!length)
        return {};
    std::string result(static_cast<std::size_t>(length), '\0');
    WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(text.data()),
                        static_cast<int>(text.size()), result.data(), length, nullptr, nullptr);
    return result;
}
inline std::u16string utf16(const char* text) { // 4332791E
    const auto length = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    if (!length)
        return {};
    std::u16string result(static_cast<std::size_t>(length), u'\0');
    MultiByteToWideChar(CP_UTF8, 0, text, -1, reinterpret_cast<wchar_t*>(result.data()), length);
    result.resize(static_cast<std::size_t>(length - 1));
    return result;
}
template <std::size_t Size> inline void single_line(std::array<wchar_t, Size>& text) { // 433CF798
    text.back() = L'\0';
    for (auto& unit : text) {
        if (!unit)
            break;
        if (unit == L'\n') {
            unit = L'\0';
            break;
        }
    }
}
inline std::uint32_t mode_hash(const char* text, std::size_t length) { // 433D6B7C
    std::uint32_t hash = 0x811c9dc5u;
    for (std::size_t index = 0; index < length; ++index) {
        auto byte = static_cast<unsigned char>(text[index]);
        if (byte >= 'A' && byte <= 'Z')
            byte += 'a' - 'A';
        hash = (hash ^ byte) * 0x01000193u;
    }
    return hash ^ 0x92ca5df1u;
}
} // namespace skeet::script::ui_values_detail

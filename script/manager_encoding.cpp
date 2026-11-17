#include "manager_detail.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <limits>
#include <stdexcept>
namespace skeet::script::manager_detail {
std::string utf8(std::u16string_view input) { //43336ED2, external WideCharToMultiByte
    if (input.empty())
        return {};
    if (input.size() > std::size_t(std::numeric_limits<int>::max()))
        throw std::length_error("Script name exceeds native conversion length");
    const auto* source = reinterpret_cast<const wchar_t*>(input.data());
    const auto count =
        WideCharToMultiByte(CP_UTF8, 0, source, int(input.size()), nullptr, 0, nullptr, nullptr);
    std::string result(std::size_t(count), 0);
    if (count)
        WideCharToMultiByte(CP_UTF8, 0, source, int(input.size()), result.data(), count, nullptr,
                            nullptr);
    return result;
}
std::u16string utf16(std::string_view input) { //4332791E, external MultiByteToWideChar
    if (input.empty())
        return {};
    if (input.size() > std::size_t(std::numeric_limits<int>::max()))
        throw std::length_error("Script name exceeds native conversion length");
    const auto count = MultiByteToWideChar(CP_UTF8, 0, input.data(), int(input.size()), nullptr, 0);
    std::u16string result(std::size_t(count), 0);
    if (count)
        MultiByteToWideChar(CP_UTF8, 0, input.data(), int(input.size()),
                            reinterpret_cast<wchar_t*>(result.data()), count);
    return result;
}
}

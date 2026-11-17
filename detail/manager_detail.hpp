#pragma once
#include <string>
#include <string_view>
namespace skeet::script::manager_detail {
std::string utf8(std::u16string_view);
std::u16string utf16(std::string_view);
}

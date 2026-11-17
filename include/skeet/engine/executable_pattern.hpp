#pragma once
#include <cstddef>
#include <cstdint>

namespace skeet::engine {
// Complete433C018C over loaded PE32 executable, nondiscardable sections.
// Matches within a section skip by pattern length; occurrence restarts for
// each section. Both native length subtractions are retained.
std::byte* executable_pattern(std::byte* module, const std::uint8_t* pattern, std::uint32_t length,
                              std::uint32_t occurrence = 0, std::uint8_t wildcard = 0xCC);
} //namespace skeet::engine

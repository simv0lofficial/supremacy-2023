#pragma once
#include <cstdint>

namespace skeet::rage {
struct ExperimentalSwitch {
    const std::uint8_t* value = nullptr;
    const std::uint8_t* master = nullptr;

    constexpr ExperimentalSwitch() = default;
    constexpr ExperimentalSwitch(const std::uint8_t* option, const std::uint8_t* parent = nullptr)
        : value(option), master(parent) {}

    explicit operator bool() const noexcept {
        return value && *value && (!master || *master);
    }
};
}

#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace skeet::engine {
struct OverlayBuildIdentity {
    std::uint32_t timestamp, checksum;
};
inline OverlayBuildIdentity require_overlay_build_identity(const std::byte* base) {
    const auto read = [&](std::size_t offset) {
        std::uint32_t value;
        std::memcpy(&value, base + offset, sizeof value);
        return value;
    };
    if (!base || static_cast<unsigned>(base[0]) != 0x4D || static_cast<unsigned>(base[1]) != 0x5A ||
        read(0x3C) != 0x130 || read(0x130) != 0x4550)
        throw std::runtime_error("Unsupported Steam overlay PE32 header layout");
    const auto timestamp = read(0x138);
    // Both installed builds retain the verified renderer producer, stream
    // callback patterns and CSharedMemStream RTTI. Each consumer still checks
    // its exact instruction/data relationships after this identity selection.
    switch (timestamp) {
    case 0x6AF24B4E:
        return {0x6AF24B4E, 0x16A236};
    case 0x6AF66EF3:
        return {0x6AF66EF3, 0x169F3A};
    default:
        char message[160];
        std::snprintf(message, sizeof message,
                      "Unsupported Steam overlay build: timestamp 0x%08X, checksum 0x%08X",
                      timestamp, read(0x188));
        throw std::runtime_error(message);
    }
}
}

#pragma once
#include "native_data_image.hpp"
#include "pe_exports.hpp"
#include "skeet/effects/glow.hpp"
#include "skeet/rage/spread.hpp"

namespace skeet::engine {
// Genuine five-stack-word engine entry. Its two output pointers are ordinary
// arguments; the function preserves the native RET14 callee cleanup.
using ShotgunPatternEntry = void(__stdcall*)(std::uint32_t, std::int32_t, std::int32_t, float*,
                                             float*);
struct BuildEspBindings {
    NativeDataImage& data;
    effects::GlowEngine glow_engine() const;
    // The adapter borrows the canonical43467DDC cell, not this result object.
    // Keeping the shared NativeDataImage alive preserves its actual context.
    rage::ShotgunPattern shotgun_pattern() const;
};
// Validate the supported loaded client image, unique complete entry anchors,
// helper calls, lookup-data operands, RandomFloat imports and return ABIs.
// Publishes4346832C/43467DDC; no game function or factory executes here.
BuildEspBindings initialize_build_esp_bindings(NativeDataImage&, NativeImageEnvironment);
BuildEspBindings initialize_build_esp_bindings(NativeDataImage&);
} // namespace skeet::engine

#pragma once
#include "native_data_image.hpp"

namespace skeet::engine {
struct BuildFinalStartupBindings {
    const std::byte* player_feedback_instruction;
    std::uint32_t player_feedback_offset;
    void** float_proxy_slot;
    const std::byte* overlay_return;
    void** overlay_stream_slot;
};
// Resolve the final ordinary client instruction, engine decoder table slot
// and overlay instruction operands from their supported loaded PE32 builds.
// Publishes only canonical data cells; no game method or target executes.
BuildFinalStartupBindings initialize_build_final_startup_bindings(NativeDataImage&);
} //namespace skeet::engine

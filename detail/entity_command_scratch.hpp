#pragma once
#include "skeet/client/entity_command_owner.hpp"
#include "skeet/engine/native_data_image.hpp"

namespace skeet::engine {
struct NativeOffsetGlobals;
}

namespace skeet::client::entity_command_detail {
// Complete primary slot4 43325439, including its two independent live gates.
void release_prediction_scratch(EntityCommandOwner* const& active, const engine::NativeDataImage&);
// Complete433E33D3. The actual lifecycle strings and live protected offsets
// supply the appended prediction fields; no datamap getter is executed.
void initialize_prediction_scratch(EntityCommandOwner* const& active,
                                   const engine::NativeDataImage&,
                                   const engine::NativeOffsetGlobals&, bool local_creation);
} // namespace skeet::client::entity_command_detail

#pragma once
#include "prediction_masks.hpp"
#include "skeet/engine/frame_update.hpp"

namespace skeet::client {
class NativeStartupSources;
}
namespace skeet::rage {
class PredictionMaskOwner;
struct PredictionMaskFactoryBindings {
    engine::OwnerRegistryBinding lifecycle;
    engine::OwnerRegistryBinding frames;
    PredictionMaskRuntimeBinding runtime;
    PredictionMaskOwner*& active; //43475A58
};

// Complete4347EA07 factory and433BFE13 destruction. Source-only context
// follows the original0x1C prefix; the masks keep their native+0C/+14 offsets.
PredictionMaskOwner* create_prediction_mask_owner(PredictionMaskFactoryBindings&);
void destroy_prediction_mask_owner(PredictionMaskOwner*, PredictionMaskOwner*&, engine::JobMemory);
PredictionMaskState& prediction_mask_state(PredictionMaskOwner&) noexcept;
engine::FrameUpdateEndpoint& prediction_mask_endpoint(PredictionMaskOwner&) noexcept;
void bind_prediction_mask_factory(client::NativeStartupSources&, PredictionMaskFactoryBindings&);
}

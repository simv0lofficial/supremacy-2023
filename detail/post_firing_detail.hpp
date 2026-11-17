#pragma once
#include "skeet/rage/post_firing.hpp"
namespace skeet::rage::post_firing_detail {
void predict_stop(PreambleState&, PreambleResult&, AcquisitionStorage&, void* local_player,
                  const PostFiringBindings&);
}

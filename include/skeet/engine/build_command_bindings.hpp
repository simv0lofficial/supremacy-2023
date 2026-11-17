#pragma once
#include "native_data_image.hpp"
#include "skeet/prediction/command.hpp"

namespace skeet::engine {
struct BuildCommandBindings {
    prediction::SeedGenerator seed_generator;
    void* move_helper;
    std::int32_t* random_seed;
    void** prediction_player;
    prediction::PredictionFlagNode** flag_head;
};
// Supported client65272025 only. Actual ordinary engine code independently
// supplies the command seed ABI and four static aliases; the working dump's
// globals match these operands. Validates PE/code/RET/RTTI provenance,
// then publishes434679AC/43468EC8/43468328/434693A4/43467C58/4346D660 together.
//4346D660 is the genuine two-word/RET8 predicted-frame restore operation.
// No engine function, constructor or captured client entry executes here.
BuildCommandBindings initialize_build_command_bindings(NativeDataImage&);
} //namespace skeet::engine

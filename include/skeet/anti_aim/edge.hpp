#pragma once
#include "skeet/movement/collision.hpp"

namespace skeet::anti_aim {
struct EdgeInput {
    std::uint8_t enabled; // secondary owner +0x60; full owner +0x68
    float frame_scalar_c0;
    float player_scalar; // decoded field 0x3EB71843
    std::uint8_t movement_type; // decoded field 0x7501CB91
};
struct EdgeSettings {
    float maximum_angle; // secondary +0x140, full owner +0x148
    float hull_padding; // secondary +0x144, full owner +0x14C
};
class EdgeEngine : public MovementCollisionEngine {
public:
    // These are the actual player/engine virtual boundaries, in their native
    // call order. Returned hull references may alias engine-owned storage.
    virtual const MovementVector& hull_minimum() = 0; // player +0x130
    virtual const MovementVector& hull_maximum() = 0; // player +0x134
    virtual const MovementVector& origin() = 0; // player +0x28
    virtual MovementAngles view_angles() = 0; // engine +0x48
    virtual std::uint32_t point_contents_world(const MovementVector&, std::uint32_t mask) = 0;
};

// Complete edge cut 0x4332D90A..0x4332E5BC, including its three gates and
// original world-trace wrapper. False continues at E5BC without changing yaw.
// True continues at E621: the caller also sets frame bit0x80000 and full-owner
// cache +0x1A8 to FLT_MAX, exactly as the native success tail does.
bool solve_edge_yaw(const EdgeInput&, const EdgeSettings&, EdgeEngine&,
                    std::uint8_t* engine_trace_flag, float& command_yaw);
} // namespace skeet::anti_aim

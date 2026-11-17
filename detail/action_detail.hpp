#pragma once
#include <cstring>
#include "skeet/rage/action_state.hpp"
#include "scanning_detail.hpp"
#include "util/address.h"

namespace skeet::rage::action_detail {
using ::skeet::read;
using ::skeet::write;
// 43332999..4333337E. The shared point-scale body is called at its native
// place, between minimum damage and the per-player cache/adjustment updates.
void prepare_settings(const client::CommandFrame&, void* target, std::uint32_t entity_index,
                      AcquisitionTargetSettings&, const ActionStateBindings&);
std::array<RageVector, 2> origins(const client::CommandFrame&, void* target,
                                  const ActionStateBindings&);
struct Point {
    RageVector position;
    std::uint32_t flags, highlight;
};
std::vector<Point> points(const client::CommandFrame&, const AcquisitionTargetSettings&,
                          const lagcomp::LagRecord&, const scan_detail::Model&,
                          const scan_detail::Geometry&, std::uint32_t hitbox, bool safe,
                          bool regular, const ActionStateBindings&);
} // namespace skeet::rage::action_detail

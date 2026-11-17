#pragma once
#include "skeet/rage/ballistics.hpp"

namespace skeet::anti_aim {
using ProximityEntity = std::uintptr_t;
struct ProximityList {
    const ProximityEntity* entities;
    std::int32_t count;
};
class ProximityEngine {
public:
    virtual ~ProximityEngine() = default;
    virtual const ProximityList* list() = 0; // global434688C0
    virtual bool active(ProximityEntity) = 0; // decoded entity field
    virtual BallisticVector origin(ProximityEntity) = 0; // virtual+0x28
};
// Complete native helpers. The first scans every list entry; the second checks
// only slot0. They retain candidate/local origin-call order and live list reload.
bool any_nearby_entity(ProximityEngine&, ProximityEntity local); //4334E49F
bool first_nearby_entity(ProximityEngine&, ProximityEntity local); //4340AEDA
} // namespace skeet::anti_aim

#pragma once
#include <cstddef>
#include <cstdint>

namespace skeet::client {
struct EntityPredictionEntities;
}
namespace skeet::rage {
struct EntityUpdateOffsets {
    std::ptrdiff_t movement_type; //7501CB91, byte
    std::ptrdiff_t owner_handle; //8D1BD8D0, int32
    std::ptrdiff_t model_index; //47877DEB, int16
    std::ptrdiff_t health; //9BB80163, int32
};
struct EntityUpdateBindings {
    void* const& engine_client; //4346A514
    void* const& entity_list; //4346A890
    void* const& model_info; //4346A614
    std::int32_t* const& maximum_clients; //43469300
    void* const& required_class; //43467478, actual ClientClass pointer
    EntityUpdateOffsets offsets;
};
// Complete43325DEC, entity-observer43472480 slot1. The incoming count is
// reread each iteration, and the two collideable getters remain distinct.
void update_rage_entities(const EntityUpdateBindings&, void* local_player,
                          const client::EntityPredictionEntities&);
}

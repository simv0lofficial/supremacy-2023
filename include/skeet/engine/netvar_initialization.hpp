#pragma once
#include "offset_storage.hpp"
#include <span>

namespace skeet::engine {
// Fixed-build data recovered from the supplied observed operand records.
// Names were matched independently against the working dump's engine trees;
// initial_xor_key is the pinned input image's static key, not a saved offset.
struct ReceiveOffsetRecipe {
    std::uint32_t operand;
    const char* client_class;
    const char* property;
    std::uint32_t xor_key_identity, pointer_identity, addend, initial_xor_key;
};
enum class DataMapRoot {
    player,
    weapon_base_gun
};
struct DataMapOffsetRecipe {
    std::uint32_t operand;
    DataMapRoot root;
    const char* table;
    const char* property;
    std::uint32_t xor_key_identity, pointer_identity, initial_xor_key;
};
std::span<const ReceiveOffsetRecipe, 268> build_receive_offset_recipes() noexcept;
std::span<const DataMapOffsetRecipe, 14> build_data_map_offset_recipes() noexcept;

// Source43407CDE schedule:231 receive rows,37 adjusted rows, two genuine
// datamap discoveries, then14 datamap rows. The caller supplies its seeded
// key-stream cursor in the same NativeOffsetStorage consumed by all clients.
// Missing classes/maps preserve outputs; missing properties retain native
// UINT32_MAX encoding. No decoded VM accessor or saved return is executed.
void initialize_build_netvars(void* base_client, std::byte* client_module, NativeOffsetStorage&);
} //namespace skeet::engine

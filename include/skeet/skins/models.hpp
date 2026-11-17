#pragma once
#include <cstdint>
#include <span>

namespace skeet::skins {
struct KnifeModel {
    std::uint16_t definition;
    std::uint32_t view_model_hash;
    std::uint32_t native_word_08, native_word_0c, native_word_10;
};
// 0x43467FD0, copied into the owner by 0x4348DFCB. Hashes identify actual
// view-model paths, e.g. models/weapons/v_knife_bayonet.mdl.
std::span<const KnifeModel, 19> knife_models() noexcept;
const KnifeModel* knife_model(std::int32_t selection) noexcept;
} // namespace skeet::skins

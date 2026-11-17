#pragma once
#include "skeet/skins/sequences.hpp"
#include "skeet/skins/sequence_receive.hpp"
#include "skeet/skins/refresh.hpp"
#include "skeet/engine/offset_storage.hpp"

namespace skeet::misc {
class ViewmodelLagOwner;
}
namespace skeet::skins {
struct NativeSequenceBindings {
    using RandomSeed = void(__cdecl*)(std::int32_t);
    using RandomInteger = std::int32_t(__cdecl*)(std::int32_t, std::int32_t);
    const engine::NativeOffsetStorage& offsets;
    void* const& engine_client; //4346A514
    void* const& entity_list; //4346A890
    void* const& model_info; //4346A614
    void* const& model_cache; //43469330
    void* const& animation; //4346A460, actual engine targets+5C/+64
    misc::ViewmodelLagOwner* const& viewmodel_owner; //live 43475860
    std::int32_t* const& random_seed; //43468328
    void** const& prediction_player; //434693A4
    float* const& interval; //4346A338
    const SequenceReceiveBindings& receive;
    const engine::RecvVarProxy& viewmodel_model; //nativeowner+64
    RandomSeed seed_random; //vstdlib!RandomSeed
    RandomInteger random_integer; //vstdlib!RandomInt
};

// Concrete4346DC68 slot1 producer. Queued proxy replay precedes the saved
// player's life/team reads and the incoming+44/+4044 inventory traversal.
// Entity fields and decoded offset words stay live across engine calls.
class NativeSequenceRuntime {
public:
    NativeSequenceRuntime(const Settings& settings, SequenceRuntime& state,
                          NativeSequenceBindings bindings, WeaponRefresh& refresh)
        : settings_(settings), state_(state), bindings_(bindings), refresh_(refresh) {}
    void process(void* player, std::uintptr_t incoming); //complete433E53A5
private:
    std::ptrdiff_t offset(std::uint32_t pointer, std::uint32_t key, std::uint32_t mask) const;
    std::int32_t team(const void*) const;
    bool enabled(const void*) const;
    std::int32_t selection(const void*) const;
    bool same_owner(const void*) const;
    std::uint32_t class_id(void*) const;
    void* resolve(std::uint32_t) const;
    std::int32_t sequence_count(void*) const;
    void* sequence_description(void*, std::int32_t) const;
    std::int32_t weighted_sequence(void*, std::uint32_t) const;
    std::int32_t inspection_sequence(void*) const;
    void remap(void*, std::int32_t& model, std::int32_t& sequence);
    const Settings& settings_;
    SequenceRuntime& state_;
    NativeSequenceBindings bindings_;
    WeaponRefresh& refresh_;
};
} //namespace skeet::skins

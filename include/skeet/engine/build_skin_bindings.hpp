#pragma once
#include "native_data_image.hpp"
#include "pe_exports.hpp"
#include "netvars.hpp"
#include <cstdint>

namespace skeet::engine {
// Genuine external engine boundaries and class objects used by the skin
// owners. All source views below share this canonical data image. Fixtures may
// supply authored engine boundaries; production uses the acquisition function.
struct BuildSkinBindings {
    using RandomSeed = void(__cdecl*)(std::int32_t);
    using RandomInteger = std::int32_t(__cdecl*)(std::int32_t, std::int32_t);
    NativeDataImage& data;
    RandomSeed seed_random;
    RandomInteger random_integer;
    ClientClass* weapon_world_model_class;
    ClientClass* knife_class;
};

// Supported client65272025. Locate unique code signatures and read their real
// static data operands before publication. The returned function addresses are
// in the actual loaded client/vstdlib images; no captured singleton is copied.
// Acquires CEconWearable, CBaseWeaponWorldModel and CKnife through VClient018's
// actual GetAllClasses boundary. ItemSystem43468B9C must already be produced
// by initialize_build_client_factories; it is not called here.
BuildSkinBindings initialize_build_skin_bindings(NativeDataImage&, NativeImageEnvironment);
BuildSkinBindings initialize_build_skin_bindings(NativeDataImage&);
} //namespace skeet::engine

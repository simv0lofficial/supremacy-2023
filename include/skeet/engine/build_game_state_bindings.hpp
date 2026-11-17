#pragma once
#include "native_data_image.hpp"

namespace skeet::engine {
// Source acquisition for client 65272025 and engine 64EE7E86. The canonical
// module and VClient018/VEngineClient014 interface cells must already be bound.
// Reads the actual loaded PE images and virtual-method instruction operands;
// calls no game function and installs no pointer from a captured process.
//
// Publishes gpGlobals 43467B04, CInput 4346A894, CClientState 43468E44 and the
// gpGlobals member aliases 43467EB8/+10, 434678F8/+4, 4346A338/+20. CClientState
// is the engine's current split-screen record plus eight, stored without XOR.
// All identities, signatures and pointer provenance are checked before the
// first publication. Unsupported builds or missing initialized objects fail.
void initialize_build_game_state_bindings(NativeDataImage&);
// The two genuine client factories consumed by4341F57F: GlowObjectManager
// (434678C4, consumed once) and ItemSystem (43468B9C, retained for skins/icons).
// Verify their actual singleton/constructor code and publish both entries;
// also binds43467C04 to the real lazy refresh-vector pointer slot after
// verifying its static reset, getter and indexed virtual0 notifier loop;
// this operation never calls a factory or copies a captured singleton object.
void initialize_build_client_factories(NativeDataImage&);
} // namespace skeet::engine

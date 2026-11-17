#pragma once
#include "native_data_image.hpp"
#include "pe_exports.hpp"

namespace skeet::engine {
// Source inputs consumed once by43485DC9. Recover the real engine CALL sites,
// their constructor/destructor pairs and exact protobuf class identities.
// This publishes4346D668/4346D66C without executing an engine instruction.
void initialize_build_shared_esp_bindings(NativeDataImage&, NativeImageEnvironment);
void initialize_build_shared_esp_bindings(NativeDataImage&);
} // namespace skeet::engine

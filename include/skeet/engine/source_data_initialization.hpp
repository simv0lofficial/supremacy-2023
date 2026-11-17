#pragma once
#include "native_data_image.hpp"

namespace skeet::engine {
// Reconstruct only the proven static defaults needed by these data consumers.
// Call once on newly-created source storage, before any producer or parser.
// This does not discover interfaces, initialize owners or complete startup.
void initialize_source_native_data(NativeDataImage&);
// Proven static metadata for the five typed configuration views borrowed by
// source runtime owners. Existing-data views never invoke this initializer.
void initialize_source_configuration_data(NativeDataImage&);
} //namespace skeet::engine

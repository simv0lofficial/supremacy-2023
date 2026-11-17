#pragma once
#include "verified_command.hpp"
#include "skeet/engine/native_data_image.hpp"

namespace skeet::client {
// Complete client307260 CUserCmd checksum. The14 field ranges are part of
// this source Command ABI; padding, vtable and trailing prediction state are
// deliberately outside the native checksum transaction.
std::uint32_t source_command_checksum(const Command&) noexcept;
CommandChecksum source_command_checksum_entry() noexcept;
// Publish the actual compiled source function, never a captured module address.
// A conflicting preexisting publication is rejected before any change.
void initialize_source_command_checksum(engine::NativeDataImage&);
} //namespace skeet::client

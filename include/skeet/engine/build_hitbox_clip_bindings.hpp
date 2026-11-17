#pragma once
#include "native_data_image.hpp"
#include "skeet/rage_hitchance.hpp"

namespace skeet::engine {
// Supported client65272025. Verifies the ordinary loaded client function and
// publishes43468C3C. No module load or engine function call is performed.
void* initialize_build_hitbox_clip_bindings(NativeDataImage&);
// Source packing for4339FA8E's genuine external boundary. The entry is read
// from the canonical cell on every call. Its x86 ABI uses ECX=Ray_t,
// EDX=mstudiobbox_t, stack(matrix,trace) and a plain RET with caller cleanup.
RageBoxClipper native_hitbox_clipper(NativeDataImage&);
} //namespace skeet::engine

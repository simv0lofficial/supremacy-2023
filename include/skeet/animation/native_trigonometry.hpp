#pragma once
#include "skeet/rage_hitboxes.hpp"

namespace skeet::animation {
// Actual loaded msvcrt.dll __libm_sse2_sinf/__libm_sse2_cosf imports.
// The adapters convert the source float(float) ABI to XMM0 input/result.
// All source consumers borrow the same immutable process instance. Missing
// exports or a non-x86 process fail; this never loads a replacement library.
const RageTrigonometry& native_sse_trigonometry();
//4335C2B0/4335C300 pass y inXMM0 and x inXMM1 to __libm_sse2_atan2.
double native_sse_atan2(double y, double x);
//433BE1A3 passes and receives the low float inXMM0 at __libm_sse2_expf.
float native_sse_exponential(float);
//4334331D uses these two genuine CRT exports with XMM0 input/result.
float native_sse_tangent(float);
float native_sse_arctangent(float);
} //namespace skeet::animation

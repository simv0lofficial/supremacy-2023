#pragma once
#include "native_data_image.hpp"
#include "skeet/legit/trigger_trace.hpp"

namespace skeet::engine {
//433E7C6B spills XMM0..5 in order, then passes an additional integer1.
// The genuine client function returns AL and leaves all seven words to caller.
using LineSmokeEntry = bool(__cdecl*)(float, float, float, float, float, float, std::int32_t);
struct BuildLegitBindings {
    NativeDataImage& data;
    legit::TriggerTraceBindings trigger_trace() const noexcept;
};
// Supported client65272025 only. Verifies the complete smoke entry's two RET
// tails, smoke-vector/handle-array operands and geometric helper call, then
// publishes canonical4346A82C. No engine/client function executes here.
BuildLegitBindings initialize_build_legit_bindings(NativeDataImage&);
// Complete ordinary433E7C6B. Its missing-entry false result is native behavior;
// a production source bootstrap still acquires the actual supported entry.
bool line_passes_through_smoke(const NativeDataImage&, const legit::Vector&, const legit::Vector&);
} //namespace skeet::engine

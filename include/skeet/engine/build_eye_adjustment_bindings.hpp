#pragma once
#include "native_data_image.hpp"

namespace skeet::engine {
// Locate the supported client65272025 Studio_SetPoseParameter implementation
// from its PE32 identity, unique instruction body and descriptor-helper edge.
// Publishes only canonical43467CF0; no game function is called here.
void initialize_build_eye_adjustment_bindings(NativeDataImage&);
// The adjacent genuine Studio_GetPoseParameter entry, called by433A0EC6.
// Its descriptor helper and both RET tails are verified independently before
// publishing43468CA4. The normalized input arrives inXMM2, result inXMM0.
void initialize_build_pose_range_bindings(NativeDataImage&);
class PoseRangeRuntime {
public:
    explicit PoseRangeRuntime(const NativeDataImage&);
    float convert(const void* studio_wrapper, std::uint32_t index, float normalized) const;

private:
    const std::uintptr_t& entry_;
};

class EyePoseParameterRuntime {
public:
    explicit EyePoseParameterRuntime(const NativeDataImage&);
    // Genuine engine ABI: ECX=studio wrapper, EDX=index, XMM2=angle,
    // caller-cleaned normalized-output pointer on the stack; result XMM0.
    // The source bridge converts only the float return to ordinary C++ ABI.
    float convert(const void* studio_wrapper, std::uint32_t index, float angle,
                  float& normalized) const;

private:
    const std::uintptr_t& entry_;
};
namespace detail {
// Explicit ABI bridge, exposed internally for authored/Unicorn contract tests.
float invoke_eye_pose_parameter(std::uintptr_t entry, const void* studio_wrapper,
                                std::uint32_t index, float angle, float& normalized);
float invoke_pose_range(std::uintptr_t, const void*, std::uint32_t, float);
}
} //namespace skeet::engine

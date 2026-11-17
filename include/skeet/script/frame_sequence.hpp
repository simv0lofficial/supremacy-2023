#pragma once
#include "bridge.hpp"

namespace skeet::script {

// The live engine word reached through native global434678F8. This binding
// deliberately requires that word by reference: no copied counter, default
// storage, last-outgoing-command substitution, or inferred GlobalVars alias.
class FrameSequenceBinding {
public:
    explicit FrameSequenceBinding(const std::uint32_t& native_sequence) noexcept
        : sequence_(native_sequence) {}
    FrameSequenceBinding(std::uint32_t&&) = delete;
    FrameSequenceBinding(const std::uint32_t&&) = delete;
    void bind(Bridge& bridge) const noexcept {
        bridge.frame_number = &sequence_;
    }
    const std::uint32_t& get() const noexcept {
        return sequence_;
    }

private:
    const std::uint32_t& sequence_;
};

} // namespace skeet::script

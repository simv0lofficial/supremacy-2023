#pragma once
#include "command_runtime.hpp"
#include "release_command.hpp"

namespace skeet {
// The native singleton publishes MovementCommandOwner, not a release adapter
// or a separately allocated feedback pointer. Keep the optional owner gate
// live while borrowing the real source adapter and its embedded feedback.
class MovementReleaseBinding {
public:
    MovementReleaseBinding(MovementCommandOwner* const& owner,
                           MovementReleaseRuntime& runtime) noexcept
        : owner_(&owner), runtime_(&runtime) {}
    // Explicit standalone boundary used by authored command fixtures.
    MovementReleaseBinding(MovementReleaseRuntime* const& runtime) noexcept
        : standalone_(&runtime) {}
    MovementReleaseRuntime* get() const noexcept {
        return owner_ ? (*owner_ ? runtime_ : nullptr) : *standalone_;
    }
    explicit operator bool() const noexcept {
        return get() != nullptr;
    }
    MovementReleaseRuntime& operator*() const noexcept {
        return *get();
    }

private:
    MovementCommandOwner* const* owner_ = nullptr;
    MovementReleaseRuntime* runtime_ = nullptr;
    MovementReleaseRuntime* const* standalone_ = nullptr;
};
class MovementFeedbackBinding {
public:
    explicit MovementFeedbackBinding(MovementCommandOwner* const& owner) noexcept
        : owner_(&owner) {}
    // Explicit standalone boundary used by authored command fixtures.
    MovementFeedbackBinding(MovementFeedback* const& feedback) noexcept : standalone_(&feedback) {}
    MovementFeedback* get() const noexcept {
        return owner_ ? (*owner_ ? &(*owner_)->feedback : nullptr) : *standalone_;
    }
    MovementFeedback& operator*() const noexcept {
        return *get();
    }

private:
    MovementCommandOwner* const* owner_ = nullptr;
    MovementFeedback* const* standalone_ = nullptr;
};
} //namespace skeet

#pragma once
#include <functional>
#include <variant>
#include <stdexcept>

namespace skeet::animation {
struct BoneRuntime;
// A production context names its actual raw bone runtime. Decoded fixture
// inputs exist only in the other arm and cannot be accidentally consumed by
// production. Copies preserve the selected source and never fabricate a model.
template <class T> class BoneSourceReference {
    std::variant<std::reference_wrapper<T>, std::reference_wrapper<const BoneRuntime>> source_;

public:
    BoneSourceReference(T& decoded) noexcept : source_(std::ref(decoded)) {}
    BoneSourceReference(const BoneRuntime& live) noexcept : source_(std::cref(live)) {}
    T& decoded() const {
        if (auto* value = std::get_if<std::reference_wrapper<T>>(&source_))
            return value->get();
        throw std::logic_error(
            "A live bone context cannot be consumed through its decoded fixture entry");
    }
    const BoneRuntime* live() const noexcept {
        if (auto* value = std::get_if<std::reference_wrapper<const BoneRuntime>>(&source_))
            return &value->get();
        return nullptr;
    }
};
} //namespace skeet::animation

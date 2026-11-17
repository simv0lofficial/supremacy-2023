#pragma once
#include <cstddef>
#include <span>

namespace skeet::script {
struct ShutdownState;
}
namespace skeet {
struct NativeConfigVector;

// A borrowed source, not a cached range. Copying the view preserves the owner
// identity; bytes() reads its currently published range on every invocation.
class ConfigStreamView {
public:
    static ConfigStreamView native_vector(const NativeConfigVector&) noexcept;
    static ConfigStreamView script_state(const script::ShutdownState&) noexcept;
    // Immutable standalone/captured inputs explicitly select snapshot semantics.
    static ConfigStreamView snapshot(std::span<const std::byte>) noexcept;
    // Two-phase owner construction only. Reading this route is an error.
    static ConfigStreamView unbound() noexcept;

    std::span<const std::byte> bytes() const;
    bool is_unbound() const noexcept;
    bool is_snapshot() const noexcept;
    bool is_script_state(const script::ShutdownState&) const noexcept;
    bool same_source(const ConfigStreamView&) const noexcept;

private:
    enum class Route {
        native,
        script,
        snapshot,
        unbound
    };
    ConfigStreamView(Route route, const void* owner,
                     std::span<const std::byte> captured = {}) noexcept
        : route_(route), owner_(owner), captured_(captured) {}
    Route route_;
    const void* owner_;
    std::span<const std::byte> captured_;
};
} //namespace skeet

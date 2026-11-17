#pragma once
#include "native_data_image.hpp"
#include <cstring>
#include <span>
#include <type_traits>
#include <variant>

namespace skeet::client {
struct StartupConVars;
}
namespace skeet::engine {
// A live view of one pointer cell. Production binds the canonical encoded
// word and its key. Standalone decoded cells are borrowed by address without
// aliasing their pointer type or retaining a snapshot of the pointed object.
class NativeConVarReference {
public:
    NativeConVarReference(const std::uint32_t& encoded, std::uint32_t key) noexcept
        : storage_(&encoded), key_(key), representation_(Representation::encoded_word) {}
    NativeConVarReference(std::uint32_t&&, std::uint32_t) = delete;
    NativeConVarReference(const std::uint32_t&&, std::uint32_t) = delete;
    template <class Pointer>
        requires(std::is_pointer_v<Pointer> && !std::is_volatile_v<Pointer> &&
                 (std::is_object_v<std::remove_pointer_t<Pointer>> ||
                  std::is_void_v<std::remove_pointer_t<Pointer>>))
    NativeConVarReference(Pointer& decoded) noexcept
        : storage_(&decoded), key_(0), representation_(Representation::decoded_pointer) {
        static_assert(sizeof(Pointer) == sizeof(void*));
    }
    void* get() const noexcept {
        if (representation_ == Representation::encoded_word) {
            std::uint32_t encoded;
            std::memcpy(&encoded, storage_, sizeof encoded);
            return reinterpret_cast<void*>(std::uintptr_t(encoded ^ key_));
        }
        void* decoded;
        std::memcpy(&decoded, storage_, sizeof decoded);
        return decoded;
    }
    operator void*() const noexcept {
        return get();
    }

private:
    enum class Representation {
        encoded_word,
        decoded_pointer
    };
    const void* storage_;
    std::uint32_t key_;
    Representation representation_;
};
// Explicit decoded semantic objects and canonical live pointer-cell views are
// different input profiles. Production passes NativeConVarReference directly.
class ConVarBinding {
public:
    ConVarBinding(const void* decoded) noexcept : value_(decoded) {}
    ConVarBinding(NativeConVarReference live) noexcept : value_(live) {}
    const void* get() const noexcept {
        if (const auto* live = std::get_if<NativeConVarReference>(&value_))
            return live->get();
        return std::get<const void*>(value_);
    }
    operator const void*() const noexcept {
        return get();
    }

private:
    std::variant<const void*, NativeConVarReference> value_;
};
struct StartupConVarSlot {
    std::uint32_t address, xor_key;
    const char* name;
};
std::span<const StartupConVarSlot, 9> startup_convar_slots() noexcept;

// Only the genuine ICvar lookup boundary is replaceable by an isolated test.
class StartupConVarProvider {
public:
    virtual ~StartupConVarProvider() = default;
    virtual void* find_variable(void* cvar, const char* exact_name) = 0;
};
class StartupConVarBindings {
public:
    NativeDataImage& data() const noexcept {
        return data_;
    }
    client::StartupConVars startup_variables() const;
    NativeConVarReference accuracy_variable() const;

private:
    explicit StartupConVarBindings(NativeDataImage& data) noexcept : data_(data) {}
    NativeDataImage& data_;
    friend class BuildStartupConVarBindings;
    friend StartupConVarBindings discover_startup_convars(NativeDataImage&, void*,
                                                          StartupConVarProvider&);
};

// Resolve all eight post-initialization variables and the menu accuracy
// variable before publishing any canonical word. The default calls ICvar15.
StartupConVarBindings discover_startup_convars(NativeDataImage&, void* cvar,
                                               StartupConVarProvider&);
StartupConVarBindings discover_startup_convars(NativeDataImage&, void* cvar);
} //namespace skeet::engine

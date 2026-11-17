#pragma once
#include "skeet/engine/owner_registry.hpp"
#include "skeet/engine/hook_descriptors.hpp"
#include "skeet/misc/config.hpp"
#include "skeet/misc/native_text.hpp"
#include <span>

namespace skeet::misc {
class AutoAcceptLifecycleOwner;

// Descriptor43475814 is the single backing for target+4,original+8,owner+C,
// replacement+10. The recipe, owner, handlers and checkbox share this object.
struct AutoAcceptPatchState {
    engine::VtableHookMetadata& metadata;
};
struct AutoAcceptOwnerBindings {
    AutoAcceptPatchState& patch;
    AutoAcceptLifecycleOwner*& active_owner; //43475970
    void* const& panorama; //43468E98, actual service slot11
    std::byte* const& source_module; //live43469234, also recipe23 source
    void* const& constructor_callback; //4346D554, snapshot at construction
};

struct AutoAcceptHandlerData {
    const AutoAcceptConfig& configuration;
    const NativeEncryptedText& name;
    const std::uint32_t& name_hash;
    void* const& callback;
};

std::size_t auto_accept_owner_size() noexcept;
// Complete ordinary constructor4348E0E8. Uses fresh, native aligned storage.
// Its embedded descriptor is the ONE canonical141F80B7 value; do not initialize
// another AutoAcceptConfig. The x86 prefix retains every native field offset
// consumed by the concrete handler43320430. Recipe23's actual descriptor
// producer owns the same metadata.original_table/replacement_table cells.
AutoAcceptLifecycleOwner& construct_auto_accept(void* storage,
                                                engine::OwnerRegistryBinding lifecycle,
                                                ConfigRegistries&, AutoAcceptOwnerBindings);
AutoAcceptConfig& auto_accept_configuration(AutoAcceptLifecycleOwner&) noexcept;
AutoAcceptHandlerData auto_accept_handler_data(AutoAcceptLifecycleOwner&) noexcept;
void install_auto_accept(AutoAcceptLifecycleOwner&); //complete4347DB61
void restore_auto_accept(AutoAcceptLifecycleOwner&); //complete434029F3

// Complete4338A9E9. Captures original virtual0 BEFORE exchanging the receiver's
// first dword, clears the shared target, then forwards the real engine argument.
std::uint32_t restore_auto_accept_receiver(AutoAcceptPatchState&, void* receiver,
                                           std::uintptr_t argument);
void bind_auto_accept_restore_hook(AutoAcceptPatchState&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
std::uint32_t __fastcall auto_accept_restore_hook(void* receiver, void*, std::uintptr_t argument);
#else
std::uint32_t auto_accept_restore_hook(void* receiver, void*, std::uintptr_t argument);
#endif

inline constexpr std::uint32_t auto_accept_hook_descriptor = 0x43475814u;
inline constexpr std::uint32_t auto_accept_hook_source_global = 0x43469234u;
inline constexpr std::uint32_t auto_accept_hook_class_hash = 0xF8B3A903u;
inline constexpr std::uint32_t auto_accept_hook_base_hash = 0;
inline constexpr std::uint32_t auto_accept_hook_replacements_operand = 0x8C8u;
struct AutoAcceptHookReplacement {
    std::uint32_t slot;
    std::uint32_t native_entry;
    void* source_entry;
};
std::span<const AutoAcceptHookReplacement, 2> auto_accept_hook_replacements();

// Whole captured recipe23, contexts2110..2119. The shared descriptor producer
// resolves the live module/type/base and clones its genuine executable prefix;
// these two concrete source replacements are bound AFTER that producer.
class AutoAcceptHookFamily {
public:
    AutoAcceptHookFamily(AutoAcceptPatchState&, engine::VtableHookRegistry&,
                         std::byte* const& source_module, engine::NativeImageEnvironment,
                         engine::HookDescriptorSystem) noexcept;
    AutoAcceptHookFamily(const AutoAcceptHookFamily&) = delete;
    AutoAcceptHookFamily& operator=(const AutoAcceptHookFamily&) = delete;
    // The actual constructor scheduler decides the timing. Do not follow this
    // whole source operation with a raw operand loop overwriting its entries.
    void prepare();
    void bind_replacements() noexcept;
    // Installs the prepared table exactly as it currently stands. Capturing
    // target/owner remains lifecycle4347DB61; slot writes belong to prepare.
    engine::VtableHookNode* install(void* actual_receiver);
    engine::VtableHookMetadata& metadata() const noexcept {
        return patch_.metadata;
    }

private:
    AutoAcceptPatchState& patch_;
    engine::VtableHookRegistry& hooks_;
    std::byte* const& source_module_;
    engine::NativeImageEnvironment images_;
    engine::HookDescriptorSystem system_;
};
} //namespace skeet::misc

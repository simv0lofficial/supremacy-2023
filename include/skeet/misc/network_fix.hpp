#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace skeet::client {
struct PredictionHistoryOwner;
}
namespace skeet::misc {
#if defined(_M_IX86) || defined(__i386__)
using NetworkFixEngineEntry = void(__fastcall*)(std::uint8_t);
#else
using NetworkFixEngineEntry = void (*)(std::uint8_t);
#endif
struct NetworkFixProxyBindings {
    NetworkFixEngineEntry const& process; //live4346A454, called with CL0
    const std::uint32_t& encoded_singlestep; //4346A648 xor66E946DB
    const std::uintptr_t& guarded_return_site; //4346A64C
};
struct NetworkFixProxyTransfer {
    std::uint32_t continuation, eax, ecx;
};

// Source counterpart of4332CC3D/4332CC45 and433E8B4D/433D6D2C. The native
// far-transfer pair only writes/reads XMM14; thread-local source storage keeps
// the same original continuation without executing retained instructions.
// The two source x86 veneers preserve the native return-stack transfers.
// Keep this runtime alive while a patched caller frame or singlestep.parent
// can reference it. Its clone owns one0x68-byte object and22 pointer words.
class NetworkFixProxyRuntime {
public:
    explicit NetworkFixProxyRuntime(NetworkFixProxyBindings bindings) noexcept
        : bindings_(bindings) {}
    NetworkFixProxyRuntime(const NetworkFixProxyRuntime&) = delete;
    NetworkFixProxyRuntime& operator=(const NetworkFixProxyRuntime&) = delete;
    NetworkFixProxyRuntime(NetworkFixProxyRuntime&&) = delete;
    NetworkFixProxyRuntime& operator=(NetworkFixProxyRuntime&&) = delete;
    void record_original_proxy(std::uint32_t raw_entry) const;
    void* proxy_entry() const;
    const NetworkFixProxyTransfer* prepare_proxy_transfer() const; //433D6D2C
    bool finish_singlestep(std::uintptr_t outer_return_site) const; //43336B8A
private:
    NetworkFixProxyBindings bindings_;
    mutable std::array<std::byte, 0x68> clone_;
    mutable std::array<std::uintptr_t, 22> table_;
    mutable void* active_clone_ = nullptr;
};
struct NetworkFixOffsets {
    std::ptrdiff_t byte_9d920e35; //43468E78 ^434684E4 ^9D920E35
    std::ptrdiff_t scalar_7e9ba7bc; //4346A464 ^43468584 ^7E9BA7BC
    std::ptrdiff_t byte_ac7f2913; //434678A4 ^4346905C ^AC7F2913
    std::ptrdiff_t pointer_f58e6118; //43468FD0 ^43467C70 ^F58E6118
};
struct NetworkFixBindings {
    const std::uint8_t& enabled; //SettingsMenuValues.fix_networking,43467CC0
    const std::uintptr_t& proxy_call_site; //434689A0
    void* const& proxy_context; //4346A64C
    const std::uintptr_t& sequence_call_site; //434692F4
    void* const& engine; //4346A514
    client::PredictionHistoryOwner* const& sequence_state; //live434759B4,canonical landing tags
    NetworkFixOffsets offsets;
    const NetworkFixProxyRuntime& proxy_runtime;
};
// Full ordinary native433463A1..43346525. Valid engine receivers are required;
// native null-receiver paths deliberately execute invalid instructions. The
// caller-frame saved return is patched only after its original continuation
// is recorded in the source proxy runtime.
std::uintptr_t process_network_fix(void* receiver, void* supplied_object,
                                   std::uintptr_t return_address, const NetworkFixBindings&);
void bind_network_fix(const NetworkFixBindings&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
std::uintptr_t __fastcall network_fix_hook(void* receiver, void* supplied_object);
#else
std::uintptr_t network_fix_hook(void* receiver, void* supplied_object);
#endif
} //namespace skeet::misc

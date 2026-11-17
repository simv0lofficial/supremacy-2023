#pragma once
#include <cstdint>

namespace skeet::network {
struct ClientStateHooksBindings;
// Six remaining methods of actual descriptor43475A3C. The one-word methods
// preserve the original EAX result; slots2/47 preserve its returned AL byte.
class ClientStateHooksRuntime {
public:
    explicit ClientStateHooksRuntime(ClientStateHooksBindings& bindings) noexcept
        : bindings(bindings) {}
    ClientStateHooksBindings& bindings;
    bool slot2(void* object, std::uintptr_t incoming); //complete433C04CC
    std::uintptr_t slot15(void* object, std::uintptr_t incoming); //complete433FBA1C
    std::uintptr_t slot32(void* object, std::uintptr_t incoming); //complete433F2918
    bool slot47(void* object, std::int32_t state, std::uintptr_t second,
                std::uintptr_t third); //43320766
};
void bind_client_state_hooks_runtime(ClientStateHooksRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_CLIENT_STATE_HOOK __fastcall
#else
#define SKEET_CLIENT_STATE_HOOK
#endif
bool SKEET_CLIENT_STATE_HOOK client_state_slot2_hook(void*, void*, std::uintptr_t);
std::uintptr_t SKEET_CLIENT_STATE_HOOK client_state_slot12_hook(void*, void*, std::uintptr_t);
std::uintptr_t SKEET_CLIENT_STATE_HOOK client_state_slot15_hook(void*, void*, std::uintptr_t);
std::uintptr_t SKEET_CLIENT_STATE_HOOK client_state_slot25_hook(void*, void*, std::uintptr_t);
std::uintptr_t SKEET_CLIENT_STATE_HOOK client_state_slot32_hook(void*, void*, std::uintptr_t);
bool SKEET_CLIENT_STATE_HOOK client_state_slot47_hook(void*, void*, std::int32_t, std::uintptr_t,
                                                      std::uintptr_t);
#undef SKEET_CLIENT_STATE_HOOK
} //namespace skeet::network

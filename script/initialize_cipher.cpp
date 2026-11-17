#include "skeet/script/initialize.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winternl.h>
#include <intrin.h>
#include <stdexcept>

namespace skeet::script {
BuiltinCipher BuiltinCipher::system() {
    return {GetCurrentProcessId(),
            static_cast<std::uint32_t>(
                reinterpret_cast<std::uintptr_t>(NtCurrentTeb()->ProcessEnvironmentBlock)),
            reinterpret_cast<Query>(
                GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationProcess"))};
}
std::array<std::uint8_t, 16> BuiltinCipher::key() const { // 434198F1
    if (!query)
        throw std::logic_error("Native process-times query is not bound");
    ProcessTimes times{};
    query(reinterpret_cast<void*>(~std::uintptr_t(0)), 4, &times, sizeof(times), nullptr);
    auto seed = static_cast<std::uint32_t>(times.created) ^
                static_cast<std::uint32_t>(times.created >> 32) ^ peb ^ (process_id << 12);
    if (!seed)
        seed = static_cast<std::uint32_t>(__rdtsc());
    std::array<std::uint8_t, 16> result{};
    for (auto& value : result) {
        seed = seed * 0x343fd + 0x269ec3;
        value = static_cast<std::uint8_t>(seed >> 16);
    }
    return result;
}
void BuiltinCipher::transform(std::span<std::byte> bytes) const { // 4331E6F0
    const auto secret = key();
    std::array<std::uint8_t, 256> state{};
    for (std::size_t i = 0; i < state.size(); ++i)
        state[i] = static_cast<std::uint8_t>(i);
    std::uint8_t j = 0;
    for (std::size_t i = 0; i < state.size(); ++i) {
        j = static_cast<std::uint8_t>(j + state[i] + secret[i % 16]);
        std::swap(state[i], state[j]);
    }
    std::uint8_t i = 0;
    j = 0;
    for (auto& byte : bytes) {
        ++i;
        j = static_cast<std::uint8_t>(j + state[i]);
        std::swap(state[i], state[j]);
        byte ^= static_cast<std::byte>(state[static_cast<std::uint8_t>(state[i] + state[j])]);
    }
}
void ScriptRuntime::encrypt_builtin(BuiltinModule& module) {
    if (module.encrypted)
        throw std::logic_error("Builtin source is already encrypted");
    cipher.transform(module.source);
    module.encrypted = true;
}
void ScriptRuntime::encrypt_required(RequiredModule& module) {
    if (module.encrypted)
        throw std::logic_error("Required module source is already encrypted");
    cipher.transform(module.source);
    module.encrypted = true;
}
} // namespace skeet::script

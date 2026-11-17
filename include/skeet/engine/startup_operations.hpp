#pragma once
#include <cstddef>
#include <cstdint>

namespace skeet::engine {
struct VtableHookNode;
struct LoadedModuleEnvironment;

// Recovered portions of VM entry 0x4354DB7E. The enclosing callback has further
// operations; these helpers do not replace its complete initialization order.
void clear_startup_process_flag(volatile std::uint32_t& process_flags) noexcept;
std::byte* find_startup_jump_target(std::byte* client_module, std::uint32_t timestamp_low);
void initialize_startup_hook_list(VtableHookNode& sentinel) noexcept;
std::int64_t startup_unix_seconds(std::uint64_t file_time) noexcept;
std::byte* take_startup_common_controls(volatile std::uint32_t& module_hash,
                                        const LoadedModuleEnvironment& modules);
}

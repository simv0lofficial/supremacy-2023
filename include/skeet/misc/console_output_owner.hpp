#pragma once
#include "network_console_handlers.hpp"

namespace skeet::misc {
class ConsoleOutputOwner;
inline constexpr std::uint32_t native_console_output_owner_size = 0xECu;
std::size_t console_output_owner_size() noexcept;
// Whole43481F99. settings must be the unregistered canonical52EADF91
// descriptor. storage must carry the native43387448 allocation header;
// lifecycle slot0 performs4332503F/43369B05 and frees that allocation.
ConsoleOutputOwner& construct_console_output_owner(void* storage,
                                                   engine::OwnerRegistryBinding lifecycle,
                                                   engine::OwnerRegistryBinding hud,
                                                   ConfigRegistries&, ConsoleOutputConfig& settings,
                                                   ColoredLogRing*& active,
                                                   const float& frame_time);
ColoredLogRing& console_output_ring(ConsoleOutputOwner&) noexcept;
// Restored-source wrapper43484E4E..43484E6C. It allocates0xEC bytes; the semantic owner uses
// its host layout's size. Allocation failure explicitly publishes null at
//43484E65/E67. ErrorLog/NormalLog::hud_slot must refer to this exact slot.
ColoredLogRing* initialize_console_output_owner(engine::OwnerRegistryBinding lifecycle,
                                                engine::OwnerRegistryBinding hud, ConfigRegistries&,
                                                ConsoleOutputConfig& settings,
                                                ColoredLogRing*& active, const float& frame_time);
} //namespace skeet::misc

#pragma once
#include "auto_accept_owner.hpp"

namespace skeet::misc {
// Whole43320430. The installed table's metadata supplies its actual owner
// (+C) and original table (+8). event_name is the first of seven native stack
// words; the remaining six words are forwarded unchanged to original slot1.
std::uint32_t process_auto_accept_event(void* receiver, const char* event_name,
                                        std::uintptr_t second, std::uintptr_t third,
                                        std::uintptr_t fourth, std::uintptr_t fifth,
                                        std::uintptr_t sixth, std::uintptr_t seventh);

#if defined(_M_IX86) || defined(__i386__)
// ECX receiver, ignored EDX, seven stack words, RET1C; EAX is original result.
std::uint32_t __fastcall auto_accept_event_hook(void* receiver, void*, const char* event_name,
                                                std::uintptr_t second, std::uintptr_t third,
                                                std::uintptr_t fourth, std::uintptr_t fifth,
                                                std::uintptr_t sixth, std::uintptr_t seventh);
#else
std::uint32_t auto_accept_event_hook(void* receiver, void*, const char* event_name,
                                     std::uintptr_t second, std::uintptr_t third,
                                     std::uintptr_t fourth, std::uintptr_t fifth,
                                     std::uintptr_t sixth, std::uintptr_t seventh);
#endif
// Paired slot0 is auto_accept_restore_hook, declared in auto_accept_owner.hpp.
} //namespace skeet::misc

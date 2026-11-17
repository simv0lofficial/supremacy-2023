#pragma once
#include <cstddef>
#include <stdexcept>

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
template <class Result, class... Arguments>
using MemberFunction = Result(__thiscall*)(void*, Arguments...);
#else
template <class Result, class... Arguments> using MemberFunction = Result (*)(void*, Arguments...);
#endif
// Source engine interfaces use the Microsoft x86 member-function ABI. The
// explicit receiver also permits host-side verification with interface spies.
template <class Result, class... Arguments>
Result invoke(void* object, std::size_t slot, Arguments... arguments) {
    if (!object)
        throw std::logic_error("Required engine interface is not bound");
    const auto table = *static_cast<void***>(object);
    return reinterpret_cast<MemberFunction<Result, Arguments...>>(table[slot])(object,
                                                                               arguments...);
}
}

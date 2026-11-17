#pragma once
#include "panorama_api.hpp"
#include <type_traits>
#include <utility>

namespace skeet::script::v8_abi {
// V8 was built with MSVC. Local, MaybeLocal and Maybe<bool> use a hidden
// result pointer on x86, even though MinGW returns our small POD views in EAX.
// Keep the native result slot explicit without changing by-value arguments.
template <auto Member, class Signature> struct StaticResult;
template <auto Member, class Result, class... Arguments>
struct StaticResult<Member, Result (*)(Arguments...)> {
    using Native = Result* (*)(Result*, Arguments...);
    inline static Native target = nullptr;
    static Result invoke(Arguments... arguments) {
        Result result{};
        target(&result, arguments...);
        return result;
    }
};

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_V8_RESULT_METHOD __thiscall
#else
#define SKEET_V8_RESULT_METHOD
#endif
template <auto Member, class Signature> struct MethodResult;
template <auto Member, class Result, class Receiver, class... Arguments>
struct MethodResult<Member, Result(SKEET_V8_RESULT_METHOD*)(Receiver, Arguments...)> {
    using Native = Result*(SKEET_V8_RESULT_METHOD*)(Receiver, Result*, Arguments...);
    inline static Native target = nullptr;
    static Result SKEET_V8_RESULT_METHOD invoke(Receiver receiver, Arguments... arguments) {
        Result result{};
        target(receiver, &result, arguments...);
        return result;
    }
};
#undef SKEET_V8_RESULT_METHOD

template <auto Member> auto bind_static(void* address) {
    using Signature = std::remove_reference_t<decltype(std::declval<PanoramaV8Api>().*Member)>;
    using Boundary = StaticResult<Member, Signature>;
    Boundary::target = reinterpret_cast<typename Boundary::Native>(address);
    return &Boundary::invoke;
}
template <auto Member> auto bind_method(void* address) {
    using Signature = std::remove_reference_t<decltype(std::declval<PanoramaV8Api>().*Member)>;
    using Boundary = MethodResult<Member, Signature>;
    Boundary::target = reinterpret_cast<typename Boundary::Native>(address);
    return &Boundary::invoke;
}
} //namespace skeet::script::v8_abi

#pragma once
#ifdef small
#undef small
#endif
#include <cstdint>
#include <cstring>
#include <bit>
#include <type_traits>

namespace skeet {

inline float real(std::uint32_t value) noexcept {
    return std::bit_cast<float>(value);
}

inline std::uint32_t bits(float value) noexcept {
    return std::bit_cast<std::uint32_t>(value);
}

inline std::int32_t integer(std::uint32_t value) noexcept {
    return std::bit_cast<std::int32_t>(value);
}

template <class T>
inline T read(const void* object, std::ptrdiff_t offset = 0) noexcept {
    std::remove_const_t<T> value{};
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof(value));
    return value;
}

template <class T>
inline void write(void* object, std::ptrdiff_t offset, const T& value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}

template <class T>
inline void store(void* object, std::ptrdiff_t offset, const T& value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}

template <class T>
inline T field(const void* object, std::ptrdiff_t offset) noexcept {
    std::remove_const_t<T> value{};
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof(value));
    return value;
}

template <class T>
inline void field(void* object, std::ptrdiff_t offset, const T& value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}

inline std::int32_t ticks(float time, float interval) noexcept {
    return static_cast<std::int32_t>(0.5f + time / interval);
}

template <class T, class U>
    requires (std::is_arithmetic_v<T> && std::is_arithmetic_v<U>)
inline auto add(T a, U b) noexcept {
    return a + b;
}

template <class T, class U>
    requires (std::is_arithmetic_v<T> && std::is_arithmetic_v<U>)
inline auto subtract(T a, U b) noexcept {
    return a - b;
}

template <class T, class U>
    requires (std::is_arithmetic_v<T> && std::is_arithmetic_v<U>)
inline auto multiply(T a, U b) noexcept {
    return a * b;
}

template <class T, class U>
    requires (std::is_arithmetic_v<T> && std::is_arithmetic_v<U>)
inline auto divide(T a, U b) noexcept {
    return a / b;
}

inline std::int32_t truncate(float value) noexcept {
    return static_cast<std::int32_t>(value);
}

namespace rage {
    using skeet::real;
    using skeet::bits;
    using skeet::integer;
    using skeet::read;
    using skeet::write;
    using skeet::store;
    using skeet::field;
    using skeet::ticks;
    using skeet::add;
    using skeet::subtract;
    using skeet::multiply;
    using skeet::divide;
    using skeet::truncate;
}

namespace client {
    using skeet::real;
    using skeet::bits;
    using skeet::integer;
    using skeet::read;
    using skeet::write;
    using skeet::field;
    using skeet::ticks;
    using skeet::add;
    using skeet::subtract;
    using skeet::multiply;
    using skeet::divide;
    using skeet::truncate;
}

namespace legit {
    using skeet::real;
    using skeet::bits;
    using skeet::integer;
    using skeet::read;
    using skeet::write;
    using skeet::field;
    using skeet::ticks;
    using skeet::add;
    using skeet::subtract;
    using skeet::multiply;
    using skeet::divide;
    using skeet::truncate;
}

namespace detail {
    using skeet::real;
    using skeet::bits;
    using skeet::integer;
    using skeet::read;
    using skeet::write;
    using skeet::field;
    using skeet::ticks;
    using skeet::add;
    using skeet::subtract;
    using skeet::multiply;
    using skeet::divide;
    using skeet::truncate;
}

namespace lagcomp {
    using skeet::real;
    using skeet::bits;
    using skeet::integer;
    using skeet::read;
    using skeet::write;
    using skeet::field;
    using skeet::ticks;
    using skeet::add;
    using skeet::subtract;
    using skeet::multiply;
    using skeet::divide;
    using skeet::truncate;
}

namespace shots {
    using skeet::real;
    using skeet::bits;
    using skeet::integer;
    using skeet::read;
    using skeet::write;
    using skeet::field;
    using skeet::ticks;
    using skeet::add;
    using skeet::subtract;
    using skeet::multiply;
    using skeet::divide;
    using skeet::truncate;
}

namespace visuals {
    using skeet::real;
    using skeet::bits;
    using skeet::integer;
    using skeet::read;
    using skeet::write;
    using skeet::field;
    using skeet::ticks;
    using skeet::add;
    using skeet::subtract;
    using skeet::multiply;
    using skeet::divide;
    using skeet::truncate;
}

namespace aimbot {
    using skeet::real;
    using skeet::bits;
    using skeet::integer;
    using skeet::read;
    using skeet::write;
    using skeet::field;
    using skeet::ticks;
    using skeet::add;
    using skeet::subtract;
    using skeet::multiply;
    using skeet::divide;
    using skeet::truncate;
}

namespace engine {
    using skeet::real;
    using skeet::bits;
    using skeet::integer;
    using skeet::read;
    using skeet::write;
    using skeet::store;
    using skeet::field;
    using skeet::ticks;
    using skeet::add;
    using skeet::subtract;
    using skeet::multiply;
    using skeet::divide;
    using skeet::truncate;
}

namespace effects {
    using skeet::real;
    using skeet::bits;
    using skeet::integer;
    using skeet::read;
    using skeet::write;
    using skeet::field;
    using skeet::ticks;
    using skeet::add;
    using skeet::subtract;
    using skeet::multiply;
    using skeet::divide;
    using skeet::truncate;
}

} // namespace skeet

#pragma once
#include <algorithm>
#include <bit>
#include <cstdint>
#include <span>
#include <xmmintrin.h>

namespace skeet::engine {
namespace hash_growth_detail {
inline float unsigned_float(std::uint32_t value) noexcept {
    double converted = double(std::bit_cast<std::int32_t>(value));
    if (value & 0x80000000u)
        converted += 4294967296.0;
    return float(converted);
}
// Low result word of433EC12E's float-to-signed64 truncation. The native hash
// policies supply finite positive values; preserve the low word past2^32.
inline std::uint32_t integer_low_word(float value) noexcept {
    const auto bits = std::bit_cast<std::uint32_t>(value);
    const auto exponent = std::int32_t((bits >> 23) & 0xFFu) - 127;
    if (exponent < 0)
        return 0;
    const auto fraction = bits & 0x7FFFFFu;
    const bool negative = (bits >> 31) != 0;
    if (exponent > 63 || (!negative && exponent == 63) || (negative && exponent == 63 && fraction))
        return 0;
    std::uint64_t magnitude = 0x800000u | fraction;
    if (exponent >= 23)
        magnitude <<= unsigned(exponent - 23);
    else
        magnitude >>= unsigned(23 - exponent);
    return std::uint32_t(negative ? std::uint64_t(0) - magnitude : magnitude);
}
}
struct NativeHashGrowthDecision {
    bool rehash;
    std::uint32_t bucket_count;
};

// Shared433B20BA policy body. maximum_load/multiplier/next_resize are the
// actual policy words; ceiling is the recorded external CRT ceil import.
inline NativeHashGrowthDecision
plan_native_hash_growth(std::uint32_t buckets, std::uint32_t size, const float& maximum_load,
                        const float& multiplier, std::uint32_t& next_resize,
                        std::span<const std::uint32_t> bucket_primes, double (*ceiling)(double)) {
    using namespace hash_growth_detail;
    const auto wanted = size + 1u;
    if (wanted <= next_resize)
        return {false, 0};
    const auto current = unsigned_float(buckets == 1 ? 0u : buckets);
    const auto minimum = unsigned_float(wanted) / maximum_load;
    if (minimum > current) {
        const auto grown = current * multiplier;
        const auto maximum = _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(grown), _mm_set_ss(minimum)));
        const auto requested = integer_low_word(maximum);
        const auto count = *std::lower_bound(bucket_primes.begin(), bucket_primes.end(), requested);
        const float threshold = unsigned_float(count) * maximum_load;
        next_resize = integer_low_word(float(ceiling(double(threshold))));
        return {true, count};
    }
    const float threshold = current * maximum_load;
    next_resize = integer_low_word(float(ceiling(double(threshold))));
    return {false, 0};
}
} // namespace skeet::engine

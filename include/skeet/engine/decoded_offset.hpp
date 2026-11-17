#pragma once
#include "offset_storage.hpp"
#include <bit>

namespace skeet::engine {
// A live operand from the protected offset globals. Copying this object keeps
// its binding; only conversion reads the current pointer, pointee and key.
class DecodedOffset {
public:
    DecodedOffset(const NativeOffsetGlobals& globals, std::uint32_t pointer, std::uint32_t key,
                  std::uint32_t salt, std::uint32_t displacement)
        : globals_(&globals), pointer_(pointer), key_(key), salt_(salt),
          displacement_(displacement) {}
    operator std::ptrdiff_t() const {
        const auto value = static_cast<std::uint32_t>(
            (*globals_->pointers.at(pointer_) ^ globals_->keys.at(key_) ^ salt_) + displacement_);
        return static_cast<std::ptrdiff_t>(std::bit_cast<std::int32_t>(value));
    }

private:
    const NativeOffsetGlobals* globals_;
    std::uint32_t pointer_, key_, salt_, displacement_;
};
} //namespace skeet::engine

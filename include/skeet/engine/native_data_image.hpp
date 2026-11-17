#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace skeet::engine {
// Stable storage for the recovered x86 data layouts. Native addresses identify
// byte offsets; source-owned storage need not occupy the original address.
// This owner supplies no instructions, operand decoder or exception runtime.
class NativeDataImage {
public:
    static constexpr std::uintptr_t preferred_address = 0x43310000;
    static constexpr std::size_t image_size = 0x2FC000;
    static std::shared_ptr<NativeDataImage> create_source();
    virtual ~NativeDataImage() = default;
    NativeDataImage(const NativeDataImage&) = delete;
    NativeDataImage& operator=(const NativeDataImage&) = delete;
    NativeDataImage(NativeDataImage&&) = delete;
    NativeDataImage& operator=(NativeDataImage&&) = delete;

    std::byte* data() const noexcept {
        return mapping_.get();
    }
    std::size_t size() const noexcept {
        return mapped_size_;
    }
    template <class T> T& native_cell(std::uint32_t address) const {
        static_assert(std::is_trivially_copyable_v<T>);
        if (sizeof(void*) != 4)
            throw std::logic_error("Native data cells require their original x86 pointer layout");
        if (address < preferred_address)
            throw std::out_of_range("Native cell precedes its logical data image");
        const auto offset = std::uintptr_t(address) - preferred_address;
        if (offset > mapped_size_ || mapped_size_ - offset < sizeof(T))
            throw std::out_of_range("Native cell is outside its logical data image");
        auto* const bytes = mapping_.get() + offset;
        if (reinterpret_cast<std::uintptr_t>(bytes) % alignof(T))
            throw std::invalid_argument(
                "Native data cell alignment does not match its recovered layout");
        return *reinterpret_cast<T*>(bytes);
    }

protected:
    NativeDataImage(std::shared_ptr<std::byte>, std::size_t);

private:
    std::shared_ptr<std::byte> mapping_;
    std::size_t mapped_size_;
};

// Validate ownership before a view binds reference members into its cells.
NativeDataImage& require_native_data_image(const std::shared_ptr<NativeDataImage>&);
} //namespace skeet::engine

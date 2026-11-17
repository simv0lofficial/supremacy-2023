#pragma once
#include "skeet/engine/preserved_initialization.hpp"
#include "skeet/engine/unload.hpp"
#include <array>

namespace skeet::engine {
// Exact cells named by the26 ordinary CRT finalizers. This is a view of the
// canonical native data storage, not another set of hook metadata objects.
inline constexpr std::array<std::uint32_t, 26> hook_descriptor_cells{
    0x43475954, 0x43475b48, 0x43475878, 0x43475998, 0x43475830, 0x4347589c, 0x43475a28,
    0x43475b60, 0x43475864, 0x43475aec, 0x43475b18, 0x43475a08, 0x43475b34, 0x43475ac4,
    0x43475908, 0x434758f4, 0x434757f4, 0x434757c8, 0x43475a3c, 0x43475b04, 0x43475a68,
    0x434757b0, 0x434757e0, 0x43475814, 0x434758b8, 0x4347591c};

class HookDescriptorStorage {
public:
    explicit HookDescriptorStorage(PreservedInitialization&);
    explicit HookDescriptorStorage(std::shared_ptr<NativeDataImage>);
    HookDescriptorStorage(const HookDescriptorStorage&) = delete;
    HookDescriptorStorage& operator=(const HookDescriptorStorage&) = delete;
    HookDescriptorStorage(HookDescriptorStorage&&) = delete;
    HookDescriptorStorage& operator=(HookDescriptorStorage&&) = delete;

    VtableHookMetadata& at(std::uint32_t native_address) const;
    const NativeDataImage& image() const noexcept {
        return *image_;
    }

private:
    std::shared_ptr<NativeDataImage> image_;
};
}

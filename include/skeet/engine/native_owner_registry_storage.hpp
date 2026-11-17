#pragma once
#include "skeet/engine/owner_registry.hpp"
#include "skeet/engine/preserved_initialization.hpp"
#include <array>

namespace skeet::engine {
// These ten native begin/end/limit triplets are consumed by the ordinary
// registry-buffer finalizers. All construction, dispatch and cleanup users
// borrow the same native data bytes; this view creates no registry.
inline constexpr std::array<std::uint32_t, 10> native_owner_registry_cells{
    0x43475b78, // main lifecycle owners
    0x43475a9c, // HUD callbacks
    0x43475b94, // network callbacks
    0x43475988, // update callbacks
    0x4347584c, // packet callbacks
    0x43475ad8, // frame callbacks
    0x43475770, // map callbacks
    0x43475a84, // blur callbacks
    0x434758e0, // event callbacks
    0x43475930 // feedback position callbacks, constructor4347F752 ->433F1665
};

class NativeOwnerRegistryStorage {
public:
    explicit NativeOwnerRegistryStorage(PreservedInitialization&);
    explicit NativeOwnerRegistryStorage(std::shared_ptr<NativeDataImage>);
    NativeOwnerRegistryStorage(const NativeOwnerRegistryStorage&) = delete;
    NativeOwnerRegistryStorage& operator=(const NativeOwnerRegistryStorage&) = delete;
    NativeOwnerRegistryStorage(NativeOwnerRegistryStorage&&) = delete;
    NativeOwnerRegistryStorage& operator=(NativeOwnerRegistryStorage&&) = delete;

    OwnerRegistry& at(std::uint32_t native_begin_address) const;
    const NativeDataImage& image() const noexcept {
        return *image_;
    }

private:
    std::shared_ptr<NativeDataImage> image_;
};
}

#pragma once
#include "skeet/config_storage/native_blob.hpp"
#include "skeet/engine/preserved_initialization.hpp"
#include "skeet/menu/rage_clipboard.hpp"
#include <array>
#include <memory>

namespace skeet::client {
class NativeStartupSources;
struct ExitCallbacks;
}
namespace skeet::rage {
// One canonical producer for the tables43472B98 (tag12 descriptors) and
//4346EFE0 (enabled descriptors). Each blob header and pointer triple share
// their actual adjacent canonical cells; semantic blobs borrow that storage.
// Construction binds storage only. Individual CRT callbacks perform native
// registration/writes in the live scheduler order and append actual finalizers.
class GroupStorage {
public:
    GroupStorage(engine::PreservedInitialization&, engine::JobMemory);
    GroupStorage(std::shared_ptr<engine::NativeDataImage>, engine::JobMemory);
    GroupStorage(const GroupStorage&) = delete;
    GroupStorage& operator=(const GroupStorage&) = delete;
    std::span<BooleanConfig*, 12> enabled_descriptors() noexcept {
        return enabled_pointers_;
    }
    std::span<ConfigBlob*, 12> blob_descriptors() noexcept {
        return blob_pointers_;
    }
    ui::RageWeaponEnabledSettings& enabled_settings() noexcept {
        return enabled_;
    }
    void initialize_blob(std::size_t, ConfigLink&);
    void initialize_enabled(std::size_t, ConfigLink&) noexcept;
    void finalize_blob(std::size_t);
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }

private:
    std::shared_ptr<engine::NativeDataImage> image_;
    ui::RageWeaponEnabledSettings enabled_;
    std::array<NativeBlobBytes, 12> native_;
    std::array<ConfigBlob, 12> blobs_;
    std::array<BooleanConfig*, 12> enabled_pointers_;
    std::array<ConfigBlob*, 12> blob_pointers_;
};

// Retain this context and GroupStorage through the same ExitCallbacks::run()
// used by startup/shutdown. No grouped initialization runs when this is bound.
struct GroupInitializerBindings {
    ConfigRegistries& configuration;
    client::ExitCallbacks& callbacks;
    GroupStorage& groups;
    void (*source_finalizer(std::uint32_t native_address))();
};
void bind_native_group_initializers(client::NativeStartupSources&, GroupInitializerBindings&);
}

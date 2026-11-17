#pragma once
#include "icon_cache.hpp"
#include "startup_icons.hpp"
#include <memory>

namespace skeet::client {
class ClientRuntimeStorage;
}
namespace skeet::engine {
class NativeDataImage;
struct LoadedModuleEnvironment;
struct NativeImageEnvironment;
}
namespace skeet::ui {
class StartupRenderer;
class RendererContainer;
class RendererDeviceListener;
// Bind the actual loaded build's codec/job functions and allocator export,
// then borrow the canonical filesystem/ItemSystem slots used by initialization.
IconBindings initialize_build_icon_bindings(engine::NativeDataImage&);
IconBindings initialize_build_icon_bindings(engine::NativeDataImage&,
                                            const engine::LoadedModuleEnvironment&,
                                            engine::NativeImageEnvironment);

class RendererAssets {
public:
    explicit RendererAssets(client::ClientRuntimeStorage&);
    RendererAssets(client::ClientRuntimeStorage&, StartupRenderer&);
    // Explicit already-acquired engine boundaries for independent source
    // composition. Every pointer/reference must match its canonical cell;
    // the ordinary overload performs the genuine loaded-module discovery.
    RendererAssets(client::ClientRuntimeStorage&, StartupRenderer&, IconBindings);
    void bind_renderer(StartupRenderer&);
    void bind_renderer(StartupRenderer&, IconBindings);
    bool renderer_bound() const noexcept;
    ~RendererAssets();
    RendererAssets(const RendererAssets&) = delete;
    RendererAssets& operator=(const RendererAssets&) = delete;
    RendererAssets(RendererAssets&&) = delete;
    RendererAssets& operator=(RendererAssets&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    StartupRenderer& startup_renderer() const;
    RendererContainer& renderer() const;
    RendererDeviceListener& device_listener() const noexcept;
    IconCache& icon_cache() noexcept;
    StartupIcons& startup_icons() noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
}

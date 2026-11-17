#pragma once
#include "startup.hpp"
#include "skeet/engine/native_data_image.hpp"
#include "skeet/menu/window_lifetime.hpp"
#include <memory>

namespace skeet::ui {
class MainMenuWindowOwner;
class StartupRenderer;
}
namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;

// The reset paths, packet history and post-initialization callbacks borrow
// these same process cells. Constructing the view does not reset live state.
class PostInitializationData {
public:
    explicit PostInitializationData(std::shared_ptr<engine::NativeDataImage>);
    engine::NativeDataImage& data() const noexcept {
        return *data_;
    }
    PostInitializationState state() const;
    CommandResetState command_reset() const;
    std::span<StartupAcknowledgement, 150> acknowledgements() const;

private:
    std::shared_ptr<engine::NativeDataImage> data_;
};

struct PostInitializationImports {
    StartupImports startup;
    ui::WindowLifetimeImports window_lifetime;
};
PostInitializationImports native_post_initialization_imports();

// Compose the real4341F57F source callback after the menu and script owners
// exist. Construction binds dependencies only; the focus callback invokes
// runtime().initialize() at its actual position in engine startup.
class PostInitializationOwner {
public:
    PostInitializationOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                            ui::MainMenuWindowOwner&, ui::StartupRenderer&, ui::IconCache&,
                            ui::StartupIcons&, input::WindowSubclassRuntime&, ConfigStorageOwner&,
                            script::ScriptOwner&, ShutdownRuntime&);
    PostInitializationOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                            ui::MainMenuWindowOwner&, ui::StartupRenderer&, ui::IconCache&,
                            ui::StartupIcons&, input::WindowSubclassRuntime&, ConfigStorageOwner&,
                            script::ScriptOwner&, ShutdownRuntime&, PostInitializationImports);
    PostInitializationOwner(const PostInitializationOwner&) = delete;
    PostInitializationOwner& operator=(const PostInitializationOwner&) = delete;
    PostInitializationOwner(PostInitializationOwner&&) = delete;
    PostInitializationOwner& operator=(PostInitializationOwner&&) = delete;
    PostInitializationRuntime& runtime() noexcept {
        return runtime_;
    }
    ui::RootMenu& menu() const noexcept {
        return menu_;
    }
    engine::NativeDataImage& data() const noexcept {
        return data_.data();
    }

private:
    PostInitializationData data_;
    ui::RootMenu& menu_;
    PostInitializationRuntime runtime_;
};
} //namespace skeet::client

#pragma once
#include "skeet/effects/projectile_ribbons.hpp"
#include "skeet/engine/game_system.hpp"
#include "skeet/esp/indicators.hpp"
#include <memory>

namespace skeet::ui {
class StartupRenderer;
class RendererAssets;
class RendererBackend;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class ShutdownRuntime;
struct EngineRuntimeBindings;
struct GameSystemFactoryContext;

// Shared source views of the game-system projection and the real renderer.
// Feature factories borrow these stable objects rather than making independent
// camera, viewport, icon or indicator snapshots. Construction installs no hook
// and runs no feature constructor or game-system callback.
class VisualRuntimeOwner {
public:
    VisualRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                       ConfigurationRuntimeOwner&, ui::StartupRenderer&, ui::RendererAssets&);
    ~VisualRuntimeOwner();
    VisualRuntimeOwner(const VisualRuntimeOwner&) = delete;
    VisualRuntimeOwner& operator=(const VisualRuntimeOwner&) = delete;
    VisualRuntimeOwner(VisualRuntimeOwner&&) = delete;
    VisualRuntimeOwner& operator=(VisualRuntimeOwner&&) = delete;

    ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    ui::StartupRenderer& startup_renderer() const noexcept {
        return renderer_;
    }
    ui::RendererBackend& renderer() const;
    esp::Projection& projection() noexcept {
        return projection_;
    }
    const Point& screen_size() const noexcept {
        return screen_size_;
    }
    const esp::IndicatorLayout& indicators() const noexcept {
        return indicators_;
    }
    const esp::GrenadeWarningTexture& explosive_texture() const noexcept {
        return explosive_;
    }
    const esp::GrenadeWarningTexture& fire_texture() const noexcept {
        return fire_;
    }
    const effects::ProjectileRibbonBindings& ribbons() const noexcept {
        return ribbons_;
    }
    void* key_values_system() const noexcept {
        return key_values_;
    }
    engine::GameSystemCameraState camera_state() noexcept;

    // Refresh the source HUD views at its real renderer capture cut. Texture
    // dimensions used by indicators are still queried from the backend at each
    // native metric operation, including textures added by a Lua callback.
    void refresh();

    // Shutdown borrows the already constructed menu/window. Compose this part
    // after that real owner exists and before binding WorldRuntime factories;
    // no substitute shutdown context is created to break the lifetime cycle.
    void initialize_game_system(ShutdownRuntime&);
    engine::GameSystemRuntime& game_system();
    GameSystemFactoryContext game_system_factory();

private:
    ClientRuntimeStorage& storage_;
    ConfigurationRuntimeOwner& configuration_;
    ui::StartupRenderer& renderer_;
    ui::RendererAssets& assets_;
    void* key_values_;
    effects::ProjectileRibbonBindings ribbons_;
    esp::Projection projection_;
    Point screen_size_{};
    esp::IndicatorLayout indicators_;
    esp::GrenadeWarningTexture explosive_, fire_;
    std::unique_ptr<engine::GameSystemRuntime> game_system_;
};
} // namespace skeet::client

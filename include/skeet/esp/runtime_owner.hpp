#pragma once
#include "skeet/engine/build_esp_bindings.hpp"
#include <memory>

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
class VisualRuntimeOwner;
class CrtRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::ui {
class RendererAssets;
}
namespace skeet::esp {
struct EspOwnerBindings;
struct EspFrameOwner;
class EspOwnerRuntime;
// Production lifetime for all contexts borrowed by the complete43481EAB ESP
// factory. That factory creates the sole EnginePlayers and its sound/frame/
// sight endpoints. This context creates no feature/configuration registration,
// calls no game factory and installs no hook during construction.
class EspRuntimeOwner {
public:
    EspRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                    client::ConfigurationRuntimeOwner&, client::PlayerRuntimeOwner&,
                    engine::SceneQueryRuntimeOwner&, client::VisualRuntimeOwner&,
                    client::CrtRuntimeOwner&, script::SourceScriptRuntimeOwner&,
                    ui::RendererAssets&, engine::BuildEspBindings);
    ~EspRuntimeOwner();
    EspRuntimeOwner(const EspRuntimeOwner&) = delete;
    EspRuntimeOwner& operator=(const EspRuntimeOwner&) = delete;
    EspRuntimeOwner(EspRuntimeOwner&&) = delete;
    EspRuntimeOwner& operator=(EspRuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    EspOwnerBindings& factory_bindings() noexcept;
    EspOwnerRuntime*& active() const noexcept; //canonical434758DC
    EspFrameOwner& state() noexcept;
    effects::GlowRuntime& glow() noexcept;
    const rage::ShotgunPattern& shotgun_pattern() const noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} // namespace skeet::esp

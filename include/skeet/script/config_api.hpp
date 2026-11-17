#pragma once
#include "skeet/client/commands.hpp"
#include "skeet/config_storage/presets.hpp"
#include "skeet/config_storage/config_stream_view.hpp"
#include "skeet/menu/category_config.hpp"
#include "skeet/script/bridge.hpp"

namespace skeet::script {
struct ShutdownState;

struct ConfigBindings {
    ConfigRegistries& configuration;
    NamedConfigStore& named;
    ui::MenuWindow& window;
    ConfigPresetsServices& services;
    ConfigPresetsManager& presets;
    ConfigStreamView baseline_a; // live native43478CB8 vector
    ConfigStreamView baseline_b; // live owner ShutdownState.current_config
    void* engine; // native 4346A514, slot 26
    client::ScriptCommandEnvironment restriction;
};

// The module borrows the existing configuration, menu and preset owners.
class ConfigModule {
public:
    explicit ConfigModule(ConfigBindings bindings) : bindings_(bindings) {}
    void register_exports(Bridge&);
    // Called by ScriptModulesOwner after shutdown_ has been constructed and
    // before any runtime initialization can register or invoke Lua exports.
    void bind_current_config(ShutdownState&);
    void validate_streams(ConfigStreamView baseline_a, const ShutdownState&) const;
    int load(void* state); // 0x433F2180
    int export_values(void* state); // 0x4336593F
    int import_values(void* state); // 0x43382068

private:
    ConfigBindings bindings_;
    Bridge* bridge_ = nullptr;
    bool restricted() const;
    void restore_restricted(); // 0x4336E6CA
    void load_full(std::span<const std::byte>);
    void import_full(std::span<const std::byte>);
    static int load_entry(void*);
    static int export_entry(void*);
    static int import_entry(void*);
};

} // namespace skeet::script

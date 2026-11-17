#pragma once
#include "skeet/skins/catalog.hpp"
#include <map>
#include <vector>

namespace skeet::ui {
class SkinsMenu;
}
namespace skeet::skins {
struct SchemaItem {
    const char* type_token = nullptr;
    const char* base_name_token = nullptr;
    const char* view_model = nullptr;
    const char* world_model = nullptr;
};
class SchemaEngine {
public:
    virtual ~SchemaEngine() = default;
    virtual bool acquire_schema() = 0;
    virtual std::int32_t definition_count() = 0;
    virtual std::optional<SchemaItem> definition(std::uint32_t index) = 0;
    virtual const char* paintkit_token(std::uint32_t id) = 0;
    virtual std::optional<std::u16string> localize(const char* token) = 0;
};
class SchemaFiles {
public:
    virtual ~SchemaFiles() = default;
    virtual std::u16string current_directory() = 0;
    virtual std::optional<std::string> read_file(std::u16string_view path) = 0;
};
// Concrete 0x433CD70E compositor, including its static native agent additions
// and the items_game weapon_icons mapping parser.
class NativeCatalog final : public Catalog {
public:
    NativeCatalog(SchemaEngine& engine, SchemaFiles& files) : engine_(engine), files_(files) {}
    void initialize(ui::SkinsMenu& menu);
    bool available() override;
    std::uint32_t maximum_paintkit() const override {
        return maximum_;
    }
    std::span<const std::uint16_t> paintkits(std::uint16_t item) override;
    std::optional<std::u16string> paintkit_name(std::uint32_t id) override;
    std::span<const CatalogChoice> glove_models() override {
        return gloves_;
    }
    std::span<const CatalogChoice> agent_models(std::int32_t team) override;
    std::span<const std::string> agent_paths(std::int32_t team) const;
    // Native parser slice 0x433CD830..0x433CDB2B, also usable on captured data.
    void parse_weapon_icons(std::string_view text);

private:
    void append_native_agents(std::string_view data, std::int32_t team);
    SchemaEngine& engine_;
    SchemaFiles& files_;
    std::vector<CatalogChoice> gloves_, agents_t_, agents_ct_;
    std::vector<std::string> paths_t_, paths_ct_;
    std::map<std::uint16_t, std::vector<std::uint16_t>> paintkits_;
    std::uint32_t maximum_ = 0;
    bool native_agents_consumed_ = false;
};
} // namespace skeet::skins

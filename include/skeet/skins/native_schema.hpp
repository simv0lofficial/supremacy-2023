#pragma once
#include "skeet/skins/native_catalog.hpp"
#include "skeet/engine/interface_call.hpp"

namespace skeet::skins {
struct NativeSchemaBindings {
    using AcquireItemSystem = void*(__cdecl*)();
    using PaintkitDefinition = engine::MemberFunction<const void*, std::uint32_t>;
    void*& item_system; //shared cached43476424
    AcquireItemSystem const& acquire_item_system; //live resolved43468B9C
    PaintkitDefinition const& paintkit_definition; //live resolved43467040
    void* const& localization; //live4346A31C, slot11
};

// The concrete schema boundary used by433CD70E,43368201 and434084F0.
// All pointers above borrow the same engine cells as the cosmetic owners.
class NativeSchemaEngine final : public SchemaEngine {
public:
    explicit NativeSchemaEngine(NativeSchemaBindings bindings) noexcept : bindings_(bindings) {}
    bool acquire_schema() override;
    std::int32_t definition_count() override;
    std::optional<SchemaItem> definition(std::uint32_t id) override;
    const char* paintkit_token(std::uint32_t id) override;
    std::optional<std::u16string> localize(const char* token) override;

private:
    NativeSchemaBindings bindings_;
    void* schema_ = nullptr; //current acquisition;433CD70E keeps this in its local frame
};

//43493058 chooses GetCurrentDirectoryW's actual process directory and converts
// the supplied UTF16 DOS path to a counted NT name.433EBC7E is shared with the
// preset store; there is no second file reader or alternate catalog path.
class NativeSchemaFiles final : public SchemaFiles {
public:
    std::u16string current_directory() override;
    std::optional<std::string> read_file(std::u16string_view path) override;
};

// Own the adapter lifetimes required by the main menu's NativeCatalog& binding.
class NativeSchemaCatalog final {
public:
    explicit NativeSchemaCatalog(NativeSchemaBindings bindings) noexcept
        : engine_(bindings), catalog_(engine_, files_) {}
    NativeSchemaCatalog(const NativeSchemaCatalog&) = delete;
    NativeSchemaCatalog& operator=(const NativeSchemaCatalog&) = delete;
    NativeCatalog& catalog() noexcept {
        return catalog_;
    }

private:
    NativeSchemaEngine engine_;
    NativeSchemaFiles files_;
    NativeCatalog catalog_;
};
} //namespace skeet::skins

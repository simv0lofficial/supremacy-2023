#pragma once
#include <cstdint>
#include <string_view>

namespace skeet::skins {
class ModelInfo {
public:
    virtual ~ModelInfo() = default;
    virtual std::int32_t model_index(std::string_view path) = 0; // engine vtable +8
    virtual bool load_model(std::string_view path) = 0; // +0xAC
};
class StringTable {
public:
    virtual ~StringTable() = default;
    virtual const char* name() = 0;
    virtual std::int32_t add_string(std::int32_t flags, std::string_view path, std::int32_t length,
                                    const void* data) = 0;
};
class StringTables {
public:
    virtual ~StringTables() = default;
    virtual std::int32_t count() = 0;
    virtual StringTable* at(std::int32_t index) = 0;
};
// 0x43331B0C and 0x433AE8B7: find or load the model, then register it in
// the engine's modelprecache string table. All calls below cross engine APIs.
std::int32_t resolve_model_index(std::string_view path, ModelInfo&, StringTables&);
} // namespace skeet::skins

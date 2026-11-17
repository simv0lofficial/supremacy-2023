#pragma once
#include "skeet/engine/owner_registry.hpp"
#include <string>

namespace skeet::esp {
struct EspFrameOwner;
#if defined(_M_IX86) || defined(__i386__)
using MapScalarEntry = void(__vectorcall*)(void*, float);
using MapTextEntry = void(__fastcall*)(void*, const char*);
#else
using MapScalarEntry = void (*)(void*, float);
using MapTextEntry = void (*)(void*, const char*);
#endif
struct MapEntityParserTable {
    MapScalarEntry scalar;
    MapTextEntry text;
};
struct MapEntityParserEndpoint {
    const MapEntityParserTable* vtable = nullptr;
};
// Actual433817AA registration, vector43475770/74/78.
MapEntityParserEndpoint* register_map_entity_parser(engine::OwnerRegistryBinding,
                                                    MapEntityParserEndpoint*);
//43328352 forwards the stack float in XMM0;43325578 reads actual engine
//virtual99 and forwards the returned map-entity text in EDX when nonnull.
void dispatch_map_scalar(const engine::OwnerRegistry&, float);
void dispatch_map_entity_text(const engine::OwnerRegistry&, void* engine);

// Native43472B58 at ESP owner+30. Scalar slot433CCE8E is return-only;
//4339F24A parses the constructor-owned bomb-radius prefix and updates+6B0.
class EspMapEntityParser final : public MapEntityParserEndpoint {
public:
    EspMapEntityParser(engine::OwnerRegistryBinding, EspFrameOwner&, const std::string& prefix);

private:
#if defined(_M_IX86) || defined(__i386__)
    static void __vectorcall scalar(void*, float);
    static void __fastcall text(void*, const char*);
#else
    static void scalar(void*, float);
    static void text(void*, const char*);
#endif
    inline static constexpr MapEntityParserTable entries_{scalar, text};
    EspFrameOwner& owner_;
    const std::string& prefix_;
};
} //namespace skeet::esp

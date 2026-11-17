#include "skeet/script/entity_api.hpp"
#include "skeet/engine/offset_storage.hpp"
#include "skeet/esp/owner_runtime.hpp"
#include <stdexcept>

namespace skeet::script {
EntityPlayerSource EntityPlayerSource::canonical(esp::EspOwnerRuntime* const& slot) {
    return EntityPlayerSource(&slot);
}
esp::EnginePlayers& EntityPlayerSource::get() const {
    if (const auto* slot = std::get_if<esp::EspOwnerRuntime* const*>(&source_)) {
        auto* const owner = **slot;
        if (!owner)
            throw std::logic_error(
                "Lua entity player access requires the constructed canonical ESP owner");
        return owner->player_engine();
    }
    return *std::get<esp::EnginePlayers*>(source_);
}
const esp::PlayerDrawContext&
EntityPlayerSource::draw_context(const esp::PlayerDrawContext* standalone) const {
    if (const auto* slot = std::get_if<esp::EspOwnerRuntime* const*>(&source_)) {
        auto* const owner = **slot;
        if (!owner)
            throw std::logic_error("Lua entity flags require the constructed canonical ESP owner");
        return owner->frame_context().draw;
    }
    if (!standalone)
        throw std::logic_error(
            "Standalone Lua entity flags require their explicit player draw context");
    return *standalone;
}
bool EntityPlayerSource::is_canonical(esp::EspOwnerRuntime* const& slot) const noexcept {
    const auto* owner = std::get_if<esp::EspOwnerRuntime* const*>(&source_);
    return owner && *owner == &slot;
}
void bind_entity_offsets(EntityBindings& bindings, const engine::NativeOffsetStorage& storage) {
    const auto& globals = storage.globals;
    const auto decode = [&](std::uint32_t pointer, std::uint32_t key, std::uint32_t mask) {
        return static_cast<std::ptrdiff_t>(*globals.pointers.at(pointer) ^ globals.keys.at(key) ^
                                           mask);
    };
    bindings.offsets = {
        decode(0x43468e78, 0x434684e4, 0x9d920e35), decode(0x4346a5dc, 0x43468430, 0xcde290d2),
        decode(0x43468bf8, 0x43467b8c, 0xd070684c), decode(0x43467a74, 0x434678c0, 0xcd812654),
        decode(0x4346a724, 0x4346857c, 0xd266b537), decode(0x43467cb8, 0x4346a814, 0x9571d5f5),
        decode(0x434684f0, 0x4346a558, 0xc1c49a34), decode(0x4346716c, 0x434675b4, 0x9bb80163),
        decode(0x43467e8c, 0x43467900, 0x32e4daf1), decode(0x43468240, 0x4346a84c, 0xc2b6fb10),
        decode(0x43469064, 0x4346a484, 0x17978090)};
    bindings.targeting.offsets.team = decode(0x434688b0, 0x4346a518, 0x8b5586f6);
    bindings.targeting.offsets.survival_team = decode(0x434678ac, 0x43467b7c, 0x430c8993);
    bindings.targeting.entity_list = bindings.entity_list;
    bindings.byte_proxy = storage.property_proxy_slots[0];
    bindings.short_proxy = storage.property_proxy_slots[1];
}
} // namespace skeet::script

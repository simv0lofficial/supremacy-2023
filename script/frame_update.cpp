#include "skeet/script/frame_update.hpp"
#include "skeet/script/manager.hpp"

namespace skeet::script {
const engine::FrameUpdateEntry ScriptFrameUpdate::entries_[1]{update_entry};

ScriptFrameUpdate::ScriptFrameUpdate(ScriptManager& manager, std::uint8_t& reload,
                                     engine::OwnerRegistryBinding frames)
    : manager_(manager), reload_requested_(reload) {
    engine::register_frame_update(frames, this);
    vtable = entries_;
}

#if defined(_M_IX86) || defined(__i386__)
void __fastcall ScriptFrameUpdate::update_entry(void* receiver, std::uint8_t) {
#else
void ScriptFrameUpdate::update_entry(void* receiver, std::uint8_t) {
#endif
    auto& owner =
        *static_cast<ScriptFrameUpdate*>(static_cast<engine::FrameUpdateEndpoint*>(receiver));
    owner.manager_.update(owner.reload_requested_);
}
} // namespace skeet::script

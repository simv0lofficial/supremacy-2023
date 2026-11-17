#pragma once
#include "command_empty_tick_receiver.hpp"
#include "skeet/core.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/engine/preserved_initialization.hpp"

namespace skeet::misc {
class GrenadeCommandRuntime;
}
namespace skeet::effects {
struct EffectsCommandOwner;
}

namespace skeet::client {
//Separate native registry43475978/7C. These five interfaces are not command
//phase owners or prediction observers. Runtime construction order is supplied.
enum class CommandEmptyTickId : std::uint32_t {
    prediction_history = 0x43470470u, //PredictionHistoryOwner*
    legit = 0x434704B4u, //LegitCommandOwner*
    timer = 0x4346F3B0u, //CommandEmptyTickTimer*
    grenade = 0x434704BCu, //misc::GrenadeCommandRuntime*
    camera = 0x4347270Cu, //effects::EffectsCommandOwner*
};
struct PredictionHistoryOwner;
struct LegitCommandOwner;
//43398A4D's receiver is fullowner+20 and its only field is receiver+64.
struct CommandEmptyTickTimer {
    float& remaining;
    NativeCommandEmptyTickReceiver empty_tick_receiver{};
};

// Borrows actual43475978/7C/80. The buffer contains physical receiver pointers;
// construction neither initializes its cells nor allocates a parallel vector.
class CommandEmptyTickRegistry {
public:
    // input is the same address-stable CommandOwnerDispatch.input view of the
    // genuine key words and must outlive this registry and its receivers.
    CommandEmptyTickRegistry(engine::PreservedInitialization&, engine::JobMemory,
                             const InputBitmap&);
    CommandEmptyTickRegistry(std::shared_ptr<engine::NativeDataImage>, engine::JobMemory,
                             const InputBitmap&);
    CommandEmptyTickRegistry(const CommandEmptyTickRegistry&) = delete;
    CommandEmptyTickRegistry& operator=(const CommandEmptyTickRegistry&) = delete;
    CommandEmptyTickRegistry(CommandEmptyTickRegistry&&) = delete;
    CommandEmptyTickRegistry& operator=(CommandEmptyTickRegistry&&) = delete;
    void append(PredictionHistoryOwner&);
    void append(LegitCommandOwner&);
    void append(CommandEmptyTickTimer&);
    // Grenade registers before constructing its runtime. Its physical endpoint
    // belongs to the already constructed whole owner, not that runtime buffer.
    void append_grenade(NativeCommandEmptyTickReceiver&, misc::GrenadeCommandRuntime*);
    void append(effects::EffectsCommandOwner&);
    // Final table writes are separate from their earlier registrar calls.
    void publish(PredictionHistoryOwner&) const noexcept; //4347D086
    void publish(LegitCommandOwner&) const noexcept; //43484601
    void publish(CommandEmptyTickTimer&) const noexcept; //43485E1F
    void publish_grenade(NativeCommandEmptyTickReceiver&) const noexcept; //434867CD
    void publish(effects::EffectsCommandOwner&) const noexcept; //4348E766
    void require_sources() const;
    void dispatch(void* player, float sample_time, Command&, std::uint8_t& original_result) const;
    engine::OwnerRegistry& native_storage() const noexcept {
        return owners_;
    }
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }
    // Exact43349ECB, registered by433B22B6. Sole terminal native-buffer free;
    // the C++ destructor does not release it and all three cells stay intact.
    void finalize_storage() const;

private:
    std::shared_ptr<engine::NativeDataImage> image_;
    engine::OwnerRegistry& owners_;
    engine::JobMemory memory_;
    const InputBitmap& input_; // same live keyboard bitmap used by command dispatch
    void append_receiver(NativeCommandEmptyTickReceiver&, void* source);
};
void require_empty_tick_sources(const CommandEmptyTickRegistry&);
void dispatch_empty_tick_command(const CommandEmptyTickRegistry&, void* player, float sample_time,
                                 Command&, std::uint8_t& original_result);
} //namespace skeet::client

#pragma once
#include "skeet/client/startup_reset.hpp"
#include "skeet/misc/ping_spike.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace skeet::misc {
struct PingSpikeLatencySample {
    double sent_at;
    std::uint8_t present;
    std::array<std::byte, 7> preserved;
    double round_trip;
};
static_assert(sizeof(PingSpikeLatencySample) == 0x18);
struct PingSpikeLatencyFlow {
    double raw_average; //4347A500 /4347B118
    double smoothed; //4347A508 /4347B120
    std::int32_t sequence;
    std::uint32_t preserved;
    std::array<PingSpikeLatencySample, 128> samples;
};
static_assert(offsetof(PingSpikeLatencyFlow, samples) == 0x18);
static_assert(sizeof(PingSpikeLatencyFlow) == 0xC18);
using PingSpikeNetworkHistory = std::array<PingSpikeLatencyFlow, 2>;

// Genuine engine clock function resolved into43468C84, with an ST0 double
// result on x86. This is the original imported boundary, not a feature hook.
using PingSpikeNetworkClock = double (*)();
void record_ping_spike_sequences(PingSpikeNetworkHistory&, std::uint32_t direction,
                                 std::int32_t sequence, std::int32_t acknowledged,
                                 PingSpikeNetworkClock); //433F5077
void average_ping_spike_sequences(PingSpikeNetworkHistory&,
                                  std::uint32_t direction) noexcept; //433EA740

// Complete4333600F, also used by incoming-packet433CF7E4. Reads the real
// channel's virtual13 instruction operand to locate its two flow histories.
float query_channel_interpolation(void* engine_client, const PingSpikeNetworkHistory&);
std::int32_t acknowledge_datagram_sequence(std::span<const client::StartupAcknowledgement, 150>,
                                           std::int32_t sequence) noexcept; //4339C502

struct PingSpikeChannelBindings {
    PingSpikeNetworkHistory& history; //4347A500, both contiguous flows
    std::uint32_t& pending_rewind_skips; //4347C9F4, only low eight bits consumed
    float& interpolation; //43476928, shared lag/LEGIT/RAGE timing
    std::span<const client::StartupAcknowledgement, 150> acknowledgements; //43475BE0
    void* const& engine_client; //4346A514, read after original send
    PingSpikeNetworkClock const& clock; //live resolved43468C84 slot
};
class PingSpikeChannelRuntime {
public:
    PingSpikeChannelRuntime(PingSpikeRuntime&, PingSpikeChannelBindings);
    PingSpikeRuntime& feature;
    PingSpikeChannelBindings bindings;
    // Complete433204DC. Original SendDatagram is read from live channel hook
    // metadata[-2]->original(+8), slot46, after the optional sequence rewind.
    std::int32_t send_datagram(void* channel, void* datagram);
};
void bind_ping_spike_channel_runtime(PingSpikeChannelRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
std::int32_t __fastcall ping_spike_send_datagram_hook(void* channel, void*, void* datagram);
#else
std::int32_t ping_spike_send_datagram_hook(void* channel, void*, void* datagram);
#endif
} //namespace skeet::misc

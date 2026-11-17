#pragma once
#include "entity_state.hpp"
#include "timeline.hpp"
#include <memory>
#include <vector>

namespace skeet::lagcomp {
struct RecordRing {
    std::uint32_t first = 0;
    std::uint32_t count = 0;
    std::array<LagRecord*, 32> records{};
    LagRecord& at(std::uint32_t age) const noexcept {
        return *records[(first + age) & 31u];
    }
};
// The native global free list is 434797B8. Removed records are returned LIFO;
// their first dword temporarily stores the next native 32-bit record address.
// The host ownership vector keeps the same records alive across ring reuse.
class RecordPool {
public:
    LagRecord& acquire();
    void release(LagRecord&);
    std::size_t available() const noexcept {
        return free.size();
    }
    LagRecord* free_head() const noexcept {
        return free.empty() ? nullptr : free.back();
    }
    //4335BB22 consumes the free list after all64 canonical rings are drained.
    // Frees each returned source allocation once, in native LIFO order.
    void finalize_free_records();

private:
    std::vector<std::unique_ptr<LagRecord>> allocated;
    std::vector<LagRecord*> free;
};
LagRecord& push_history_record(RecordRing&, RecordPool&); // complete 4334C657
void pop_history_record(RecordRing&, RecordPool&); // complete 433B04F3
void clear_history_records(RecordRing&, RecordPool&); // 4341680F..4341682F
// 4340CEC3..4340CF87. Absolute wrapped clock differences prune records and
// packet samples once they reach one second, rounded to native tick units.
void prune_history_records(RecordRing&, RecordPool&, std::int32_t clock, float interval);
void prune_packet_samples(LagTimeline&, std::int32_t clock, float interval) noexcept;

struct ExtrapolationPlan {
    std::int32_t ticks = 0;
    bool inspected_third = false;
};
// Complete 4336C692..4336C8AE after the copied record has been selected.
// The layer values are read at their proven native offsets FC/104/108.
ExtrapolationPlan plan_history_extrapolation(const RecordRing&, std::uint32_t selected_ordinal,
                                             std::uint32_t options, float interval,
                                             std::int32_t prediction_limit) noexcept;

struct HistoryTickLiveGlobals {
    float* const& current_time; //43467EB8
    const float& interpolation; //43476928
    float* const& interval; //4346A338
    const std::int32_t& maximum_unlag_ms; //43468230
};
struct HistoryTickTiming {
    float current_time;
    float latency_198;
    float interpolation;
    float interval;
    std::int32_t maximum_unlag_ms;
    std::int32_t tick_d4;
    std::int32_t tick_18c;
    const HistoryTickLiveGlobals* live_globals = nullptr;
    const std::uint32_t* live_tick_d4 = nullptr;
    const std::uint32_t* live_tick_18c = nullptr;
};
HistoryTickTiming resolve_history_tick_timing(const HistoryTickTiming&) noexcept;
struct HistoryTickWindow {
    std::int32_t first, last, corrected;
};
// Complete 4339438E,4337EA43,4337F00E. The last helper deliberately truncates
// seconds to an integer before converting back to tick units.
HistoryTickWindow history_tick_window(const HistoryTickTiming&) noexcept;
struct TickTimingSamples {
    std::uint32_t first;
    std::int32_t count;
    std::array<float, 16> values;
};
// Complete43385D57. Empty/nonpositive count returns1. Conversion uses the
// current SSE rounding mode, including ties, rather than truncating.
std::int32_t mean_history_timing_ticks(const TickTimingSamples&) noexcept;

// Native repeated publication block 4341651A..434165CF (also434160D6..1618B).
void publish_record_flags(LagRecord&, const LagRecord* previous, bool compare_layer,
                          std::uint32_t update_flags, bool flag_43468d7c) noexcept;
struct HistoryRecordingState {
    RecordRing ring;
    LagRecord fallback{}; // native owner+0x660; receive tick aliases owner+0x668
    std::uint32_t flags = 0;
    std::uint8_t resolver_index = 0;
};
enum class HistoryCaptureKind {
    none,
    inserted,
    fallback
};
struct HistoryCaptureSlot {
    LagRecord* record;
    HistoryCaptureKind kind;
};
// Recording slot lifecycle43415FF9..43416064 /43416497..4341649E and clear
// 4341680F..43416836. The following actual entity capture and bone builds use
// EntityStateRuntime and build_deferred_history_record, respectively.
HistoryCaptureSlot prepare_history_capture(HistoryRecordingState&, RecordPool&, bool enabled,
                                           bool suppressed, std::int32_t simulation_tick,
                                           bool refresh_fallback);
// Complete fallback publication43416749..43416836. A newly captured fallback
// marks the owner for another update; an older eligible one replaces the full
// newest record and receives the native -1 receive-tick marker.
bool publish_history_fallback(HistoryRecordingState&, bool captured_fallback_now, void* engine,
                              float interval, float receive_interval,
                              std::int32_t maximum_unlag_ms);
} // namespace skeet::lagcomp

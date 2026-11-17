#pragma once
#include "records.hpp"
#include "skeet/engine/jobs.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace skeet::lagcomp {
// Native queue payload434161DC..434161EC /43416614..43416624. These are
// native x86 addresses; no worker state is appended to the three dwords.
struct HistoryJobItem {
    std::uint32_t player;
    float speed_fraction;
    std::uint32_t record;
};
static_assert(sizeof(HistoryJobItem) == 12);
static_assert(offsetof(HistoryJobItem, speed_fraction) == 4);
static_assert(offsetof(HistoryJobItem, record) == 8);

struct HistoryJobQueue {
    HistoryJobItem* begin;
    HistoryJobItem* end;
    HistoryJobItem* limit;
    std::uint32_t unknown_0c;
    std::array<HistoryJobItem, 64> items;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(HistoryJobQueue, items) == 0x10);
static_assert(sizeof(HistoryJobQueue) == 0x310);
#endif

// Complete433986B9. Initialization leaves the unknown dword and items alone.
void initialize_history_job_queue(HistoryJobQueue&) noexcept;
std::uint32_t history_job_count(const HistoryJobQueue&) noexcept;
// The two native append gates4341618B..434161EF /434165CF..43416627.
// A rejected append leaves the queue untouched; its caller executes the
// corresponding immediate recording branch, including its different setup.
bool append_history_job(HistoryJobQueue&, bool asynchronous, void* player, float speed_fraction,
                        LagRecord&) noexcept;

// Concrete source worker4336A5B3, defined by the canonical recording runtime.
// It reads the live process policy/cache at execution time, as native does.
#if defined(_M_IX86) || defined(__i386__)
void __fastcall execute_history_record_job(void* item);
#else
void execute_history_record_job(void* item);
#endif

// The dispatcher and its vtables must outlive every submitted task/handle.
// This is the native12-byte specialization, not one of the8/36-byte queues.
class HistoryJobDispatcher {
public:
    HistoryJobDispatcher(void* thread_pool, engine::JobMemory);
    HistoryJobDispatcher(const HistoryJobDispatcher&) = delete;
    HistoryJobDispatcher& operator=(const HistoryJobDispatcher&) = delete;
    engine::JobTask* create(); // 43387448 + complete4341F142
    void dispatch(engine::JobTask&, HistoryJobItem*, std::uint32_t count); // complete433943E6
private:
    struct TaskTable {
        std::array<void*, 4> methods;
        HistoryJobDispatcher* owner;
    } table_;
    std::array<void*, 4> destroyed_;
    void* pool_;
    engine::JobMemory memory_;
    static engine::JobTask* destroy(engine::JobTask&, std::uint8_t flags);
#if defined(_M_IX86) || defined(__i386__)
    static engine::JobTask* __thiscall destruction_entry(void*, std::uint8_t flags);
#else
    static engine::JobTask* destruction_entry(void*, std::uint8_t flags);
#endif
};
// Complete433DF5F6 /4339E700. Every claimed item uses the task's concrete
// worker, increments completed after return, then releases its task reference.
void consume_history_jobs(engine::JobTask&);
// Nonempty local-queue drain434169AD..43416A01. The valid native path requires
// a nonnull task allocation; no failure substitute is introduced. Queue
// pointers retain their native final values.
void dispatch_history_jobs(HistoryJobQueue&, HistoryJobDispatcher&);
} // namespace skeet::lagcomp

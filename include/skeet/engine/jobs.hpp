#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_JOB_FASTCALL __fastcall
#define SKEET_JOB_STDCALL __stdcall
#define SKEET_JOB_MEMBER __thiscall
#else
#define SKEET_JOB_FASTCALL
#define SKEET_JOB_STDCALL
#define SKEET_JOB_MEMBER
#endif

// The callback is the actual recovered item worker. Native433EDC66/433C131E
// pass the claimed item pointer in ECX; no dispatcher-owned replacement action.
using JobWork = void(SKEET_JOB_FASTCALL*)(void* item);

struct JobTask {
    void** vtable;
    std::uint32_t unknown_4;
    std::uint32_t references;
    JobWork work;
    std::uintptr_t cursor;
    std::uintptr_t end;
    std::uint32_t completed;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(JobTask) == 0x1c);
static_assert(offsetof(JobTask, references) == 8);
static_assert(offsetof(JobTask, work) == 0xc);
static_assert(offsetof(JobTask, cursor) == 0x10);
static_assert(offsetof(JobTask, end) == 0x14);
static_assert(offsetof(JobTask, completed) == 0x18);
#endif

struct JobMemory {
    using AllocateHeap = void*(SKEET_JOB_STDCALL*)(void*, std::uint32_t, std::size_t);
    using FreeHeap = unsigned char(SKEET_JOB_STDCALL*)(void*, std::uint32_t, void*);
    AllocateHeap allocate = nullptr;
    FreeHeap free = nullptr;
    void* process_heap = nullptr;
    void* purecall = nullptr; // actual linked CRT _purecall for the destroyed task table
};
JobMemory native_job_memory();
void* native_thread_pool(); // already loaded vstdlib!g_pThreadPool, dereferenced

// Complete43418461/43346F24. release calls actual task virtual[2](1) at zero.
std::int32_t retain_job(JobTask& task);
std::int32_t release_job(JobTask& task);
void consume_eight_byte_jobs(JobTask& task); // complete433EDC66 / thunk433E128F
void consume_thirty_six_byte_jobs(JobTask& task); // complete433C131E / thunk433E0A68

// The dispatcher and its vtables must outlive their tasks. Each task starts
// with the native caller reference1; caller releases it after dispatch. Tasks
// are single-use because native dispatch does not reset the completed counter.
class JobDispatcher {
public:
    JobDispatcher(void* thread_pool, JobMemory memory);
    JobDispatcher(const JobDispatcher&) = delete;
    JobDispatcher& operator=(const JobDispatcher&) = delete;
    JobTask* create_eight(JobWork work);
    JobTask* create_thirty_six(JobWork work);
    void dispatch_eight(JobTask& task, void* items, std::uint32_t count); // complete4338714F
    void dispatch_thirty_six(JobTask& task, void* items, std::uint32_t count); // complete4334B70B
    void* thread_pool() const noexcept {
        return pool_;
    }

private:
    struct TaskTable {
        std::array<void*, 4> methods;
        JobDispatcher* owner;
    } eight_, thirty_six_;
    std::array<void*, 4> destroyed_;
    void* pool_;
    JobMemory memory_;
    JobTask* create(JobWork work, TaskTable& table);
    void dispatch(JobTask& task, void* items, std::uint32_t count, std::uint32_t stride);
    static JobTask* destroy(JobTask& task, std::uint8_t flags);
    static JobTask* SKEET_JOB_MEMBER destruction_entry(void* object, std::uint8_t flags);
};

#undef SKEET_JOB_FASTCALL
#undef SKEET_JOB_STDCALL
#undef SKEET_JOB_MEMBER
} // namespace skeet::engine

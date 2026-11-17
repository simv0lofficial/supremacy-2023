#pragma once
#include "jobs.hpp"

namespace skeet::engine {
//43387448: zeroed process-heap storage with a16-byte aligned owner and the
//displacement/total-size words consumed by433837D0. Returns null on failure.
void* allocate_owner_storage(JobMemory, std::size_t owner_size);
//Destroy the C++ object before releasing its native allocation header.
void release_owner_storage(JobMemory, void*) noexcept;
}

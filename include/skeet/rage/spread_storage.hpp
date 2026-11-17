#pragma once
#include "spread.hpp"
#include "skeet/engine/jobs.hpp"
#include "skeet/engine/preserved_initialization.hpp"

namespace skeet::rage {
struct SpreadStorageBindings {
    // Genuine ntdll RtlAllocateHeap/RtlFreeHeap and PEB process-heap binding.
    engine::JobMemory memory;
    const SpreadRandom& random;
    const RageTrigonometry& trigonometry;
};

// Direct views of the retained initialization image's43478C60/43478C64 cells.
// Construction keeps that image alive without resetting either pointer.
// Consumer tables() views and the registered4337D954 finalizer use these same
// cells as the preserved VM; there is no second source pointer-cell owner.
class SpreadStorage {
public:
    explicit SpreadStorage(engine::PreservedInitialization&);
    explicit SpreadStorage(std::shared_ptr<engine::NativeDataImage>);
    SpreadStorage(const SpreadStorage&) = delete;
    SpreadStorage& operator=(const SpreadStorage&) = delete;
    SpreadStorage(SpreadStorage&&) = delete;
    SpreadStorage& operator=(SpreadStorage&&) = delete;

    // Complete43402E73. Ordinary startup supplies bound imports and successful
    // allocations. No allocation-failure continuation or reinit guard exists
    // in the recovered producer.
    // Native return is the address of the first pointer cell, not its payload.
    const SpreadSeed* const* initialize(const SpreadStorageBindings&);
    // Complete4337D954 ->433AD1CD ->433837D0. Explicit CRT finalization owns
    // this operation; ordinary C++ destruction does not add another cleanup.
    void finalize(const SpreadStorageBindings&);
    const SpreadTables& tables() const noexcept {
        return tables_;
    }
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }
    std::shared_ptr<engine::NativeDataImage> image_owner() const noexcept {
        return image_;
    }

private:
    std::shared_ptr<engine::NativeDataImage> image_;
    SpreadSeed*& standard_; //actual image cell43478C60
    SpreadSeed*& shotgun_patterns_; //actual image cell43478C64
    const SpreadTables tables_;
};
} // namespace skeet::rage

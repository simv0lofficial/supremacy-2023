#pragma once
#include "flags.hpp"
#include "skeet/engine/jobs.hpp"
#include <memory>

namespace skeet::client {
class NativeStartupSources;
struct ExitCallbacks;
}
namespace skeet::engine {
class PreservedInitialization;
class NativeDataImage;
}
namespace skeet::esp {
// Actual43479120 row representation. The fourth string word is not touched by
// the first15 constructors; the three trailing rows are cleared in full.
struct NativeFlagRow {
    std::uint32_t mask, color;
    std::uint32_t first, last, limit, preserved;
};
static_assert(sizeof(NativeFlagRow) == 24);
static_assert(offsetof(NativeFlagRow, first) == 8);
static_assert(offsetof(NativeFlagRow, preserved) == 20);

// This binds the retained native rows, without initializing or copying them.
// CRT43407D58 owns construction and registration of the terminal destructor.
// Destruction of the view itself does not release native string allocations.
class FlagTable {
public:
    FlagTable(engine::PreservedInitialization&, engine::JobMemory);
    FlagTable(std::shared_ptr<engine::NativeDataImage>, engine::JobMemory);
    FlagTable(const FlagTable&) = delete;
    FlagTable& operator=(const FlagTable&) = delete;
    FlagTable(FlagTable&&) = delete;
    FlagTable& operator=(FlagTable&&) = delete;
    std::span<NativeFlagRow, 18> rows() noexcept {
        return rows_;
    }
    std::span<const NativeFlagRow, 18> rows() const noexcept {
        return rows_;
    }
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }
    std::u16string_view text(std::size_t index) const noexcept;
    void initialize(); //complete43485097
    void finalize(); //43401BC2 ->433C9151, reverse18 terminal releases
private:
    void construct_text(NativeFlagRow&, const char16_t*);
    std::shared_ptr<engine::NativeDataImage> image_;
    engine::JobMemory memory_;
    std::array<NativeFlagRow, 18>& rows_;
    char16_t& shared_empty_; //actual434759AC
};

class FlagTableInitialization {
public:
    FlagTableInitialization(FlagTable& flags, client::ExitCallbacks& callbacks) noexcept
        : flags(flags), callbacks(callbacks) {}
    FlagTableInitialization(const FlagTableInitialization&) = delete;
    FlagTableInitialization& operator=(const FlagTableInitialization&) = delete;
    FlagTableInitialization(FlagTableInitialization&&) = delete;
    FlagTableInitialization& operator=(FlagTableInitialization&&) = delete;
    FlagTable& flags;
    client::ExitCallbacks& callbacks; //same512-slot process exit registry
    void (*source_finalizer(std::uint32_t native_address))();
};
// Individual ordinary callback43407D58 (operand1A8), not a grouped startup.
// Table, context and borrowed process heap must outliveExitCallbacks::run().
void bind_flag_table_initializer(client::NativeStartupSources&, FlagTableInitialization&);
} //namespace skeet::esp

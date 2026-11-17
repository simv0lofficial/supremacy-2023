#pragma once
#include "skeet/engine/native_data_image.hpp"
#include "skeet/script/owner.hpp"
#include <array>
#include <span>

namespace skeet::script {
namespace detail {
struct SourceScriptPe32Range {
    std::uintptr_t base;
    std::uint32_t size;
};
// Pure bounded PE-header validation, also used by the source fixture. The
// caller supplies header bytes already known to be readable; no address in
// these bytes is dereferenced. The anchor must belong to executable code.
SourceScriptPe32Range inspect_source_script_pe32_headers(std::span<const std::byte>,
                                                         std::uintptr_t module_base,
                                                         std::uintptr_t source_function);
// Complete bitmap effects of native 4339CE92 over one validated loaded image
// in the x86 address space. Marks VirtualSize in 128KB steps from section start;
// discardable sections are skipped. This does not publish a descriptor.
void mark_source_script_pe32_executable_sections(std::span<const std::byte>,
                                                 std::uintptr_t module_base,
                                                 ScriptExecutableBitmap&);
}

// Owns the real source DLL's script address bitmap. Construct before binding
// SourceScriptRuntimeOwner modules; keep it alive until ScriptOwner and its
// vectored exception handler have been removed. The enclosing DLL load must
// outlive this object, as it must outlive all other source callback owners.
// No module pin, self-unload, executable allocation or captured image is used.
// Construction enumerates the actual process loader modules and marks their
// nondiscardable executable sections before publishing the canonical bitmap.
class SourceScriptExecutableImage final {
public:
    static constexpr std::uint32_t bitmap_bits = 0x8000;
    static constexpr std::size_t bitmap_words = bitmap_bits / 32;
    explicit SourceScriptExecutableImage(std::shared_ptr<engine::NativeDataImage>);
    ~SourceScriptExecutableImage();
    SourceScriptExecutableImage(const SourceScriptExecutableImage&) = delete;
    SourceScriptExecutableImage& operator=(const SourceScriptExecutableImage&) = delete;
    SourceScriptExecutableImage(SourceScriptExecutableImage&&) = delete;
    SourceScriptExecutableImage& operator=(SourceScriptExecutableImage&&) = delete;

    engine::NativeDataImage& data() const noexcept {
        return *data_;
    }
    ScriptExecutableRange range() const;

private:
    std::shared_ptr<engine::NativeDataImage> data_;
    std::array<std::uint32_t, bitmap_words> buffer_{};
    std::uintptr_t image_base_ = 0;
    std::uint32_t image_size_ = 0;
};
} // namespace skeet::script

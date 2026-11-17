#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "skeet/script/source_executable_image.hpp"
#include "skeet/engine/loaded_modules.hpp"
#include <algorithm>
#include <cstring>
#include <exception>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace skeet::script {
namespace {
constexpr std::size_t maximum_header_bytes = 0x100000;
void require(bool condition, const char* reason) {
    if (!condition)
        throw std::invalid_argument(reason);
}
template <class T> T header_value(std::span<const std::byte> headers, std::size_t offset) {
    require(offset <= headers.size() && sizeof(T) <= headers.size() - offset,
            "Source script PE header read exceeds its bounded input");
    T value;
    std::memcpy(&value, headers.data() + offset, sizeof value);
    return value;
}
bool power_of_two(std::uint32_t value) noexcept {
    return value && !(value & (value - 1));
}
bool executable_protection(DWORD protection) noexcept {
    protection &= 0xFF;
    return protection == PAGE_EXECUTE || protection == PAGE_EXECUTE_READ ||
           protection == PAGE_EXECUTE_READWRITE || protection == PAGE_EXECUTE_WRITECOPY;
}
void require_readable_image(const void* allocation, const void* address, std::size_t size) {
    auto cursor = reinterpret_cast<std::uintptr_t>(address);
    require(size <= std::numeric_limits<std::uintptr_t>::max() - cursor,
            "Source script PE read wraps the address space");
    const auto end = cursor + size;
    while (cursor < end) {
        MEMORY_BASIC_INFORMATION region{};
        require(VirtualQuery(reinterpret_cast<const void*>(cursor), &region, sizeof region) ==
                        sizeof region &&
                    region.AllocationBase == allocation && region.Type == MEM_IMAGE &&
                    region.State == MEM_COMMIT && !(region.Protect & (PAGE_NOACCESS | PAGE_GUARD)),
                "Source script PE headers are not readable loaded-image memory");
        const auto protection = region.Protect & 0xFF;
        require(protection == PAGE_READONLY || protection == PAGE_READWRITE ||
                    protection == PAGE_WRITECOPY || protection == PAGE_EXECUTE_READ ||
                    protection == PAGE_EXECUTE_READWRITE || protection == PAGE_EXECUTE_WRITECOPY,
                "Source script PE headers have an unreadable protection");
        const auto begin = reinterpret_cast<std::uintptr_t>(region.BaseAddress);
        require(begin <= cursor && region.RegionSize &&
                    region.RegionSize <= std::numeric_limits<std::uintptr_t>::max() - begin &&
                    begin + region.RegionSize > cursor,
                "Source script PE memory region has invalid bounds");
        cursor = std::min(end, begin + static_cast<std::uintptr_t>(region.RegionSize));
    }
}
// Taking this genuine linked function's address identifies the source module.
// Its bytes are never called through a retained address or copied elsewhere.
__declspec(noinline) void source_script_image_anchor() {}
std::vector<std::byte> loaded_module_headers(HMODULE module) {
    auto* const base = reinterpret_cast<const std::byte*>(module);
    require_readable_image(module, base, sizeof(IMAGE_DOS_HEADER));
    IMAGE_DOS_HEADER dos;
    std::memcpy(&dos, base, sizeof dos);
    require(dos.e_magic == IMAGE_DOS_SIGNATURE && dos.e_lfanew >= static_cast<LONG>(sizeof dos) &&
                static_cast<std::size_t>(dos.e_lfanew) <=
                    maximum_header_bytes - sizeof(IMAGE_NT_HEADERS32),
            "Source script module has an unsupported DOS/NT header position");
    const auto nt_offset = static_cast<std::size_t>(dos.e_lfanew);
    require_readable_image(module, base + nt_offset, sizeof(IMAGE_NT_HEADERS32));
    IMAGE_NT_HEADERS32 nt;
    std::memcpy(&nt, base + nt_offset, sizeof nt);
    require(nt.OptionalHeader.SizeOfHeaders >= nt_offset + sizeof nt &&
                nt.OptionalHeader.SizeOfHeaders <= maximum_header_bytes,
            "Source script module has an invalid bounded PE header size");
    require_readable_image(module, base, nt.OptionalHeader.SizeOfHeaders);
    std::vector<std::byte> headers(nt.OptionalHeader.SizeOfHeaders);
    std::memcpy(headers.data(), base, headers.size());
    return headers;
}
struct ValidatedHeaders {
    std::uint32_t image_size;
    std::size_t section_offset, section_count;
};
ValidatedHeaders validate_headers(std::span<const std::byte> headers, std::uintptr_t module_base,
                                  bool source_module, std::uintptr_t source_function) {
    require(module_base && module_base <= std::numeric_limits<std::uint32_t>::max() &&
                !(module_base & 0xFFFF),
            "Source script image requires a valid x86 module allocation base");
    require(headers.size() <= maximum_header_bytes,
            "Source script PE headers exceed the validation bound");
    const auto dos = header_value<IMAGE_DOS_HEADER>(headers, 0);
    require(dos.e_magic == IMAGE_DOS_SIGNATURE && dos.e_lfanew >= static_cast<LONG>(sizeof dos),
            "Source script image has an invalid DOS header");
    const auto nt_offset = static_cast<std::size_t>(dos.e_lfanew);
    const auto nt = header_value<IMAGE_NT_HEADERS32>(headers, nt_offset);
    const auto& file = nt.FileHeader;
    const auto& optional = nt.OptionalHeader;
    const auto pe32 = file.Machine == IMAGE_FILE_MACHINE_I386 &&
                      file.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER32) &&
                      optional.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC;
    const auto pe64 = file.Machine == IMAGE_FILE_MACHINE_AMD64 &&
                      file.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER64) &&
                      optional.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC;
    // The source DLL must be x86. The native section visitor also accepts the
    // variable optional-header size of a low-address WOW64 support image.
    require(nt.Signature == IMAGE_NT_SIGNATURE && (pe32 || (!source_module && pe64)),
            "Source script image has an unsupported executable PE layout");
    require((file.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) &&
                (!source_module || (file.Characteristics & IMAGE_FILE_DLL)) &&
                file.NumberOfSections && file.NumberOfSections <= 96,
            "Source script image has invalid DLL characteristics or section count");
    require(optional.SectionAlignment >= 0x1000 && power_of_two(optional.SectionAlignment) &&
                optional.FileAlignment >= 0x200 && optional.FileAlignment <= 0x10000 &&
                power_of_two(optional.FileAlignment) &&
                optional.FileAlignment <= optional.SectionAlignment,
            "Source script PE alignments are unsupported");
    const auto size = optional.SizeOfImage;
    require(size >= (source_module ? 0x10000u : 0x1000u) &&
                !(size & (optional.SectionAlignment - 1)) &&
                size <= std::numeric_limits<std::uint32_t>::max() - module_base,
            "Source script image range is too small, unaligned or wraps x86 addresses");
    const auto section_offset =
        nt_offset + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + file.SizeOfOptionalHeader;
    const auto section_end =
        section_offset + std::size_t(file.NumberOfSections) * sizeof(IMAGE_SECTION_HEADER);
    require(optional.SizeOfHeaders >= section_end && optional.SizeOfHeaders <= headers.size() &&
                optional.SizeOfHeaders <= size &&
                !(optional.SizeOfHeaders & (optional.FileAlignment - 1)),
            "Source script PE section headers exceed their declared readable header range");
    require(!source_module ||
                (source_function >= module_base && source_function - module_base < size),
            "Source script function does not belong to its module range");
    const auto anchor_rva =
        source_module ? static_cast<std::uint32_t>(source_function - module_base) : 0;
    bool executable_anchor = false;
    std::uint64_t previous_end = optional.SizeOfHeaders;
    for (std::size_t index = 0; index < file.NumberOfSections; ++index) {
        const auto section = header_value<IMAGE_SECTION_HEADER>(
            headers, section_offset + index * sizeof(IMAGE_SECTION_HEADER));
        const auto extent = std::max(section.Misc.VirtualSize, section.SizeOfRawData);
        require(!(section.VirtualAddress & (optional.SectionAlignment - 1)) &&
                    section.VirtualAddress >= previous_end && section.VirtualAddress <= size &&
                    extent <= size - section.VirtualAddress,
                "Source script PE section is overlapping, misaligned or outside its image");
        previous_end = std::uint64_t(section.VirtualAddress) + extent;
        if (source_module && anchor_rva >= section.VirtualAddress &&
            anchor_rva - section.VirtualAddress < extent) {
            require((section.Characteristics & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ)) ==
                        (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ),
                    "Source script anchor belongs to a non-executable section");
            executable_anchor = true;
        }
    }
    require(!source_module || executable_anchor,
            "Source script anchor is not covered by an executable image section");
    return {size, section_offset, file.NumberOfSections};
}
detail::SourceScriptPe32Range source_module_range() {
    const auto* anchor = reinterpret_cast<const void*>(&source_script_image_anchor);
    HMODULE module = nullptr;
    require(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                   GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               reinterpret_cast<LPCWSTR>(anchor), &module) != 0 &&
                module,
            "Cannot identify the DLL containing the actual source script function");
    MEMORY_BASIC_INFORMATION code{};
    require(VirtualQuery(anchor, &code, sizeof code) == sizeof code &&
                code.AllocationBase == module && code.Type == MEM_IMAGE &&
                code.State == MEM_COMMIT && !(code.Protect & (PAGE_NOACCESS | PAGE_GUARD)) &&
                executable_protection(code.Protect),
            "Source script anchor is not loaded executable image code");
    const auto headers = loaded_module_headers(module);
    return detail::inspect_source_script_pe32_headers(headers,
                                                      reinterpret_cast<std::uintptr_t>(module),
                                                      reinterpret_cast<std::uintptr_t>(anchor));
}
struct ModuleBitmapContext {
    ScriptExecutableBitmap bitmap;
    detail::SourceScriptPe32Range source;
    std::exception_ptr failure;
    bool found_source = false;
};
void NTAPI mark_loader_module(engine::LoaderModule* module, void* opaque, BOOLEAN* stop) {
    auto& context = *static_cast<ModuleBitmapContext*>(opaque);
    try {
        require(module && module->base,
                "Script bitmap enumeration received an invalid loader module");
        const auto headers = loaded_module_headers(static_cast<HMODULE>(module->base));
        const auto base = reinterpret_cast<std::uintptr_t>(module->base);
        const auto image = validate_headers(headers, base, false, 0);
        require(image.image_size == module->image_size,
                "Script bitmap loader/PE image sizes disagree");
        detail::mark_source_script_pe32_executable_sections(headers, base, context.bitmap);
        if (base == context.source.base) {
            require(image.image_size == context.source.size,
                    "Script source module changed during bitmap enumeration");
            context.found_source = true;
        }
    } catch (...) {
        context.failure = std::current_exception();
        *stop = TRUE;
    }
}
void mark_loaded_modules(ScriptExecutableBitmap bitmap, detail::SourceScriptPe32Range source) {
    using Callback = void(NTAPI*)(engine::LoaderModule*, void*, BOOLEAN*);
    using Enumerate = LONG(NTAPI*)(ULONG, Callback, void*);
    const auto native = GetModuleHandleW(L"ntdll.dll");
    const auto enumerate =
        native ? reinterpret_cast<Enumerate>(GetProcAddress(native, "LdrEnumerateLoadedModules"))
               : nullptr;
    require(enumerate != nullptr,
            "Script executable bitmap requires the actual loader enumeration import");
    ModuleBitmapContext context{bitmap, source, {}, false};
    const auto status = enumerate(0, &mark_loader_module, &context);
    if (context.failure)
        std::rethrow_exception(context.failure);
    require(status >= 0 && context.found_source,
            "Script executable bitmap could not enumerate its real source module");
}
}

detail::SourceScriptPe32Range
detail::inspect_source_script_pe32_headers(std::span<const std::byte> headers,
                                           std::uintptr_t module_base,
                                           std::uintptr_t source_function) {
    return {module_base, validate_headers(headers, module_base, true, source_function).image_size};
}
void detail::mark_source_script_pe32_executable_sections(std::span<const std::byte> headers,
                                                         std::uintptr_t module_base,
                                                         ScriptExecutableBitmap& bitmap) {
    require(bitmap.bit_count == SourceScriptExecutableImage::bitmap_bits && bitmap.buffer,
            "Script executable-section marking requires the complete x86 address bitmap");
    const auto image = validate_headers(headers, module_base, false, 0);
    for (std::size_t index = 0; index < image.section_count; ++index) {
        const auto section = header_value<IMAGE_SECTION_HEADER>(
            headers, image.section_offset + index * sizeof(IMAGE_SECTION_HEADER));
        if ((section.Characteristics & IMAGE_SCN_MEM_DISCARDABLE) ||
            !(section.Characteristics & IMAGE_SCN_MEM_EXECUTE))
            continue;
        auto remaining = static_cast<std::uint32_t>(section.Misc.VirtualSize);
        auto address = std::uint64_t(module_base) + section.VirtualAddress;
        while (remaining) {
            const auto bit = static_cast<std::uint32_t>(address >> 17);
            bitmap.buffer[bit >> 5] |= std::uint32_t(1) << (bit & 31);
            remaining -= std::min(remaining, std::uint32_t(0x20000));
            address += 0x20000;
        }
    }
}

SourceScriptExecutableImage::SourceScriptExecutableImage(
    std::shared_ptr<engine::NativeDataImage> data)
    : data_(std::move(data)) {
    auto& image = engine::require_native_data_image(data_);
    require(sizeof(void*) == 4, "Source script executable ownership requires the original x86 ABI");
    auto& bitmap = image.native_cell<ScriptExecutableBitmap>(0x4346D638u);
    auto& base = image.native_cell<std::uintptr_t>(0x43467E70u);
    auto& size = image.native_cell<std::uint32_t>(0x434679DCu);
    require(!image.native_cell<ScriptOwner*>(0x43475B00u) && !bitmap.buffer &&
                (!bitmap.bit_count || bitmap.bit_count == bitmap_bits) && !base && !size,
            "Source script executable cells are already owned or active");
    const auto discovered = source_module_range();
    mark_loaded_modules({bitmap_bits, buffer_.data()}, discovered);
    image_base_ = discovered.base;
    image_size_ = discovered.size;
    // Original static data declares 0x8000 bits and a zeroed 0x1000-byte buffer.
    // Address >>17 indexes every 128KB block in the complete x86 address space.
    // Apply the recovered loaded-module visitor before publishing the bitmap;
    // ScriptOwner's process initializer later marks its own complete range.
    bitmap = {bitmap_bits, buffer_.data()};
    base = image_base_;
    size = image_size_;
}
SourceScriptExecutableImage::~SourceScriptExecutableImage() {
    auto& bitmap = data_->native_cell<ScriptExecutableBitmap>(0x4346D638u);
    if (bitmap.buffer != buffer_.data())
        return;
    bitmap = {0, nullptr};
    auto& base = data_->native_cell<std::uintptr_t>(0x43467E70u);
    auto& size = data_->native_cell<std::uint32_t>(0x434679DCu);
    if (base == image_base_ && size == image_size_) {
        base = 0;
        size = 0;
    }
}
ScriptExecutableRange SourceScriptExecutableImage::range() const {
    auto& bitmap = data_->native_cell<ScriptExecutableBitmap>(0x4346D638u);
    require(bitmap.buffer == buffer_.data() && bitmap.bit_count == bitmap_bits &&
                data_->native_cell<std::uintptr_t>(0x43467E70u) == image_base_ &&
                data_->native_cell<std::uint32_t>(0x434679DCu) == image_size_,
            "Source script executable publication no longer belongs to this owner");
    return {image_base_, image_size_, bitmap};
}
} // namespace skeet::script

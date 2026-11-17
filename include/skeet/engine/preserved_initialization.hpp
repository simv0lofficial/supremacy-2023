#pragma once
#include "skeet/engine/bindings.hpp"
#include "native_data_image.hpp"
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace skeet::engine {
// An already mapped input image and the lifetime of its installed live
// VM/exception environment. Mapping, imports, globals and exception dispatch
// are supplied by that environment, not constructed by this adapter.
//
// In the supplied bytes43493903 begins CC040051CC1000, so the image alone is
// not a callable operand decoder. The environment must implement its actual
// live transfer; the old ctx.h register/return-value replay is not that input.
class PreservedInitializationImage final : public NativeDataImage {
public:
    PreservedInitializationImage(std::shared_ptr<std::byte> mapped_image, std::size_t mapped_size,
                                 std::shared_ptr<const void> live_exception_environment);
    PreservedInitializationImage(const PreservedInitializationImage&) = delete;
    PreservedInitializationImage& operator=(const PreservedInitializationImage&) = delete;
    PreservedInitializationImage(PreservedInitializationImage&&) = delete;
    PreservedInitializationImage& operator=(PreservedInitializationImage&&) = delete;

private:
    std::shared_ptr<const void> exception_environment_;
};

// Required live protected-code boundary for ordinary source initialization.
// This class never decodes VM bytecode or supplies captured operand values.
// Entry-prefix checks do not establish that the remaining VM/environment is
// complete; its loader, exception handler and VM-to-source translation remain
// separate required work.
class PreservedInitialization final : public InitializationMetadata {
public:
    explicit PreservedInitialization(std::shared_ptr<PreservedInitializationImage>);
    PreservedInitialization(const PreservedInitialization&) = delete;
    PreservedInitialization& operator=(const PreservedInitialization&) = delete;
    PreservedInitialization(PreservedInitialization&&) = delete;
    PreservedInitialization& operator=(PreservedInitialization&&) = delete;

    //43493903: the ordinary caller passes one stack word and resumes without
    //adjusting ESP. The live environment must supply that effective return/
    //cleanup contract. No actual RET4 instruction has been proved at this
    //INT3 entry, and the old captured-return handler is not used here.
    std::uintptr_t operand(std::uint32_t address) override;
    void last_startup_callback(); //operand2D4,43484F06 ->4354DB7E
    const PreservedInitializationImage& image() const noexcept {
        return *image_;
    }
    std::shared_ptr<PreservedInitializationImage> image_owner() const noexcept {
        return image_;
    }

private:
    std::shared_ptr<PreservedInitializationImage> image_;
};
}

#include "skeet/script/required_module_storage.hpp"
#include <iterator>
#include <stdexcept>

namespace skeet::script {
namespace {
struct RequiredModuleSource {
    const char* name;
    std::size_t offset, size;
};
#include "required_module_data.inc"
static_assert(std::size(required_module_sources) == RequiredModuleStorage::module_count);
static_assert(std::size(required_module_bytes) == RequiredModuleStorage::source_size);
}

RequiredModuleStorage::RequiredModuleStorage() {
    modules_.reserve(module_count);
    for (const auto& source : required_module_sources) {
        const auto* begin =
            reinterpret_cast<const std::byte*>(required_module_bytes + source.offset);
        modules_.push_back({source.name, {begin, begin + source.size}, false});
    }
}

void RequiredModuleStorage::encrypt_for(ScriptRuntime& runtime) {
    if (runtime.required_modules.data() != modules_.data() ||
        runtime.required_modules.size() != modules_.size())
        throw std::invalid_argument(
            "Required modules must encrypt their canonical ScriptRuntime span");
    if (encrypted_)
        throw std::logic_error("Required modules have already been encrypted for this process");
    if (!runtime.cipher.query)
        throw std::invalid_argument(
            "Required modules require the actual process-times cipher query");
    for (const auto& module : modules_)
        if (module.encrypted)
            throw std::logic_error("Required module plaintext was already transformed");
    for (auto& module : modules_)
        runtime.encrypt_required(module);
    encrypted_ = true;
}
} //namespace skeet::script

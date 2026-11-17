#include "skeet/script/files.hpp"
#include "files_detail.hpp"
#include <cstring>

namespace skeet::script {
int FileModule::write(void* state) {
    if (api_->gettop(state) < 2)
        return 0;
    std::size_t length = 0;
    const auto* path = api_->checklstring(state, 1, &length);
    if (!path || !length || length >= 260)
        return 0;
    std::size_t size = 0;
    const auto* bytes = api_->checklstring(state, 2, &size);
    if (!bytes || !size || size > 0x640000)
        return 0;
    if (size > 2 && bytes[0] == 'M' && bytes[1] == 'Z')
        return 0;
    const auto name = files_detail::normalize({path, length});
    const auto* extension = std::strrchr(name.data(), '.');
    const auto hash = callback_hash(extension ? extension : name.data() + std::strlen(name.data()));
    if (hash == 0x7EC4899E || hash == 0xF6E4CAFA)
        return 0;
    files_detail::NativePath native;
    if (!files_detail::write_path(name, native))
        return 0;
    files_detail::File file{files_detail::open_write(native)};
    if (file.handle) {
        DWORD written = 0;
        WriteFile(file.handle, bytes, DWORD(size), &written, nullptr);
    }
    return 0;
}
} // namespace skeet::script

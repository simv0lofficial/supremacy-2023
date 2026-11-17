#include "skeet/script/files.hpp"
#include <array>
#include <stdexcept>

namespace skeet::script {
namespace {
void* (*export_context)(void*, int) = nullptr;
constexpr int globals = -10002, first_upvalue = -10003;
}
FileModule& FileModule::owner(void* state) {
    return *static_cast<FileModule*>(export_context(state, first_upvalue));
}
int FileModule::read_entry(void* state) {
    return owner(state).read(state);
}
int FileModule::write_entry(void* state) {
    return owner(state).write(state);
}
void FileModule::register_exports(Bridge& bridge) {
    if (!bridge.api || !bridge.state)
        throw std::logic_error("Lua files require an initialized state");
    api_ = bridge.api;
    export_context = api_->touserdata;
    constexpr std::array names{"readfile", "writefile"};
    constexpr std::array<LuaApi::CFunction, 2> entries{read_entry, write_entry};
    for (std::size_t i = 0; i < names.size(); ++i) {
        api_->pushlightuserdata(bridge.state, this);
        api_->pushcclosure(bridge.state, entries[i], 1);
        api_->setfield(bridge.state, globals, names[i]);
    }
}
} // namespace skeet::script

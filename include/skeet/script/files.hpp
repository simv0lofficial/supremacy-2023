#pragma once
#include "skeet/script/bridge.hpp"

namespace skeet::script {
// Global readfile/writefile, registered by 0x433D2335. Paths use the current
// process directory and the native UTF-8/NT conversion rules. The module must
// outlive its Lua state; constructing it opens no files.
class FileModule {
public:
    FileModule() = default;
    FileModule(const FileModule&) = delete;
    FileModule& operator=(const FileModule&) = delete;
    void register_exports(Bridge&);
    int read(void* state); // 0x433FA416
    int write(void* state); // 0x433D0059
private:
    const LuaApi* api_ = nullptr;
    static FileModule& owner(void*);
    static int read_entry(void*);
    static int write_entry(void*);
};
} // namespace skeet::script

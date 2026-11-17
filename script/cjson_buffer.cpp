#include "cjson_internal.hpp"
#include <bit>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace skeet::script::cjson_detail {
void terminate_json_process(unsigned int exit_code) {
    ::TerminateProcess(reinterpret_cast<HANDLE>(static_cast<std::intptr_t>(-1)), exit_code);
}
// Native compact buffer initialization 4332939E. Growth and dynamic ownership
// retain the Lua-CJSON strbuf contract without its configurable increment or
// diagnostic counters, which are absent from this client's four-field buffer.
void strbuf_init(strbuf_t* buffer, int length) {
    buffer->buf = nullptr;
    buffer->size =
        length > 0 ? std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(length) + 1u) : 1024;
    buffer->length = 0;
    buffer->dynamic = 0;
    buffer->buf = static_cast<char*>(std::malloc(buffer->size));
    if (!buffer->buf)
        terminate_json_process(0xfffffffb);
    buffer->buf[0] = 0;
}
strbuf_t* strbuf_new(int length) {
    auto* buffer = static_cast<strbuf_t*>(std::malloc(sizeof(strbuf_t)));
    if (!buffer)
        terminate_json_process(0xfffffffb);
    strbuf_init(buffer, length);
    buffer->dynamic = 1;
    return buffer;
}
void strbuf_free(strbuf_t* buffer) {
    if (buffer->buf) {
        std::free(buffer->buf);
        buffer->buf = nullptr;
    }
    if (buffer->dynamic)
        std::free(buffer);
}
void strbuf_resize(strbuf_t* buffer, int length) {
    if (length <= 0)
        terminate_json_process(0xfffffffb);
    const auto required = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(length) + 1u);
    auto capacity = buffer->size;
    if (capacity > required)
        capacity = required;
    else
        while (capacity < required)
            capacity = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(capacity) * 2u);
    if (buffer->size < capacity) {
        buffer->buf = static_cast<char*>(std::realloc(buffer->buf, capacity));
        if (!buffer->buf)
            terminate_json_process(0xfffffffb);
    }
    buffer->size = capacity;
}
void strbuf_append_string(strbuf_t* buffer, const char* text) {
    auto space = strbuf_empty_length(buffer);
    for (; *text; ++text) {
        if (space < 1) {
            strbuf_resize(buffer, buffer->length + 1);
            space = strbuf_empty_length(buffer);
        }
        buffer->buf[buffer->length++] = *text;
        --space;
    }
}
} // namespace skeet::script::cjson_detail

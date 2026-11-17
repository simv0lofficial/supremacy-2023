#include "skeet/script/database.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace skeet::script {
namespace {
bool supported(int type) {
    return type == 0 || type == 1 || type == 3 || type == 4 || type == 5;
}
void append32(DatabaseBytes& out, std::uint32_t value) {
    for (int shift = 0; shift < 32; shift += 8)
        out.push_back(std::byte(value >> shift));
}
struct CodecError : std::runtime_error {
    CodecError() : std::runtime_error("33s?!") {}
};
class Encoder {
public:
    const LuaApi& api;
    void* state;
    int strings, tables;
    std::uint32_t next_string = 1, next_table = 1;
    Encoder(const LuaApi& api, void* state) : api(api), state(state) {
        api.createtable(state, 0, 0);
        strings = api.gettop(state);
        api.createtable(state, 0, 0);
        tables = api.gettop(state);
    }
    void value(DatabaseBytes& out, int index) {
        if (index < 0)
            index = api.gettop(state) + index + 1;
        const auto type = api.type(state, index);
        if (!supported(type))
            return;
        out.push_back(std::byte(type));
        switch (type) {
        case 0:
            return;
        case 1:
            out.push_back(std::byte(api.toboolean(state, index)));
            return;
        case 3: {
            const auto number =
                std::bit_cast<std::array<std::byte, 8>>(api.checknumber(state, index));
            out.insert(out.end(), number.begin(), number.end());
            return;
        }
        case 4: {
            api.pushvalue(state, index);
            api.gettable(state, strings);
            if (api.type(state, -1) != 0) {
                out.back() = std::byte{0x10};
                append32(out, static_cast<std::uint32_t>(api.checkinteger(state, -1)));
                api.settop(state, -2);
                return;
            }
            api.settop(state, -2);
            api.pushvalue(state, index);
            api.pushinteger(state, next_string++);
            api.settable(state, strings);
            std::size_t length = 0;
            const auto* bytes = api.tolstring(state, index, &length);
            if (length > std::numeric_limits<std::uint32_t>::max())
                throw std::length_error("Database string exceeds uint32");
            append32(out, static_cast<std::uint32_t>(length));
            const auto* first = reinterpret_cast<const std::byte*>(bytes);
            out.insert(out.end(), first, first + length);
            return;
        }
        case 5: {
            api.pushvalue(state, index);
            api.gettable(state, tables);
            if (api.type(state, -1) != 0) {
                out.push_back(std::byte{1});
                append32(out, static_cast<std::uint32_t>(api.checkinteger(state, -1)));
                api.settop(state, -2);
                return;
            }
            api.settop(state, -2);
            api.pushvalue(state, index);
            api.pushinteger(state, next_table++);
            api.settable(state, tables);
            DatabaseBytes payload;
            api.pushnil(state);
            while (api.next(state, index)) {
                if (supported(api.type(state, -2)) && supported(api.type(state, -1))) {
                    value(payload, -2);
                    value(payload, -1);
                }
                api.settop(state, -2);
            }
            if (payload.size() > std::numeric_limits<std::uint32_t>::max())
                throw std::length_error("Database table exceeds uint32");
            out.push_back(std::byte{2});
            append32(out, static_cast<std::uint32_t>(payload.size()));
            out.insert(out.end(), payload.begin(), payload.end());
            return;
        }
        }
    }
};
class Decoder {
public:
    const LuaApi& api;
    void* state;
    std::span<const std::byte> bytes;
    std::size_t cursor = 0;
    int strings, tables;
    int next_string = 1, next_table = 1;
    Decoder(const LuaApi& api, void* state, std::span<const std::byte> bytes)
        : api(api), state(state), bytes(bytes) {
        api.createtable(state, 0, 0);
        strings = api.gettop(state);
        api.createtable(state, 0, 0);
        tables = api.gettop(state);
    }
    void require(std::size_t count) {
        if (cursor > bytes.size() || count > bytes.size() - cursor)
            throw CodecError();
    }
    unsigned byte() {
        require(1);
        return std::to_integer<unsigned>(bytes[cursor++]);
    }
    std::uint32_t word() {
        require(4);
        std::uint32_t result = 0;
        for (int shift = 0; shift < 32; shift += 8)
            result |= byte() << shift;
        return result;
    }
    void value() {
        switch (byte()) {
        case 0:
            api.pushnil(state);
            return;
        case 1:
            api.pushboolean(state, static_cast<signed char>(byte()));
            return;
        case 3: {
            require(8);
            std::array<std::byte, 8> data{};
            for (auto& item : data)
                item = bytes[cursor++];
            api.pushnumber(state, std::bit_cast<double>(data));
            return;
        }
        case 4: {
            const auto length = word();
            require(length);
            api.pushlstring(state, reinterpret_cast<const char*>(bytes.data() + cursor), length);
            cursor += length;
            api.pushvalue(state, -1);
            api.rawseti(state, strings, next_string++);
            return;
        }
        case 0x10:
            api.rawgeti(state, strings, static_cast<int>(word()));
            return;
        case 5: {
            const auto mode = byte();
            if (mode == 1) {
                api.rawgeti(state, tables, static_cast<int>(word()));
                return;
            }
            if (mode != 2)
                throw CodecError();
            const auto length = word();
            require(length);
            const auto end = cursor + length;
            api.createtable(state, 0, 0);
            const auto table = api.gettop(state);
            api.pushvalue(state, -1);
            api.rawseti(state, tables, next_table++);
            // 433E9322 uses the subtable payload as the recursive bound.
            const auto outer = bytes;
            bytes = bytes.first(end);
            while (cursor < end) {
                value();
                value();
                api.settable(state, table);
            }
            bytes = outer;
            return;
        }
        default:
            throw CodecError();
        }
    }
};
}

DatabaseBytes encode_database_value(const LuaApi& api, void* state, int index) {
    const auto top = api.gettop(state);
    if (index < 0)
        index = top + index + 1;
    if (api.type(state, index) <= 0)
        throw CodecError();
    DatabaseBytes bytes{std::byte{0x8E}};
    try {
        Encoder encoder(api, state);
        encoder.value(bytes, index);
    } catch (...) {
        api.settop(state, top);
        throw;
    }
    api.settop(state, top);
    return bytes;
}
void decode_database_value(const LuaApi& api, void* state, std::span<const std::byte> bytes) {
    const auto top = api.gettop(state);
    try {
        if (bytes.empty() || bytes.front() != std::byte{0x8E})
            throw CodecError();
        Decoder decoder(api, state, bytes.subspan(1));
        decoder.value();
        api.remove(state, top + 2);
        api.remove(state, top + 1);
    } catch (...) {
        api.settop(state, top);
        throw;
    }
}
} // namespace skeet::script

#include "client_detail.hpp"
#include "skeet/client/hitboxes.hpp"
#include <algorithm>
#include <stdexcept>
#include <ctime>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace skeet::script {
namespace {
std::uint64_t shared_time(std::uintptr_t address) {
    const auto* value = reinterpret_cast<const volatile std::uint32_t*>(address);
    std::uint32_t high, low;
    do {
        high = value[1];
        low = value[0];
    } while (high != value[2]);
    return (std::uint64_t(high) << 32) | low;
}
bool performance_counter(std::int64_t& value) {
    LARGE_INTEGER counter;
    counter.QuadPart = value;
    const auto result = QueryPerformanceCounter(&counter);
    value = counter.QuadPart;
    return result != FALSE;
}
std::uint64_t local_system_time() {
    return shared_time(0x7ffe0014);
}
std::uint64_t local_bias() {
    return shared_time(0x7ffe0020);
}
std::int32_t unix_seconds() {
    return static_cast<std::int32_t>(std::time(nullptr));
}
}
void initialize_client_clock(ClientClock& clock) { //4347E307
    LARGE_INTEGER frequency{};
    if (QueryPerformanceFrequency(&frequency))
        clock.milliseconds_per_count = 1000.0 / double(frequency.QuadPart);
    // The imported query writes the actual saved origin even when its return
    // value is false. A failure that leaves its output alone retains origin.
    performance_counter(clock.origin);
}
ClientClock client_clock_imports() noexcept {
    ClientClock clock;
    clock.counter = performance_counter;
    clock.system_time = local_system_time;
    clock.time_zone_bias = local_bias;
    clock.unix_seconds = unix_seconds;
    return clock;
}
ClientClock system_client_clock() {
    auto clock = client_clock_imports();
    initialize_client_clock(clock);
    return clock;
}
void ClientModule::seed_random() { //4341A641
    if (!bridge().frame_number)
        throw std::logic_error("Client random requires frame counter");
    const auto frame = *bridge().frame_number;
    if (random_seed_frame_ != frame) {
        if (!bindings.random.seed)
            throw std::logic_error("Client random requires vstdlib RandomSeed");
        bindings.random.seed(std::bit_cast<std::int32_t>(frame));
        random_seed_frame_ = *bridge().frame_number;
    }
}
int ClientModule::diagnostics(Export operation, void* state) {
    using namespace client_detail;
    const auto& api = *bridge().api;
    switch (operation) {
    case Export::draw_hitboxes: { //4334C981 parsing; concrete history/draw owner is shared
        if (api.gettop(state) <= 5)
            return 0;
        const auto index = std::uint32_t(api.tointeger(state, 1));
        if (index - 1 >= 63)
            return 0;
        auto* entity = engine::invoke<void*>(bindings.entity_list, 3, index);
        if (!entity)
            return 0;
        client::ClientHitboxesRequest request{};
        request.entity_index = std::int32_t(index);
        request.duration =
            minimum_number(maximum_number(float(api.tonumber(state, 2)), 0.0f), 60.0f);
        if (api.type(state, 3) == 5) {
            const auto count = static_cast<std::int32_t>(api.objlen(state, 3));
            for (std::int32_t element = 1; element <= count; ++element) {
                api.rawgeti(state, 3, element);
                const auto hitbox = std::uint32_t(api.tointeger(state, -1));
                if (hitbox < 19)
                    request.hitbox_mask |= 1u << hitbox;
                api.settop(state, -2);
            }
        } else {
            const auto hitbox = std::uint32_t(api.tointeger(state, 3));
            request.hitbox_mask = hitbox < 19 ? 1u << hitbox : 0xffffffffu;
        }
        const auto component = [&](int argument) {
            return std::uint32_t(
                (std::clamp)(std::int32_t(api.tointeger(state, argument)), 0, 255));
        };
        const auto count = api.gettop(state);
        const auto red = component(4), green = component(5), blue = component(6);
        const auto alpha = count > 6 ? component(7) : 150u;
        request.color = red | green << 8 | blue << 16 | alpha << 24;
        if (count > 7 && api.type(state, 8) > 0) {
            request.has_tick = true;
            request.tick = std::int32_t(api.checkinteger(state, 8));
        }
        if (count > 8 && api.type(state, 9) > 0) {
            request.flags |= 1;
            request.yaw = float(api.tonumber(state, 9));
        }
        if (count > 9 && api.type(state, 10) > 0) {
            request.alternative = std::int32_t(api.checkinteger(state, 10));
            if (std::uint32_t(request.alternative) < 3)
                request.flags |= 2;
        }
        if (!bindings.hitboxes)
            throw std::logic_error("Client hitboxes require ClientHitboxRuntime");
        client::draw_client_hitboxes(*bindings.hitboxes, entity, request);
        return 0;
    }
    case Export::timestamp: { //433C9E16
        std::int64_t counter = 0;
        double milliseconds = 0;
        if (!bindings.clock.counter)
            throw std::logic_error("Client clock requires QueryPerformanceCounter");
        if (bindings.clock.counter(counter)) {
            const auto delta = std::bit_cast<std::int64_t>(std::uint64_t(counter) -
                                                           std::uint64_t(bindings.clock.origin));
            milliseconds = double(delta) * bindings.clock.milliseconds_per_count;
        }
        api.pushnumber(state, milliseconds);
        return 1;
    }
    case Export::system_time: { //433FD4A8
        if (!bindings.clock.system_time || !bindings.clock.time_zone_bias)
            throw std::logic_error("Client clock requires Windows shared time");
        const auto ticks = std::bit_cast<std::int64_t>(bindings.clock.system_time() -
                                                       bindings.clock.time_zone_bias());
        const auto seconds = ticks / 10000000;
        const auto remainder = ticks % 10000000;
        const auto day = std::uint32_t(seconds % 86400);
        api.pushinteger(state, static_cast<std::int16_t>(day / 3600));
        api.pushinteger(state, static_cast<std::int16_t>((day % 3600) / 60));
        api.pushinteger(state, static_cast<std::int16_t>((day % 3600) % 60));
        api.pushinteger(state, static_cast<std::int16_t>(remainder / 10000));
        return 4;
    }
    case Export::unix_time: //4337D252
        if (!bindings.clock.unix_seconds)
            throw std::logic_error("Client clock requires CRT time");
        api.pushinteger(state, bindings.clock.unix_seconds());
        return 1;
    case Export::random_int: { //4334D2DD
        std::int32_t value = 0;
        if (api.gettop(state) >= 2) {
            seed_random();
            const auto minimum = std::int32_t(api.tointeger(state, 1));
            const auto maximum = std::int32_t(api.tointeger(state, 2));
            value = maximum;
            if (minimum < maximum) {
                if (!bindings.random.integer)
                    throw std::logic_error("Client random requires vstdlib RandomInt");
                value = bindings.random.integer(minimum, maximum);
            }
        }
        api.pushinteger(state, value);
        return 1;
    }
    case Export::random_float: { //433E091F
        float value = 0;
        if (api.gettop(state) >= 2) {
            seed_random();
            const auto minimum = float(api.tonumber(state, 1));
            const auto maximum = float(api.tonumber(state, 2));
            value = maximum;
            if (minimum < maximum) {
                if (!bindings.random.uniform)
                    throw std::logic_error("Client random requires vstdlib RandomFloat");
                value = bindings.random.uniform(minimum, maximum);
            }
        }
        api.pushnumber(state, double(value));
        return 1;
    }
    default:
        throw std::logic_error("Incorrect client diagnostic dispatch");
    }
}
} // namespace skeet::script

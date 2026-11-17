#include "skeet/script/flags.hpp"
#include "detail.hpp"
#include <array>
#include <bit>
#include <cwchar>

namespace skeet::script {
using namespace detail;

void draw_flags(Bridge& bridge, ReferenceStrings& strings, ui::RenderQueue& queue,
                std::int32_t entity_index, Point& cursor, std::int32_t line_height) {
    const std::lock_guard lock(bridge.vm_mutex);
    if (!bridge.state || strings.entries.empty())
        return;
    const auto& api = *bridge.api;
    void* const state = bridge.state;
    CallbackFrame guard{};
    guard.active.flags = 0x67736c6a;
    guard.saved_top = api.gettop(state);
    RtlPushFrame(&guard.active);
    auto invoke = [&] {
        for (std::size_t index = 0; index < strings.entries.size(); ++index) {
            api.rawgeti(state, -10000, strings.entries[index].reference);
            api.pushinteger(state, entity_index);
            if (callback_pcall(state, 1, 2) != 0) {
                report_callback_error(bridge, state);
                continue;
            }
            if (api.toboolean(state, -2)) {
                bool advance = true;
                const auto color = strings.entries[index].color;
                if (api.isstring(state, -1)) {
                    std::size_t length = 0;
                    const auto* text = api.tolstring(state, -1, &length);
                    if (!text || length == 0) {
                        advance = false;
                    } else {
                        std::array<wchar_t, 1026> buffer{};
                        // Native 0x4334DBF5 uses msvcrt vswprintf_s with L"%hs".
                        // This deliberately follows that CRT's multibyte locale.
                        const auto converted = swprintf_s(buffer.data(), 1024, L"%hs", text);
                        if (converted > 0) {
                            static_assert(sizeof(wchar_t) == sizeof(char16_t));
                            queue.text(cursor.x, cursor.y, color, 4, 0,
                                       {reinterpret_cast<const char16_t*>(buffer.data()),
                                        std::size_t(converted)});
                        }
                    }
                } else {
                    queue.text(cursor.x, cursor.y, color, 4, 0, strings.entries[index].text);
                }
                if (advance) {
                    const auto sum = std::uint32_t(cursor.y) + std::uint32_t(line_height);
                    cursor.y = std::bit_cast<std::int32_t>(sum);
                }
            }
            api.settop(state, -3);
        }
    };
    protected_region(invoke, guard);
    api.settop(state, guard.saved_top);
    RtlPopFrame(&guard.active);
}

} // namespace skeet::script

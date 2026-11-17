#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <cstdarg>

namespace skeet::script {
struct Bridge;
}
namespace skeet::misc {
struct HideFromObsConfig;
struct ColoredLogLine {
    float lifetime = 0;
    std::u16string text;
};
// The native HUD stores eight lines at +0x28, each as lifetime plus a UTF-16
// vector. Colors are encoded as U+0007 and eight hex digits inside that text.
class ColoredLogRing {
public:
    const std::uint8_t* enabled = nullptr; //live descriptor52EADF91.value
    std::uint32_t head = 0, count = 0;
    std::array<ColoredLogLine, 8> lines;
    std::array<char16_t, 16> hex{u'0', u'1', u'2', u'3', u'4', u'5', u'6', u'7',
                                 u'8', u'9', u'A', u'B', u'C', u'D', u'E', u'F'};
    void append(const char* utf8, std::uint32_t color); // complete 0x43387EF4 chain
private:
    void append_segment(ColoredLogLine&, std::u16string_view, std::uint32_t color);
};
class ErrorLog {
public:
    const std::uint8_t* disabled = nullptr; //canonical643364E6.value,43468E58
    void* cvar = nullptr; // 0x4346A87C
    void* surface = nullptr; // 0x434689C0
    const float* current_time = nullptr; // *0x4346A334
    float last_sound = 0; // 0x4347CA34
    ColoredLogRing* const* hud_slot = nullptr; //live434758CC, value may be null
    void write(std::string_view text);
    void format(const char* format, ...); // complete 0x43416C1C
    void vformat(const char* format, std::va_list arguments);
};
class NormalLog {
public:
    const std::uint8_t* disabled = nullptr; //canonical643364E6.value,43468E58
    void* cvar = nullptr;
    ColoredLogRing* const* hud_slot = nullptr; //same live434758CC as ErrorLog
    script::Bridge* script = nullptr;
    void write(std::string_view text);
    void format(const char* format, ...); // complete 0x4339DC04
    void vformat(const char* format, std::va_list arguments);
    void console_format(const char* format, ...);
};
void bind_log_configuration(const HideFromObsConfig&, ErrorLog&, NormalLog&) noexcept;
void bind_log_hud(ColoredLogRing* const& canonical_slot, ErrorLog&, NormalLog&) noexcept;
} // namespace skeet::misc

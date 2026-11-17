#pragma once
#include "command_transform.hpp"
#include "command_weapon_postprocess.hpp"

namespace skeet::client {
// Inline 433D415B..433D4377. The previous command comes directly from
// CInput +F0, with an exact unsigned command-number check. This phase changes
// the command only; native global 434759C0 is separate from frame.flags.
void retain_command_attack_angles(const CommandFrame&, Command&, const void* player,
                                  const CommandMouseOffsets&, const void* input) noexcept;

// Inline gate 433D4377..433D4396, including its branch at 433D4600. variable
// is the resolved 43468E94 XOR 2DC3CC20 ConVar, which may be null. Its terminal
// +30 encoded word is tested by sign, before the command's sentinel tick.
bool can_predict_command_copy(const Command&, const void* variable) noexcept;

// Exact 433F27C3/4341873D optional 434683D0 byte scope. Create it immediately
// before prepare_prediction_command_copy and retain it through native 4DE5;
// CommandPrediction ends earlier, at 46F4..4709.
class CommandCopyOverrideScope {
public:
    explicit CommandCopyOverrideScope(std::uint8_t* target) noexcept;
    ~CommandCopyOverrideScope() {
        restore();
    }
    CommandCopyOverrideScope(const CommandCopyOverrideScope&) = delete;
    CommandCopyOverrideScope& operator=(const CommandCopyOverrideScope&) = delete;
    void restore() noexcept;

private:
    std::uint8_t* target;
    std::uint8_t saved;
};

// Inline 433D43A2..433D4404, after a successful gate and override entry.
// Copies all 0x64 command bytes, reorients from frame+118 with the captured
// movement type, and sanitizes using frame.flags bit 3. Native always supplies
// true for the grounded argument here; force buttons is send_packet==0.
void prepare_prediction_command_copy(const CommandFrame&, const Command&, Command& copy,
                                     std::uint8_t send_packet, const CommandButtonPolicy&,
                                     const CommandTrigonometry&, const ScriptCommandEnvironment&);
} // namespace skeet::client

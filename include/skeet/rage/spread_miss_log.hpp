#pragma once

namespace skeet::misc {
class NormalLog;
}
namespace skeet::rage {
struct ShotLogSettings;
//43386C0F..43386C65, after every hitbox missed the reconstructed bullet ray.
// This log precedes aim_miss and is independent of the shot's notified bit20.
void log_spread_miss(const ShotLogSettings&, misc::NormalLog&);
}

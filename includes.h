#pragma once

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <cstdint>
#include <intrin.h>
#include <xmmintrin.h>
#include <array>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>
#include <chrono>
#include <thread>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <deque>
#include <functional>
#include <map>
#include <filesystem>

// Utilities
#include "util/address.h"

// Core & Features (Supremacy Monolithic Layout)
#include "csgo.h"
#include "client.h"
#include "math.h"
#include "pred.h"
#include "bonesetup.h"
#include "lagcomp.h"
#include "lagrecord.h"
#include "resolver.h"
#include "penetration.h"
#include "aimbot.h"
#include "shots.h"
#include "hvh.h"
#include "movement.h"
#include "visuals.h"
#include "chams.h"
#include "skins.h"
#include "events.h"
#include "render.h"



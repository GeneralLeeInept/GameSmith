#pragma once

#include "gamesmith/core/config.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef GS_PLATFORM_WINDOWS
#define NOMINMAX
#include <Windows.h>
#endif

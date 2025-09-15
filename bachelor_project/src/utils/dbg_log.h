#pragma once

#include "defines.h"
#include <cstdio>

#if DEBUG_LOG

#define dbg_log(...) { printf(__VA_ARGS__); printf("\n"); }

#else

#define dbg_log(...) {}

#endif
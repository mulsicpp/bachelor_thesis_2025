#pragma once

#include "defines.h"
#include <cstdio>

#ifdef USE_LOGGING

#define dbg_log(format, ...) printf(format"\n", __VA_ARGS__)

#else

#define dbg_log(...)

#endif
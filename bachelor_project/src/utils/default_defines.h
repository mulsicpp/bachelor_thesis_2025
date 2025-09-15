#pragma once

#ifdef NDEBUG

#ifndef DEBUG_VULKAN
#define DEBUG_VULKAN false
#endif

#ifndef DEBUG_LOG
#define DEBUG_LOG false
#endif

#ifndef DEBUG_MOVE
#define DEBUG_MOVE false
#endif

#else

#ifndef DEBUG_VULKAN
#define DEBUG_VULKAN true
#endif

#ifndef DEBUG_LOG
#define DEBUG_LOG true
#endif

#ifndef DEBUG_MOVE
#define DEBUG_MOVE true
#endif

#endif

#ifdef _WIN32 

#ifndef HEADLESS
#define HEADLESS false
#endif

#else

#define HEADLESS true

#endif
#pragma once

#ifndef _WIN32
#error "Only supported on windows"
#endif

#ifdef NDEBUG

#ifndef USE_VALIDATION_LAYERS
#define USE_VALIDATION_LAYERS false
#endif

#ifndef USE_LOGGING
#define USE_LOGGING false
#endif

#else

#ifndef USE_VALIDATION_LAYERS
#define USE_VALIDATION_LAYERS true
#endif

#ifndef USE_LOGGING
#define USE_LOGGING true
#endif

#endif 

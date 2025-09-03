#pragma once

// #define DEBUG_LOG false
#define DEBUG_MOVE false

// #define HEADLESS true

#include "default_defines.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT  720


#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define DEFAULT_RESOLUTION_STR TOSTRING(IMAGE_WIDTH) " " TOSTRING(IMAGE_HEIGHT)

#define APP_NAME "Raytracing App"
#define APP_DESCRIPTION "An app to benchmark raytracing in different dynamic scenes"
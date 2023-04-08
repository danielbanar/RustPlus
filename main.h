#pragma once
#include <tuple>
#include <array>
#include <fstream>
#include <windowsx.h>
#include <inttypes.h>
#include <functional>
#include <iomanip>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include "rustsocket.h"
#include "cameradecoder.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

RustSocket* rs;
Uint32 SubscribeRepeat(Uint32 interval, void* name);

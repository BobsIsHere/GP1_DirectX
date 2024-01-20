#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>
#define NOMINMAX  //for directx

// SDL Headers
#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_surface.h"
#include "SDL_image.h"

// DirectX Headers
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>

// Framework Headers
#include "Timer.h"
#include "Math.h"

//Defines
#define GREEN_COLOR_TEXT "\033[1;92m"
#define BLUE_COLOR_TEXT "\033[1;94m"
#define RED_COLOR_TEXT "\033[1;91m" 
#define RESET_COLOR_TEXT "\033[0m" 
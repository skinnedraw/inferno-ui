#pragma once
#include <mutex>
#include <clipper2/clipper.h>
#include "Security/Vms/VMProtectSDK.h"
#include "Security/Backend/Backend.h"
#include "Security/xorstr.hpp"
#include "Logging.h"
#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include "Libs/stb_image.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/imgui.h"
#include <ImGui/imgui_internal.h>
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "CKeybind.hpp"
#include "Overlay/Overlay.hpp"

#include "Globals.hpp"

#include <vector>
#include <thread>
#include <bit>
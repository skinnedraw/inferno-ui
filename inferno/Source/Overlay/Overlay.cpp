#include "Overlay.hpp"
#include "../Includes.hpp"
#include <dwmapi.h>
#include "ImGui/imgui_freetype.h"
#include "Fonts/smallestpixel.h"
#include "Fonts/visitor1.h"
#include "Fonts/LatoBold.h"
#include "Images/BladeeRaw.h"
#include <fstream>
#include "Fonts/Icons/IconsFontAwesome6.h"
#include "Fonts/Icons/IconsFontAwesome6_Bytes.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Overlay
{
    inline ID3D11Device* g_pd3dDevice = nullptr;
    inline ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    inline IDXGISwapChain* g_pSwapChain = nullptr;
    inline bool                     g_SwapChainOccluded = false;
    inline UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
    inline ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
    void CreateRenderTarget()
    {
        ID3D11Texture2D* pBackBuffer;
        g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
    bool CreateDeviceD3D(HWND hWnd)
    {

        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
        HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
        if (res == DXGI_ERROR_UNSUPPORTED)
            res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
        if (res != S_OK)
            return false;

        CreateRenderTarget();
        return true;
    }
    void CleanupRenderTarget()
    {
        if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
    }
    void CleanupDeviceD3D()
    {
        CleanupRenderTarget();
        if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
        if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
        if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    }




    
    // ImGui buttons function


    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
                return 0;
            g_ResizeWidth = (UINT)LOWORD(lParam);
            g_ResizeHeight = (UINT)HIWORD(lParam);
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        }
        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    bool full_screen(HWND windowHandle)
    {
        MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
        if (GetMonitorInfo(MonitorFromWindow(windowHandle, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
        {
            RECT windowRect;
            if (GetWindowRect(windowHandle, &windowRect))
            {
                return windowRect.left == monitorInfo.rcMonitor.left
                    && windowRect.right == monitorInfo.rcMonitor.right
                    && windowRect.top == monitorInfo.rcMonitor.top
                    && windowRect.bottom == monitorInfo.rcMonitor.bottom;
            }
        }
    }


    void move_window(HWND hw)
    {
        HWND target = Globals::RobloxWindow;
        HWND foregroundWindow = GetForegroundWindow();

        if (target != foregroundWindow && hw != foregroundWindow)
        {
            MoveWindow(hw, 0, 0, 0, 0, true);
            return;
        }

        RECT rect;
        if (!GetWindowRect(target, &rect))
        {
            std::cout << "[Athena] https://discord.gg/XhVvn7McKj GetWindowRect() Failed." << std::endl;
            return;
        }

        int rsize_x = rect.right - rect.left - 17;
        int rsize_y = rect.bottom - rect.top;

        if (full_screen(target))
        {
            rsize_x += 16;
        }
        else
        {
            rsize_y -= 39;
            rect.left += 4 + 5;
            rect.top += 31;
        }

        if (!MoveWindow(hw, rect.left, rect.top, rsize_x, rsize_y, TRUE))
        {
            std::cout << "[Athena] https://discord.gg/XhVvn7McKj Could not move window." << std::endl;
        }


    }

}
static const char* KeyNames[] = {
    "OFF",
    "LBUTTON",
    "RBUTTON",
    "CANCEL",
    "MBUTTON",
    "XBUTTON1",
    "XBUTTON2",
    "Unknown",
    "BACK",
    "TAB",
    "Unknown",
    "Unknown",
    "CLEAR",
    "RETURN",
    "Unknown",
    "Unknown",
    "SHIFT",
    "CONTROL",
    "MENU",
    "PAUSE",
    "CAPITAL",
    "KANA",
    "Unknown",
    "JUNJA",
    "FINAL",
    "KANJI",
    "Unknown",
    "ESCAPE",
    "CONVERT",
    "NONCONVERT",
    "ACCEPT",
    "MODECHANGE",
    "SPACE",
    "PRIOR",
    "NEXT",
    "END",
    "HOME",
    "LEFT",
    "UP",
    "RIGHT",
    "DOWN",
    "SELECT",
    "PRINT",
    "EXECUTE",
    "SNAPSHOT",
    "INSERT",
    "DELETE",
    "HELP",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "LWIN",
    "RWIN",
    "APPS",
    "Unknown",
    "SLEEP",
    "NUMPAD0",
    "NUMPAD1",
    "NUMPAD2",
    "NUMPAD3",
    "NUMPAD4",
    "NUMPAD5",
    "NUMPAD6",
    "NUMPAD7",
    "NUMPAD8",
    "NUMPAD9",
    "MULTIPLY",
    "ADD",
    "SEPARATOR",
    "SUBTRACT",
    "DECIMAL",
    "DIVIDE",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "NUMLOCK",
    "SCROLL",
    "OEM_NEC_EQUAL",
    "OEM_FJ_MASSHOU",
    "OEM_FJ_TOUROKU",
    "OEM_FJ_LOYA",
    "OEM_FJ_ROYA",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "LSHIFT",
    "RSHIFT",
    "LCONTROL",
    "RCONTROL",
    "LMENU",
    "RMENU"
};
static const int KeyCodes[] = {
    0x0,  //Undefined
    0x01,
    0x02,
    0x03,
    0x04,
    0x05,
    0x06,
    0x07, //Undefined
    0x08,
    0x09,
    0x0A, //Reserved
    0x0B, //Reserved
    0x0C,
    0x0D,
    0x0E, //Undefined
    0x0F, //Undefined
    0x10,
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,
    0x16, //IME On
    0x17,
    0x18,
    0x19,
    0x1A, //IME Off
    0x1B,
    0x1C,
    0x1D,
    0x1E,
    0x1F,
    0x20,
    0x21,
    0x22,
    0x23,
    0x24,
    0x25,
    0x26,
    0x27,
    0x28,
    0x29,
    0x2A,
    0x2B,
    0x2C,
    0x2D,
    0x2E,
    0x2F,
    0x30,
    0x31,
    0x32,
    0x33,
    0x34,
    0x35,
    0x36,
    0x37,
    0x38,
    0x39,
    0x3A, //Undefined
    0x3B, //Undefined
    0x3C, //Undefined
    0x3D, //Undefined
    0x3E, //Undefined
    0x3F, //Undefined
    0x40, //Undefined
    0x41,
    0x42,
    0x43,
    0x44,
    0x45,
    0x46,
    0x47,
    0x48,
    0x49,
    0x4A,
    0x4B,
    0x4C,
    0x4B,
    0x4E,
    0x4F,
    0x50,
    0x51,
    0x52,
    0x53,
    0x54,
    0x55,
    0x56,
    0x57,
    0x58,
    0x59,
    0x5A,
    0x5B,
    0x5C,
    0x5D,
    0x5E, //Rservered
    0x5F,
    0x60, //Numpad1
    0x61, //Numpad2
    0x62, //Numpad3
    0x63, //Numpad4
    0x64, //Numpad5
    0x65, //Numpad6
    0x66, //Numpad7
    0x67, //Numpad8
    0x68, //Numpad8
    0x69, //Numpad9
    0x6A,
    0x6B,
    0x6C,
    0x6D,
    0x6E,
    0x6F,
    0x70, //F1
    0x71, //F2
    0x72, //F3
    0x73, //F4
    0x74, //F5
    0x75, //F6
    0x76, //F7
    0x77, //F8
    0x78, //F9
    0x79, //F10
    0x7A, //F11
    0x7B, //F12
    0x7C, //F13
    0x7D, //F14
    0x7E, //F15
    0x7F, //F16
    0x80, //F17
    0x81, //F18
    0x82, //F19
    0x83, //F20
    0x84, //F21
    0x85, //F22
    0x86, //F23
    0x87, //F24
    0x88, //Unkown
    0x89, //Unkown
    0x8A, //Unkown
    0x8B, //Unkown
    0x8C, //Unkown
    0x8D, //Unkown
    0x8E, //Unkown
    0x8F, //Unkown
    0x90,
    0x91,
    0x92, //OEM Specific
    0x93, //OEM Specific
    0x94, //OEM Specific
    0x95, //OEM Specific
    0x96, //OEM Specific
    0x97, //Unkown
    0x98, //Unkown
    0x99, //Unkown
    0x9A, //Unkown
    0x9B, //Unkown
    0x9C, //Unkown
    0x9D, //Unkown
    0x9E, //Unkown 
    0x9F, //Unkown
    0xA0,
    0xA1,
    0xA2,
    0xA3,
    0xA4,
    0xA5
};

#include <map>

void Hotkey(int* k, const ImVec2& size_arg = ImVec2(0, 0), int id = 0)
{
    // Use a unique static variable for each hotkey instance
    static std::map<int, bool> waitingForKeyMap;

    // Initialize if not present
    if (waitingForKeyMap.find(id) == waitingForKeyMap.end()) {
        waitingForKeyMap[id] = false;
    }

    bool& waitingforkey = waitingForKeyMap[id];

    if (!waitingforkey) {
        std::string txt = std::string(KeyNames[*k]) + "##" + std::to_string(id);
        ImVec4 windowBgColor = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);

        // Darken the base color a bit for the button's border and background
        ImVec4 darker;
        darker.x = (windowBgColor.x - 0.04f < 0.0f) ? 0.0f : windowBgColor.x - 0.04f;
        darker.y = (windowBgColor.y - 0.04f < 0.0f) ? 0.0f : windowBgColor.y - 0.04f;
        darker.z = (windowBgColor.z - 0.04f < 0.0f) ? 0.0f : windowBgColor.z - 0.04f;
        darker.w = windowBgColor.w;

        // Using the original and darkened colors for button
        ImVec4 borderColor = darker;  // Using darker color for the border
        ImVec4 backgroundColor = windowBgColor;
        ImGui::PushStyleColor(ImGuiCol_Border, ImColor(12, 12, 12).Value);// Using the base color for the button's background
        if (ImGui::Button(txt.c_str(), size_arg))
        {
            waitingforkey = true;
        }
        ImGui::PopStyleColor();
    }
    else {
        std::string txt = "...##" + std::to_string(id);
        // Get base color from ImGui style (WindowBg)
        ImVec4 windowBgColor = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);

        // Darken the base color a bit for the button's border and background
        ImVec4 darker;
        darker.x = (windowBgColor.x - 0.04f < 0.0f) ? 0.0f : windowBgColor.x - 0.04f;
        darker.y = (windowBgColor.y - 0.04f < 0.0f) ? 0.0f : windowBgColor.y - 0.04f;
        darker.z = (windowBgColor.z - 0.04f < 0.0f) ? 0.0f : windowBgColor.z - 0.04f;
        darker.w = windowBgColor.w;

        // Using the original and darkened colors for button
        ImVec4 borderColor = darker;  // Using darker color for the border
        ImVec4 backgroundColor = windowBgColor;  // Using the base color for the button's background

        // Call to ColoredButtonV1 with adjusted colors
        ImGui::PushStyleColor(ImGuiCol_Border, ImColor(12, 12, 12).Value);
        ImGui::Button(txt.c_str(), size_arg);
        ImGui::PopStyleColor();

        // Check for key presses
        for (auto& Key : KeyCodes)
        {
            if (GetAsyncKeyState(Key) & 0x8000) {
                *k = Key;
                waitingforkey = false;
                break;
            }
        }
    }
}
namespace ImGui
{
    bool ColoredButtonV1(const char* label, const ImVec2& size, ImU32 text_color, ImU32 bg_color_1, ImU32 bg_color_2, bool border = false);
}
bool ImGui::ColoredButtonV1(const char* label, const ImVec2& size_arg, ImU32 text_color, ImU32 bg_color_1, ImU32 bg_color_2, bool border)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    ImGuiButtonFlags flags = ImGuiButtonFlags_None;
    // if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
     //    flags |= ImGuiButtonFlags_Repeat;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const bool is_gradient = bg_color_1 != bg_color_2;
    if (held || hovered)
    {
        // Modify colors (ultimately this can be prebaked in the style)
        float h_increase = (held && hovered) ? 0.02f : 0;
        float v_increase = (held && hovered) ? 0.20f : 0;

        ImVec4 bg1f = ColorConvertU32ToFloat4(bg_color_1);
        ColorConvertRGBtoHSV(bg1f.x, bg1f.y, bg1f.z, bg1f.x, bg1f.y, bg1f.z);
        bg1f.x = ImMin(bg1f.x + h_increase, 1.0f);
        bg1f.z = ImMin(bg1f.z + v_increase, 1.0f);
        ColorConvertHSVtoRGB(bg1f.x, bg1f.y, bg1f.z, bg1f.x, bg1f.y, bg1f.z);
        bg_color_1 = GetColorU32(bg1f);
        if (is_gradient)
        {
            ImVec4 bg2f = ColorConvertU32ToFloat4(bg_color_2);
            ColorConvertRGBtoHSV(bg2f.x, bg2f.y, bg2f.z, bg2f.x, bg2f.y, bg2f.z);
            bg2f.z = ImMin(bg2f.z + h_increase, 1.0f);
            bg2f.z = ImMin(bg2f.z + v_increase, 1.0f);
            ColorConvertHSVtoRGB(bg2f.x, bg2f.y, bg2f.z, bg2f.x, bg2f.y, bg2f.z);
            bg_color_2 = GetColorU32(bg2f);
        }
        else
        {
            bg_color_2 = bg_color_1;
        }
    }
    RenderNavHighlight(bb, id);

#if 0
    // V1 : faster but prevents rounding
    window->DrawList->AddRectFilledMultiColor(bb.Min, bb.Max, bg_color_1, bg_color_1, bg_color_2, bg_color_2);
    if (g.Style.FrameBorderSize > 0.0f)
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), 0.0f, 0, g.Style.FrameBorderSize);
#endif

    // V2
    int vert_start_idx = window->DrawList->VtxBuffer.Size;
    window->DrawList->AddRectFilled(bb.Min, bb.Max, bg_color_1, g.Style.FrameRounding);
    int vert_end_idx = window->DrawList->VtxBuffer.Size;
    if (is_gradient)
        ShadeVertsLinearColorGradientKeepAlpha(window->DrawList, vert_start_idx, vert_end_idx, bb.Min, bb.GetBL(), bg_color_1, bg_color_2);
    if (g.Style.FrameBorderSize > 0.0f)
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), g.Style.FrameRounding, 0, g.Style.FrameBorderSize);
    if (border)
    {
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_ScrollbarGrabHovered), g.Style.FrameRounding, 0, g.Style.FrameBorderSize);
        window->DrawList->AddRect(bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), GetColorU32(ImGuiCol_ScrollbarGrabActive), g.Style.FrameRounding, 0, g.Style.FrameBorderSize);
    }
    if (g.LogEnabled)
        LogSetNextTextDecoration("[", "]");
    PushStyleColor(ImGuiCol_Text, text_color);
    RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    PopStyleColor();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}
ImColor VecToColor(ImVec4 Color)
{
    return ImColor(
        (int)(Color.x * 255.0f),
        (int)(Color.y * 255.0f),
        (int)(Color.z * 255.0f),
        (int)(Color.w * 255.0f)
    );
}
ImVec4 ColorFromFloat(const float color[3], float alpha = 1.0f)
{
    return ImVec4(color[0], color[1], color[2], alpha);
}
ImColor FloatToColor(float* lol)
{
    return ImColor(lol[0], lol[1], lol[2]);
}
bool Keybind(CKeybind* keybind, const ImVec2& size_arg = ImVec2(0, 0), bool clicked = false, ImGuiButtonFlags flags = 0)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(keybind->get_name().c_str());
    const ImVec2 label_size = ImGui::CalcTextSize(keybind->get_name().c_str(), NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) &&
        style.FramePadding.y < window->DC.CurrLineTextBaseOffset)
    {
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    }
    ImVec2 size = ImVec2(30.0f, 14.0f);

    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;
    if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
    {
        ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
    }
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);
    bool value_changed = false;
    int key = keybind->key;
    auto io = ImGui::GetIO();
    std::string name = keybind->get_key_name();
    if (keybind->waiting_for_input)
        name = "";
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && hovered)
    {
        if (g.ActiveId == id)
        {
            keybind->waiting_for_input = true;
        }
    }
    else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && hovered)
    {
        ImGui::OpenPopup(keybind->get_name().c_str());
    }
    else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !hovered)
    {
        if (g.ActiveId == id)
            ImGui::ClearActiveID();
    }
    if (keybind->waiting_for_input)
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !hovered)
        {
            keybind->key = VK_LBUTTON;
            ImGui::ClearActiveID();
            keybind->waiting_for_input = false;
        }
        else
        {
            if (keybind->set_key())
            {
                ImGui::ClearActiveID();
                keybind->waiting_for_input = false;
            }
        }
    }
    //ImGui::PushFont(font::lexend_regular);

    ImVec4 textcolor = ImLerp(ImVec4(201 / 255.f, 204 / 255.f, 210 / 255.f, 1.f), ImVec4(1.0f, 1.0f, 1.0f, 1.f), 1.f);


    //window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(33 / 255.0f, 33 / 255.0f, 33 / 255.0f, 0.5f), 2.f);
    window->DrawList->AddRectFilled(
        bb.Min, bb.Max,
        ImColor(ImGui::GetStyleColorVec4(ImGuiCol_WindowBg))
    );
    window->DrawList->AddRect(
        bb.Min, bb.Max,
        ImColor(ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrabActive))
    );
    window->DrawList->AddRect(
        bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1,1),
        ImColor(ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrabHovered))
    );


    ImVec2 text_pos = bb.Min + ImVec2(
        size_arg.x / 2 - ImGui::CalcTextSize(name.c_str()).x / 2,
        size_arg.y / 2 - ImGui::CalcTextSize(name.c_str()).y / 2);

    window->DrawList->AddText(text_pos - ImVec2(0,1), ImGui::GetColorU32(textcolor), name.c_str());
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
    ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Border));
    
    if (ImGui::BeginPopup(keybind->get_name().c_str()))
    {
        //ImGui::BeginGroup();
       
       
        if (ImGui::Selectable("Hold", keybind->type == CKeybind::HOLD))
            keybind->type = CKeybind::HOLD;
        if (ImGui::Selectable("Toggle  ", keybind->type == CKeybind::TOGGLE))
            keybind->type = CKeybind::TOGGLE;
        //ImGui::EndGroup();
      
       
        ImGui::EndPopup();
    }
  
    ImGui::PopStyleColor(2);
    if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
    {
        ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, false);
    }
    return pressed;
}
struct DockButton
{
    bool Enabled;
    const char* Icon;
};
std::vector<std::string> ConfigListSaved = {  };
void RefreshConfigs()
{
    ConfigListSaved.clear();
}
void Overlay::Setup()
{
    RefreshConfigs();
    while (!Globals::RobloxWindow)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    Logging::Log(Logging::LogType::DEBUG, "Overlay Started https://discord.gg/XhVvn7McKj");
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    const HWND hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, wc.lpszClassName, (L"sdwdadsgf"),
        WS_POPUP, 0, 0, GetSystemMetrics(0), GetSystemMetrics(1), nullptr, nullptr, wc.hInstance, nullptr);
    const MARGINS margin = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margin);
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        exit(0);
    }
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    
    ImGuiStyle* style = &ImGui::GetStyle();
    bool done = false;
    bool draw = true;
    ImFontConfig font_config;
    font_config.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;
    font_config.PixelSnapH = true;
    font_config.SizePixels = 13.0f;
    font_config.RasterizerMultiply = 1.0f;
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\tahoma.ttf", 13.0f, &font_config);
    static float AccentColorF[3] = { 50.0f / 255.0, 94.0f / 255.0, 252.0f / 255.0 };
    static float WindowColorF[3] = { 24.0f / 255.0, 25.0f / 255.0, 28.0f / 255.0 };
    static float ChildColorF[3] = { 11.0f / 255.0, 11.0f / 255.0, 13.0f / 255.0 };
    static float OutlineF[3] = { 0.0f / 255.0, 0.0f / 255.0, 0.0f / 255.0 };
    static float TextTitleF[3] = { 107.0f / 255.0, 107.0f / 255.0, 108.0f / 255.0 };
    static float TextF[3] = { 156.0f / 255.0, 156.0f / 255.0, 157.0f / 255.0 };
    static float UnsafeTextF[3] = { 156.0f / 255.0, 191.0f / 255.0, 209.0f / 255.0 };
    static float InlineF[3] = { 39.0f / 255.0, 39.0f / 255.0, 39.0f / 255.0 };
    static bool MenuGlow = false;
    static bool Crosshair = false;
    static float MenuGlowFloat = 30.0f;
    ImColor AccentColor = FloatToColor(AccentColorF);
    ImColor WindowColor = FloatToColor(WindowColorF);
    ImColor ChildColor = FloatToColor(ChildColorF);
    ImColor Outline = FloatToColor(OutlineF);
    ImColor TextTitle = FloatToColor(TextTitleF);
    ImColor Text = FloatToColor(TextF);
    ImColor UnsafeText = FloatToColor(UnsafeTextF);
    ImColor Inline = FloatToColor(InlineF);
    {
        const char* clipboardText = "";
        std::string content;

        clipboardText = content.c_str();
        if (clipboardText && strlen(clipboardText) > 0)
        {
            try
            {
                
            }
            catch (const std::exception& e)
            {
                
            }
        }
    }
    ImFont* BoldFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdanab.ttf", 13.0f);
    ImFont* SmallestPixel = io.Fonts->AddFontFromMemoryTTF(
        SmallestPixelRaw,
        sizeof(SmallestPixelRaw),
        9.0f
    );
    ImFont* Visitor = io.Fonts->AddFontFromMemoryTTF(
        VisitorRaw,
        sizeof(VisitorRaw),
        9.0f
    );
    ImFont* LatoBold = io.Fonts->AddFontFromMemoryTTF(
        LatoBoldRaw,
        sizeof(LatoBoldRaw),
        15.0f
    );
    static const ImWchar icon_ranges[]{ ICON_MIN_FA, ICON_MAX_FA, 0 };

    ImFontConfig icon_config;

    icon_config.PixelSnapH = true;
    icon_config.OversampleH = 3;
    icon_config.OversampleV = 3;
    ImFont* iconsfont = io.Fonts->AddFontFromMemoryCompressedTTF(fa6_solid_compressed_data, fa6_solid_compressed_size, 13.0f, &icon_config, icon_ranges);
    auto LastThemeSave = std::chrono::steady_clock::now();

    DockButton Home;
    Home.Icon = ICON_FA_HOUSE;
    Home.Enabled = true;

    DockButton EspPreview;
    EspPreview.Icon = ICON_FA_PERSON;
    EspPreview.Enabled = false;

    DockButton Style;
    Style.Icon = ICON_FA_PALETTE;
    Style.Enabled = false;

    DockButton TargetHud;
    TargetHud.Icon = ICON_FA_CROSSHAIRS;
    TargetHud.Enabled = false;

    DockButton Explorer;
    Explorer.Icon = ICON_FA_LIST;
    Explorer.Enabled = false;

    Globals::g_pd3dDevice = Overlay::g_pd3dDevice;
    Globals::g_pd3dDeviceContext = Overlay::g_pd3dDeviceContext;

    

    std::vector<DockButton> DockButtons = { Home, Style, EspPreview, TargetHud, Explorer };

    while (!done)
    {
        {
            auto now = std::chrono::steady_clock::now();
        }
        MSG msg;

        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;
        g_SwapChainOccluded = false;
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        auto getbg = GetForegroundWindow();
        AccentColor = FloatToColor(AccentColorF);
        WindowColor = FloatToColor(WindowColorF);
        ChildColor = FloatToColor(ChildColorF);
        Outline = FloatToColor(OutlineF);
        TextTitle = FloatToColor(TextTitleF);
        Text = FloatToColor(TextF);
        UnsafeText = FloatToColor(UnsafeTextF);
        Inline = FloatToColor(InlineF);
        ImGuiStyle* style = &ImGui::GetStyle();
        style->ScrollbarSize = 0.0f;
        style->WindowPadding = { 6,6 };
        style->FramePadding = { 0,0 };
        style->Colors[ImGuiCol_ResizeGrip] = ImColor(0, 0, 0, 0).Value;
        style->Colors[ImGuiCol_ResizeGripActive] = ImColor(0, 0, 0, 0).Value;
        style->Colors[ImGuiCol_ResizeGripHovered] = ImColor(0, 0, 0, 0).Value;
        style->Colors[ImGuiCol_WindowBg] = WindowColor.Value;
        style->Colors[ImGuiCol_PopupBg] = WindowColor.Value;
        style->Colors[ImGuiCol_ChildBg] = ChildColor.Value;
        style->Colors[ImGuiCol_ScrollbarGrab] = ChildColor.Value;
        style->Colors[ImGuiCol_ScrollbarGrabActive] = Outline.Value;
        style->Colors[ImGuiCol_ScrollbarGrabHovered] = Inline.Value;
        style->Colors[ImGuiCol_Border] = Outline.Value;
        style->Colors[ImGuiCol_CheckMark] = AccentColor.Value;
        style->Colors[ImGuiCol_Text] = Text.Value;
        style->WindowShadowSize = 0.0f;
        style->WindowBorderSize = 1.0f;
        if (getbg == Globals::RobloxWindow || getbg == hwnd)
        {
            Overlay::move_window(hwnd);
            if (GetAsyncKeyState(VK_INSERT) & 1)
                draw = !draw;
            if (GetAsyncKeyState(VK_F1) & 1)
                draw = !draw;
            if (Globals::Esp::Font == 1)
                ImGui::PushFont(Visitor);
            if (Globals::Esp::Font == 2)
                ImGui::PushFont(SmallestPixel);
            if (Globals::Esp::Font == 3)
                ImGui::PushFont(BoldFont);
            if (Globals::Esp::Font == 4)
                ImGui::PushFont(LatoBold);
            try
            {
                
            }
            catch (...)
            {
                Logging::Log(Logging::LogType::ERR, "Caught crash at ESP");
            }

            if (Globals::Esp::Font != 0)
                ImGui::PopFont();
            
            if (draw || Globals::Aimbot::Target::HaveTarget)
            {
                if (DockButtons[3].Enabled)
                {
                    ImGui::SetNextWindowSize({ 332, 177 });
                    ImGui::Begin("TargetHud https://discord.gg/XhVvn7McKj", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
                    ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x, 25), ChildColor, ChildColor, WindowColor, WindowColor);

                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1), ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1), AccentColor);
                    if (MenuGlow)
                        ImGui::GetBackgroundDrawList()->AddShadowRect(ImGui::GetWindowPos() + ImVec2(1, 1), ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1), AccentColor, MenuGlowFloat, { 0,0 });
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 1);
                    ImGui::TextColored(AccentColor.Value, "Indicator https://discord.gg/XhVvn7McKj");
                    style->Colors[ImGuiCol_Border] = Inline.Value;
                    ImGui::BeginChild("##c", ImGui::GetContentRegionAvail(), true);
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                        ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                        Outline
                    );
                    style->Colors[ImGuiCol_ChildBg] = WindowColor.Value;
                    style->Colors[ImGuiCol_Border] = Outline.Value;
                    ImGui::BeginChild("##b", ImGui::GetContentRegionAvail(), true);
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                        ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                        Inline
                    );
                    style->Colors[ImGuiCol_ChildBg] = ChildColor;
                    style->Colors[ImGuiCol_Border] = Inline.Value;
                    ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                        ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                        Outline
                    );
                    ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                        ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                        AccentColor
                    );
                    ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                        ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                        ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                    );
                    ImGui::TextColored(TextTitle.Value, "Info");
                    style->Colors[ImGuiCol_ChildBg] = WindowColor.Value;
                    style->Colors[ImGuiCol_Border] = Outline.Value;
                    ImGui::BeginChild("##pfp", { ImGui::GetContentRegionAvail().y, ImGui::GetContentRegionAvail().y }, true);

                    ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y), WindowColor, WindowColor, ChildColor, ChildColor);
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                        ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                        Inline
                    );
                    style->Colors[ImGuiCol_ChildBg] = ChildColor;
                    style->Colors[ImGuiCol_Border] = Inline.Value;
                    ImGui::EndChild();
                    ImGui::SameLine();
                    ImGui::BeginChild("##Information", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, false);
                    if (Globals::Aimbot::Target::HaveTarget)
                    {
                        ImGui::Text("Name:");
                        ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(Globals::Aimbot::Target::Name.c_str()).x, 0.0f);
                        ImGui::Text(Globals::Aimbot::Target::Name.c_str());

                        ImGui::Text("Display Name:");
                        ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(Globals::Aimbot::Target::DisplayName.c_str()).x, 0.0f);
                        ImGui::Text(Globals::Aimbot::Target::DisplayName.c_str());
                    }
                    else
                    {
                        ImGui::Text("Name:");
                        ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("None").x, 0.0f);
                        ImGui::Text("None");
                        ImGui::Text("Display Name:");
                        ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("None").x, 0.0f);
                        ImGui::Text("None");
                        ImGui::Text("Tool:");
                        ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("None").x, 0.0f);
                        ImGui::Text("None");

                        ImGui::Text("Distance:");
                        ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("0m").x, 0.0f);
                        ImGui::Text("0m");

                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);

                        int health = 0;
                        ImGui::SliderInt("##hleath", &health, 0, 100, "%d", ImGuiSliderFlags_ReadOnly);
                    }
                    ImGui::EndChild();
                    ImGui::EndChild();
                    ImGui::EndChild();
                    ImGui::EndChild();
                    ImGui::End();
                }
            }
            if (draw)
            {
                if (DockButtons[4].Enabled)
                {
                    ImGui::SetNextWindowSizeConstraints({  300, 500 }, {FLT_MAX, FLT_MAX});
                    ImGui::Begin("Explorer https://discord.gg/XhVvn7McKj", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
                    ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x, 25), ChildColor, ChildColor, WindowColor, WindowColor);

                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1), ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1), AccentColor);
                    if (MenuGlow)
                        ImGui::GetBackgroundDrawList()->AddShadowRect(ImGui::GetWindowPos() + ImVec2(1, 1), ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1), AccentColor, MenuGlowFloat, { 0,0 });
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 1);
                    ImGui::TextColored(AccentColor.Value, "Explorer https://discord.gg/XhVvn7McKj");
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x);
                    ImGui::Text("x");
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
                    {
                        DockButtons[4].Enabled = false;
                    }
                    style->Colors[ImGuiCol_Border] = Inline.Value;
                    ImGui::BeginChild("##c", ImGui::GetContentRegionAvail(), true);
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                        ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                        Outline
                    );
                                 
                    ImGui::EndChild();
                    ImGui::End();
                }
                ImGui::Begin("##DockBar", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
                ImVec2 wp = ImGui::GetWindowPos();


                ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                    ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                    Inline
                );
                ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                    ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                    AccentColor
                );
                ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                    ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                    ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                );
                ImGui::PushFont(iconsfont);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
                for (DockButton& BT : DockButtons)
                {
                    bool ENABLED = BT.Enabled;
                    if (ENABLED)
                        ImGui::PushStyleColor(ImGuiCol_Text, AccentColor.Value);
                    if (ImGui::ColoredButtonV1(BT.Icon, { 25,25 }, ImGui::GetColorU32(ImGuiCol_Text), ChildColor, WindowColor, true))
                        BT.Enabled = !BT.Enabled;
                    if (ENABLED)
                        ImGui::PopStyleColor();
                    ImGui::SameLine();
                }

                ImGui::PopFont();

                ImGui::SetWindowPos({ io.DisplaySize.x / 2 - ImGui::GetWindowSize().x / 2, 20 });

                ImGui::End();
                
                if (DockButtons[0].Enabled)
                {
                    ImGui::SetNextWindowSizeConstraints({ 408, 433 }, { FLT_MAX, FLT_MAX });
                    ImGui::Begin("https://discord.gg/XhVvn7McKj", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
                    ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x, 25), ChildColor, ChildColor, WindowColor, WindowColor);

                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1), ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1), AccentColor);
                    if (MenuGlow)
                        ImGui::GetBackgroundDrawList()->AddShadowRect(ImGui::GetWindowPos() + ImVec2(1, 1), ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1), AccentColor, MenuGlowFloat, { 0,0 });
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 1);
                    ImGui::TextColored(AccentColor.Value, "join https://discord.gg/XhVvn7McKj for more sources");
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x);
                    ImGui::Text("x");
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
                    {
                        DockButtons[0].Enabled = false;
                    }
                    style->Colors[ImGuiCol_Border] = Inline.Value;
                    ImGui::BeginChild("##c", ImGui::GetContentRegionAvail(), true);
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                        ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                        Outline
                    );
                    style->Colors[ImGuiCol_ChildBg] = WindowColor.Value;
                    style->Colors[ImGuiCol_Border] = Outline.Value;
                    ImGui::BeginChild("##b", ImGui::GetContentRegionAvail(), true);
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                        ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                        Inline
                    );

                    std::string tabs[] = { "Aimbot", "Visuals", "World", "Misc", "Players", "Mines" };
                    static std::string currenttab = "Aimbot";
                    float totalWidth = ImGui::GetWindowSize().x;
                    size_t tabCount = sizeof(tabs) / sizeof(tabs[0]);
                    float spacing = 4.0f;
                    ImVec2 SelectedMin;
                    ImVec2 SelectedMax;
                    ImVec2 LastMin;
                    ImVec2 LastMax;
                    ImVec2 CurrentMin;
                    ImVec2 CurrentMax;


                    float buttonWidth = (totalWidth - spacing * (tabCount - 1)) / tabCount;

                    for (size_t i = 0; i < tabCount; ++i) {

                        if (i == 0) {
                            ImGui::SetCursorPos({ 2, 2 });
                        }
                        ImVec4 color = WindowColor;


                        ImVec4 darker;
                        darker.x = (color.x - 0.04f < 0.0f) ? 0.0f : color.x - 0.04f;
                        darker.y = (color.y - 0.04f < 0.0f) ? 0.0f : color.y - 0.04f;
                        darker.z = (color.z - 0.04f < 0.0f) ? 0.0f : color.z - 0.04f;
                        darker.w = color.w;

                        if (tabs[i] == currenttab) {

                            if (ImGui::ColoredButtonV1(tabs[i].c_str(), { buttonWidth - 1, 20 }, AccentColor, ImColor(darker), WindowColor)) {
                                currenttab = tabs[i];
                            }
                            SelectedMin = ImGui::GetItemRectMin();
                            SelectedMax = ImGui::GetItemRectMax();






                        }
                        else {
                            if (ImGui::ColoredButtonV1(tabs[i].c_str(), { buttonWidth - 1, 20 }, TextTitle, WindowColor, ChildColor)) {
                                currenttab = tabs[i];
                            }

                        }


                        CurrentMin = { std::floor(ImGui::GetItemRectMin().x), std::floor(ImGui::GetItemRectMin().y) };
                        CurrentMax = { std::floor(ImGui::GetItemRectMax().x), std::floor(ImGui::GetItemRectMax().y) };


                        if (i == 0) {
                            ImGui::GetForegroundDrawList()->AddLine(ImVec2(CurrentMax.x - 1, CurrentMin.y), CurrentMax - ImVec2(1, 0), Inline);
                            ImGui::GetForegroundDrawList()->AddLine(ImVec2(CurrentMax.x, CurrentMin.y - 1), CurrentMax, Outline);
                        }
                        if (i > 0) {



                            ImVec2 fillStart = ImVec2(LastMax.x, LastMin.y - 2);
                            ImVec2 fillEnd = ImVec2(CurrentMin.x, CurrentMax.y - 1);
                            ImGui::GetWindowDrawList()->AddRectFilled(fillStart, fillEnd, ChildColor);
                            ImGui::GetForegroundDrawList()->AddRectFilled(fillStart + ImVec2(1, 0), ImVec2(fillEnd.x, fillStart.y + 1), ChildColor);
                            ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(fillStart.x, fillEnd.y - 1), fillEnd, Outline);



                            ImGui::GetForegroundDrawList()->AddLine(CurrentMin, ImVec2(CurrentMin.x, CurrentMax.y), Inline);
                            ImGui::GetForegroundDrawList()->AddLine(CurrentMin - ImVec2(1, 2), ImVec2(CurrentMin.x - 1, CurrentMax.y), Outline);

                            if (i < tabCount - 1) {
                                ImGui::GetForegroundDrawList()->AddLine(ImVec2(CurrentMax.x - 1, CurrentMin.y), CurrentMax - ImVec2(1, 0), Inline);
                                ImGui::GetForegroundDrawList()->AddLine(ImVec2(CurrentMax.x - 1, CurrentMin.y), CurrentMax - ImVec2(1, 0), Inline);
                               
                            }
                            
                        }


                        LastMin = CurrentMin;
                        LastMax = CurrentMax;


                        if (i < tabCount - 1) {
                            ImGui::SameLine(0.00f, spacing);
                        }
                    }
                    wp = ImGui::GetWindowPos();

                    ImGui::GetForegroundDrawList()->AddLine(
                        { wp.x + 1, wp.y + 21 },
                        { SelectedMin.x, wp.y + 21 },
                        Inline
                    );
                    ImGui::GetForegroundDrawList()->AddLine(
                        { SelectedMax.x - 1, wp.y + 21 },
                        { wp.x + totalWidth - 1, wp.y + 21 },
                        Inline
                    );
                    style->Colors[ImGuiCol_ChildBg] = WindowColor;

                    if (currenttab == "Visuals")
                    {
                        ImGui::BeginChild("##1F", { ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y }, false);

                        style->Colors[ImGuiCol_ChildBg] = ChildColor;
                        style->Colors[ImGuiCol_Border] = Inline.Value;
                        ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Outline
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                            AccentColor
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                            ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                        );
                        ImGui::TextColored(TextTitle.Value, "Player ESP");
                        ImGui::Checkbox("Enabled", &Globals::Esp::Enabled);
                        ImGui::Checkbox("Box", &Globals::Esp::Box); ImGui::SameLine();
                        ImGui::ColorEdit4("##Box Color", Globals::Esp::Colors::Box, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                        ImGui::Checkbox("Box Glow", &Globals::Esp::BoxGlow); ImGui::SameLine();
                        ImGui::ColorEdit4("##Box Glow Color", Globals::Esp::Colors::BoxGlow, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                        ImGui::Checkbox("Name", &Globals::Esp::Name); ImGui::SameLine();
                        ImGui::ColorEdit4("##Name Color", Globals::Esp::Colors::Name, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                        ImGui::Checkbox("Distance", &Globals::Esp::Distance); ImGui::SameLine();
                        ImGui::ColorEdit4("##Distance Color", Globals::Esp::Colors::Distance, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                        ImGui::Checkbox("Tool", &Globals::Esp::Tool); ImGui::SameLine();
                        ImGui::ColorEdit4("##Tool Color", Globals::Esp::Colors::Tool, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                        ImGui::Checkbox("Chams", &Globals::Esp::Chams); ImGui::SameLine();
                        ImGui::ColorEdit4("##Chams Color", Globals::Esp::Colors::Chams, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                        ImGui::Checkbox("Chams Outlines", &Globals::Esp::ChamsOutlines); ImGui::SameLine();
                        ImGui::ColorEdit4("##Chams Outlines", Globals::Esp::Colors::ChamsOutlines, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                        ImGui::Checkbox("Chams Glow", &Globals::Esp::ChamsGlow); ImGui::SameLine();
                        ImGui::ColorEdit4("##Chams Glow", Globals::Esp::Colors::ChamsGlow, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                        ImGui::Checkbox("Health Bar", &Globals::Esp::HealthBar); ImGui::SameLine();
                        ImGui::ColorEdit4("##Health Bar", Globals::Esp::Colors::HealthBar, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                        ImGui::Checkbox("Health Bar Glow", &Globals::Esp::HealthBarGlow);
                        ImGui::Checkbox("Team", &Globals::Esp::Team);
                        ImGui::Checkbox("Leaderstats", &Globals::Esp::Leaderstats); ImGui::SameLine();
                        ImGui::ColorEdit4("##LeaderStats", Globals::Esp::Colors::LeaderStats, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                        //ImGui::Checkbox("Region", &Globals::Esp::Region); ImGui::SameLine();
                        //ImGui::ColorEdit4("##Region", Globals::Esp::Colors::Region, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                        ImGui::Checkbox("Tracers", &Globals::Esp::Tracers); ImGui::SameLine();
                        ImGui::ColorEdit4("##Tracers", Globals::Esp::Colors::Tracers, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                        ImGui::Checkbox("Skeleton", &Globals::Esp::Skeleton); ImGui::SameLine();
                        ImGui::ColorEdit4("##Skeleton", Globals::Esp::Colors::Skeleton, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                        ImGui::Checkbox("Avatar", &Globals::Esp::Avatar);
                        ImGui::Text("Avatar Size");
                        ImGui::SliderInt("##Avatar Size", &Globals::Esp::IconSize, 1, 50);
                        ImGui::Text("Avatar Offset");
                        ImGui::SliderInt("##Avatar Offset", &Globals::Esp::OffsetOffBoxAvatar, 1, 50);
                        ImGui::EndChild();
                        ImGui::EndChild();
                        ImGui::SameLine();
                        ImGui::BeginChild("##2F", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, false);

                        style->Colors[ImGuiCol_ChildBg] = ChildColor;
                        style->Colors[ImGuiCol_Border] = Inline.Value;
                        ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Outline
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                            AccentColor
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                            ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                        );
                        ImGui::TextColored(TextTitle.Value, "Visual Settings");

                        ImGui::Checkbox("Use display name", &Globals::Esp::UseDisplayName);
                        ImGui::Checkbox("Health Number", &Globals::Esp::HealthNumber); ImGui::SameLine();
                        ImGui::ColorEdit4("##Health Number", Globals::Esp::Colors::HealthNumber, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                        ImGui::Checkbox("Healthbar Background", &Globals::Esp::HealthBarBackGround);
                        {
                            static const char* Options[] = { "Static", "Health-Based" };
                            ImGui::Text("Health bar color Type");
                            ImGui::Combo("Health bar color type", &Globals::Esp::HealthBarColorType, Options, IM_ARRAYSIZE(Options));
                        }



                        {
                            static const char* Options[] = { "Default","Visitor", "Smallest Pixel Font", "Bold verdana", "Lato Bold" };
                            ImGui::Text("Font");
                            ImGui::Combo("Font", &Globals::Esp::Font, Options, IM_ARRAYSIZE(Options));
                        }
                        ImGui::Checkbox("Local ESP", &Globals::Esp::LocalPlayerESP);

                        ImGui::Checkbox("Max Distance", &Globals::Esp::MaxDistance);
                        ImGui::SliderInt("##MaxDistanceV", &Globals::Esp::MaxDistanceValue, 10, 30000);
                        ImGui::Checkbox("Keybind List", &Globals::Esp::KeybindList);
                        ImGui::Text("Chams Outline Thickness");
                        ImGui::SliderInt("##ChamsThickness", &Globals::Esp::ChamsOutlineThickness, 1, 5);
                        ImGui::Checkbox("Inherit Body Colors Chams", &Globals::Esp::InheritBodyColorChams);
                        ImGui::Checkbox("Filter Unknown Cham Parts", &Globals::Esp::FilterUnknownChamsParts);
                        {
                            static const char* Options[] = { "Convex","Highlight" };
                            ImGui::Text("Chams type");
                            ImGui::Combo("Chams type", &Globals::Esp::ChamsType, Options, IM_ARRAYSIZE(Options));
                        }
                        {
                            static const char* Options[] = { "Cursor","Bottom", "Top"};
                            ImGui::Text("Tracer Postion");
                            ImGui::Combo("Tracer Postion", &Globals::Esp::TracersPosition, Options, IM_ARRAYSIZE(Options));
                        }
                        ImGui::EndChild();
                        ImGui::EndChild();
                    }
                    if (currenttab == "Aimbot")
                    {

                        ImGui::BeginChild("##1F", { ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y }, false);

                        style->Colors[ImGuiCol_ChildBg] = ChildColor;
                        style->Colors[ImGuiCol_Border] = Inline.Value;
                        ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 1.25f }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Outline
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                            AccentColor
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                            ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                        );
                        ImGui::TextColored(TextTitle.Value, "Aimbot");
                        ImGui::Checkbox("Enabled", &Globals::Aimbot::Enabled);
                        ImGui::SameLine(ImGui::GetWindowSize().x - 36);
                        Keybind(&Globals::Aimbot::AimbotKey, { 30,14 });
                        {
                            static const char* Options[] = { "Mouse","Memory", "Silent" };
                            ImGui::Text("Aim Type");
                            ImGui::Combo("Aim Type", &Globals::Aimbot::AimType, Options, IM_ARRAYSIZE(Options));
                        }
                        {
                            static const char* Options[] = { "Head", "HumanoidRootPart" };
                            ImGui::Text("Hitbox");
                            ImGui::Combo("Hitbox", &Globals::Aimbot::Hitbox, Options, IM_ARRAYSIZE(Options));
                        }
                        ImGui::Checkbox("Prediction", &Globals::Aimbot::Prediction);
                        ImGui::Text("Prediction X");
                        ImGui::SliderFloat("##X", &Globals::Aimbot::PredictionX, 1.0f, 15.0f);
                        ImGui::Text("Prediction Y");
                        ImGui::SliderFloat("##Y", &Globals::Aimbot::PredictionY, 1.0f, 15.0f);

                        ImGui::Checkbox("Smoothning", &Globals::Aimbot::Smoothning);
                        ImGui::SliderFloat("##SmoothningV", &Globals::Aimbot::SmoothingValue, 0.1f, 1.0f);
                        ImGui::EndChild();
                        ImGui::BeginChild("##2", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y  }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Outline
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                            AccentColor
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                            ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                        );
                        ImGui::TextColored(TextTitle.Value, "Rage");
                        ImGui::Checkbox("Target Spam TP", &Globals::Aimbot::Rage::SpamTP);
                        ImGui::Checkbox("Target Orbit", &Globals::Aimbot::Rage::Orbit);
                        ImGui::EndChild();
                        ImGui::EndChild();
                        ImGui::SameLine();
                        style->Colors[ImGuiCol_ChildBg] = WindowColor;
                        ImGui::BeginChild("##2F", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, false);

                        style->Colors[ImGuiCol_ChildBg] = ChildColor;
                        style->Colors[ImGuiCol_Border] = Inline.Value;
                        ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 2 }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Outline
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                            AccentColor
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                            ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                        );
                        ImGui::TextColored(TextTitle.Value, "Settings");
                        ImGui::Checkbox("Enable FOV", &Globals::Aimbot::EnableFOV);
                        ImGui::SliderInt("##FovValue", &Globals::Aimbot::FOV, 10, 1000);
                        ImGui::Checkbox("Draw FOV", &Globals::Aimbot::DrawFOV); ImGui::SameLine();
                        ImGui::ColorEdit3("##DrawFOV", Globals::Aimbot::FovColor, ImGuiColorEditFlags_NoInputs);

                        ImGui::Checkbox("Fill FOV", &Globals::Aimbot::FovSettings::FillFov); ImGui::SameLine();
                        ImGui::ColorEdit4("##FillFOV", Globals::Aimbot::FovSettings::FillColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                       



                        ImGui::Checkbox("HitSounds", &Globals::Aimbot::Hits::HitSounds);
                        if (Globals::Aimbot::Hits::HitSounds)
                        {
                            static const char* Options[] = { ("amongus"), ("skeet"),("beep"), ("bonk"), ("bubble"), ("cod"), ("csgo"), ("fairy"), ("fatality"), ("osu"), ("rust"), ("bameware") };
                            ImGui::Combo("HitSound", &Globals::Aimbot::Hits::HitSound, Options, IM_ARRAYSIZE(Options));
                        }
                        ImGui::Checkbox("Hit tracers", &Globals::Aimbot::Hits::HitTracers); ImGui::SameLine();
                        ImGui::ColorEdit3("##Hit tracers", Globals::Aimbot::Hits::HitTracersColor, ImGuiColorEditFlags_NoInputs);
                        ImGui::Checkbox("Hit Markers", &Globals::Aimbot::Hits::HitMarkers); ImGui::SameLine();
                        ImGui::ColorEdit3("##Hit Markers", Globals::Aimbot::Hits::HitMarkersColor, ImGuiColorEditFlags_NoInputs);
                        if (Crosshair && Globals::Aimbot::AimType == 2)
                        {
                            ImGui::Checkbox("Silent Crosshair", &Globals::Aimbot::SilentCrosshair);
                            if (Globals::Aimbot::SilentCrosshair)
                            {
                                ImGui::Checkbox("Silent Crosshair Tracer", &Globals::Aimbot::SilentCrosshairTracer); ImGui::SameLine();
                                ImGui::ColorEdit3("##Silent Tracer Color", Globals::Aimbot::SilentCrosshairTracerColor, ImGuiColorEditFlags_NoInputs);
                            }

                        }
                        ImGui::EndChild();
                        ImGui::BeginChild("##2", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Outline
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                            AccentColor
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                            ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                        );
                        ImGui::TextColored(TextTitle.Value, "Triggerbot");
                        ImGui::Checkbox("Enabled", &Globals::TriggerBot::Enabled);
                        ImGui::SameLine(ImGui::GetWindowSize().x - 36);
                        Keybind(&Globals::TriggerBot::Key, { 30,14 });
                        ImGui::Text("Delay");
                        ImGui::SliderInt("##D", &Globals::TriggerBot::Delay, 0, 1000);
                        ImGui::Text("Threshold");
                        ImGui::SliderInt("##T", &Globals::TriggerBot::Threshold, 0, 1000);
                        ImGui::Checkbox("Tool Check", &Globals::TriggerBot::ToolCheck);
                        ImGui::EndChild();
                        ImGui::EndChild();
                    }
                    if (currenttab == "World")
                    {
                        ImGui::BeginChild("##1F", { ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y }, false);

                        style->Colors[ImGuiCol_ChildBg] = ChildColor;
                        style->Colors[ImGuiCol_Border] = Inline.Value;
                        ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 2 }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Outline
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                            AccentColor
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                            ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                        );
                        ImGui::TextColored(TextTitle.Value, "Misc");

                        ImGui::Checkbox("Fly Hack", &Globals::World::Movement::Fly);
                        ImGui::SameLine(ImGui::GetWindowSize().x - 36);
                        Keybind(&Globals::World::Movement::FlyKey, { 30,14 });
                        ImGui::SliderFloat("##FlySpeed", &Globals::World::Movement::FlySpeed, 1.0f, 100.0f);

                        ImGui::Checkbox("CFrame Speed", &Globals::World::Movement::Cframe);
                        ImGui::SameLine(ImGui::GetWindowSize().x - 36);
                        Keybind(&Globals::World::Movement::CframeKey, { 30,14 });
                        ImGui::SliderFloat("##CframeSpeed", &Globals::World::Movement::CframeSpeed, 1.0f, 100.0f);

                        ImGui::Checkbox("Freecam##1", &Globals::World::Movement::Freecam);
                        ImGui::SameLine(ImGui::GetWindowSize().x - 36);
                        Keybind(&Globals::World::Movement::FreecamKey, { 30,14 });
                        ImGui::SliderFloat("##FreecamSpeed", &Globals::World::Movement::FreecamSpeed, 0.001f, 0.5f);

                        ImGui::Checkbox("Third Person##1", &Globals::World::Movement::ThirdPerson);
                        ImGui::SameLine(ImGui::GetWindowSize().x - 36);
                        Keybind(&Globals::World::Movement::ThirdPersonKey, { 30,14 });
                        ImGui::SliderFloat("##ThirdPersonDistance", &Globals::World::Movement::ThirdPersonDistance, 1.0f, 15.0f);


                        if (ImGui::Button("Force reset", { ImGui::GetContentRegionAvail().x, 17 }))
                        {
                            
                        }
                        ImGui::Checkbox("Desync##1", &Globals::Misc::Desync);
                        ImGui::SameLine(ImGui::GetWindowSize().x - 36);
                        Keybind(&Globals::Misc::DesyncBind, { 30,14 });
                        /*
                        if (ImGui::Checkbox("Client forcefield material", &Globals::World::LocalPlayerForceField))
                        {
                            if (Globals::World::LocalPlayerForceField)
                            {
                                if (SDK::Cache::LocalPlayer.PlayerObjectAddress)
                                {
                                    for (SDK::Cache::Bone Bone : SDK::Cache::LocalPlayer.Bones)
                                    {
                                        Bone.Object.Primitive().SetMaterial(1584);
                                    }
                                }
                            }
                            else
                            {
                                if (SDK::Cache::LocalPlayer.PlayerObjectAddress)
                                {
                                    for (SDK::Cache::Bone Bone : SDK::Cache::LocalPlayer.Bones)
                                    {
                                        Bone.Object.Primitive().SetMaterial(256);
                                    }
                                }
                            }
                        }
                        
                        ImGui::SameLine(0.0f, 0.0f);
                        ImGui::TextColored(AccentColor.Value, " (?)");
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Sets your LocalPlayers model material to forcefield. This is Client-Sided. You need to zoom in and out to activate it.");
                            */
                        ImGui::EndChild();
                        ImGui::BeginChild("##2", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Outline
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                            AccentColor
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                            ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                        );
                        ImGui::TextColored(TextTitle.Value, "Game specific");
                        {
                            static bool TimeChanger = false;
                            ImGui::Checkbox("Time Changer", &TimeChanger);

                            static int Time = 0;
                            if (ImGui::SliderInt("##Time", &Time, 0, 12))
                            {

                            }
                        }
                        ImGui::Checkbox("Infinite Jump", &Globals::World::Movement::InfiniteJump);
                        ImGui::Checkbox("Custom infinite jump power", &Globals::World::Movement::InfiniteJumpNameSpace::CustomJumpPower);
                        ImGui::SliderFloat("##CustomJumpPower", &Globals::World::Movement::InfiniteJumpNameSpace::CustomJumpPowerValue, 1.0f, 300.0f);

                        ImGui::Checkbox("Spinbot", &Globals::World::Movement::Spinbot);
                        ImGui::SliderInt("##SpinBotSpeed", &Globals::World::Movement::SpinbotSpeed, 1, 100);
                        if (Globals::World::Movement::Spinbot)
                            ImGui::Checkbox("Upside down", &Globals::World::Movement::UpsideDown);
                        ImGui::Checkbox("Fov Changer", &Globals::World::CameraFOV);
                        if (ImGui::SliderInt("##CameraFOV", &Globals::World::CameraFOVValue, 10, 135))
                        {
                            if (Globals::World::CameraFOV)
                            {
                                
                            }
                        }

                        ImGui::Checkbox("Zoom##1", &Globals::World::Zoom);
                        ImGui::SameLine(ImGui::GetWindowSize().x - 36);
                        Keybind(&Globals::World::ZoomKey, { 30,14 });
                        ImGui::SliderInt("##ZoomValue", &Globals::World::ZoomValue, 5, 135);

                        ImGui::EndChild();
                        ImGui::EndChild();
                        ImGui::SameLine();
                        style->Colors[ImGuiCol_ChildBg] = WindowColor;
                        ImGui::BeginChild("##2F", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, false);

                        style->Colors[ImGuiCol_ChildBg] = ChildColor;
                        style->Colors[ImGuiCol_Border] = Inline.Value;
                        ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 1.25f }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Outline
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                            AccentColor
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                            ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                        );
                        ImGui::TextColored(TextTitle.Value, "Waypoints");

                        style->Colors[ImGuiCol_ChildBg] = WindowColor;
                        style->Colors[ImGuiCol_Border] = Outline;
                        ImGui::BeginChild("##Waypoints", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 126 }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Inline
                        );
                        style->Colors[ImGuiCol_ChildBg] = ChildColor;
                        style->Colors[ImGuiCol_Border] = Inline;
                        ImGui::EndChild();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        static char WaypointName[158] = "";
                        ImGui::InputText("##WaypointName", WaypointName, IM_ARRAYSIZE(WaypointName));
                        if (ImGui::Button("Create", { ImGui::GetContentRegionAvail().x, 17 }))
                        {
                            bool AlreadyExists = false;
                        }
                        if (ImGui::Button("Teleport", { ImGui::GetContentRegionAvail().x, 17 }))
                        {
                        }
                        if (ImGui::Button("Delete", { ImGui::GetContentRegionAvail().x, 17 }))
                        {

                        }

                        ImGui::Checkbox("Name", &Globals::World::Waypoints::WaypointName); ImGui::SameLine();
                        ImGui::ColorEdit4("##Name Color", Globals::World::Waypoints::Colors::WaypointName, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                        ImGui::Checkbox("Distance", &Globals::World::Waypoints::WaypointDistance); ImGui::SameLine();
                        ImGui::ColorEdit4("##Distance Color", Globals::World::Waypoints::Colors::WaypointDistance, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                        ImGui::EndChild();
                        ImGui::BeginChild("##2", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Outline
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                            AccentColor
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                            ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                        );
                        ImGui::TextColored(TextTitle.Value, "Lighting");

                        ImGui::Text("Ambient"); ImGui::SameLine();
                        if (ImGui::ColorEdit3("##Ambient Color", Globals::World::AmbientColor, ImGuiColorEditFlags_NoInputs))
                        {

                        }

                        ImGui::Text("Outdoor ambient"); ImGui::SameLine();
                        if (ImGui::ColorEdit3("##Outdoor ambient Color", Globals::World::OutdoorAmbientColor, ImGuiColorEditFlags_NoInputs))
                        {

                        }

                        ImGui::EndChild();
                        ImGui::EndChild();
                    }
                    if (currenttab == "Misc")
                    {
                        ImGui::BeginChild("##1F", { ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y / 1.5f }, false);

                        style->Colors[ImGuiCol_ChildBg] = ChildColor;
                        style->Colors[ImGuiCol_Border] = Inline.Value;
                        ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Outline
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                            AccentColor
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                            ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                        );
                        ImGui::TextColored(TextTitle.Value, "Configs");
                        style->Colors[ImGuiCol_ChildBg] = WindowColor;
                        style->Colors[ImGuiCol_Border] = Outline;
                        ImGui::BeginChild("##Waypoints", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 105 }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Inline
                        );
                        static std::string SelectedConfig;
                        static char ConfigName[158] = "";

                        style->Colors[ImGuiCol_ChildBg] = ChildColor;
                        style->Colors[ImGuiCol_Border] = Inline;

                        for (const std::string& Config : ConfigListSaved)
                        {
                            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(Config.c_str()).x) / 2);

                            if (SelectedConfig == Config)
                                ImGui::TextColored(AccentColor.Value, Config.c_str());
                            else
                                ImGui::Text(Config.c_str());

                            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                                SelectedConfig = Config;
                                strncpy_s(ConfigName, Config.c_str(), IM_ARRAYSIZE(ConfigName));
                            }

                            ImGui::Separator();
                        }

                        ImGui::EndChild();

                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        ImGui::InputText("##SelectedConfig", ConfigName, IM_ARRAYSIZE(ConfigName));

                        std::string SelectedconfigPath = "C:\\Athena\\Configs\\" + SelectedConfig + ".rocks";
                        std::string configPath = "C:\\Athena\\Configs\\" + std::string(ConfigName) + ".rocks";

                        ImGui::EndChild();

                        ImGui::EndChild();
                        ImGui::SameLine();
                        ImGui::BeginChild("##2F", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, false);

                        style->Colors[ImGuiCol_ChildBg] = ChildColor;
                        style->Colors[ImGuiCol_Border] = Inline.Value;
                        ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Outline
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                            AccentColor
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                            ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                        );
                        ImGui::TextColored(TextTitle.Value, "Cheat Settings");
                        ImGui::Checkbox("Teamcheck", &Globals::Misc::TeamCheck);
                        ImGui::Checkbox("Vsync", &Globals::Vsync);
                        ImGui::Checkbox("Streamproof", &Globals::Misc::StreamProof);
                        ImGui::Checkbox("Watermark", &Globals::Watermark);
                        ImGui::EndChild();
                        ImGui::EndChild();
                    }
                    if (currenttab == "Players")
                    {
                        style->Colors[ImGuiCol_ChildBg] = ChildColor;
                        style->Colors[ImGuiCol_Border] = Inline.Value;
                        ImGui::BeginChild("##1", ImGui::GetContentRegionAvail(), true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Outline
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                            AccentColor
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                            ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                        );
                        ImGui::TextColored(TextTitle.Value, "Players");
                        style->Colors[ImGuiCol_ChildBg] = WindowColor;
                        style->Colors[ImGuiCol_Border] = Outline;
                        ImGui::BeginChild("##Players", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 70 }, true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Inline
                        );
                        style->Colors[ImGuiCol_ChildBg] = ChildColor;
                        style->Colors[ImGuiCol_Border] = Inline;

                        ImGui::EndChild();
                        static float ButtonHeight = ImGui::GetContentRegionAvail().y / 3 - 3;
                        style->Colors[ImGuiCol_ChildBg] = WindowColor.Value;
                        style->Colors[ImGuiCol_Border] = Outline.Value;
                        ImGui::BeginChild("##pfp", { ImGui::GetContentRegionAvail().y, ImGui::GetContentRegionAvail().y }, true);

                        ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y), WindowColor, WindowColor, ChildColor, ChildColor);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Inline
                        );
                        
                        style->Colors[ImGuiCol_ChildBg] = ChildColor;
                        style->Colors[ImGuiCol_Border] = Inline.Value;
                        ImGui::EndChild();
                        ImGui::SameLine();
                        ImGui::BeginChild("AthenamationOnPlayer", ImGui::GetContentRegionAvail(), false);
                            
                        if (ImGui::Button("Teleport", { ImGui::GetContentRegionAvail().x, ButtonHeight }))
                        {

                        }
                        if (ImGui::Button("Spectate", { ImGui::GetContentRegionAvail().x, ButtonHeight }))
                        {
                            
                        }
                        if (ImGui::Button("Unspectate", { ImGui::GetContentRegionAvail().x, ButtonHeight }))
                        {

                        }
                        ImGui::EndChild();
                        ImGui::EndChild();
                    }
                    if (currenttab == "Mines")
                    {
                        style->Colors[ImGuiCol_ChildBg] = ChildColor;
                        style->Colors[ImGuiCol_Border] = Inline.Value;
                        ImGui::BeginChild("##1", ImGui::GetContentRegionAvail(), true);
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                            ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                            Outline
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                            AccentColor
                        );
                        ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                            ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                            ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                        );
                        ImGui::TextColored(TextTitle.Value, "Mines");

                        static bool gameStarted = false;
                        static bool gameOver = false;
                        static int mineX = -1, mineY = -1;
                        static const int gridSize = 5;
                        static bool revealed[gridSize][gridSize] = {};
                        ImVec2 avail = { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 30 };
                        float spacingX = ImGui::GetStyle().ItemSpacing.x;
                        float spacingY = ImGui::GetStyle().ItemSpacing.y;
                        float totalSpacingX = spacingX * (gridSize - 1);
                        float totalSpacingY = spacingY * (gridSize - 1);
                        ImVec2 buttonSize{
                            (avail.x - totalSpacingX) / gridSize,
                            (avail.y - totalSpacingY) / gridSize
                        };

                        for (int y = 0; y < gridSize; ++y) {
                            for (int x = 0; x < gridSize; ++x) {
                                char label[32];
                                snprintf(label, sizeof(label), "##btn%d%d", x, y);

                                if (revealed[x][y]) {
                                    char revealedLabel[64];
                                    snprintf(revealedLabel, sizeof(revealedLabel), "%s##revealed_%d_%d_%d", (x == mineX && y == mineY) ? "Mine" : "Diamond", x, y, std::rand());
                                    if (x == mineX && y == mineY)
                                    {
                                        ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 0, 0).Value);
                                    }
                                    else
                                    {
                                        ImGui::PushStyleColor(ImGuiCol_Text, ImColor(37, 150, 190).Value);
                                    }
                                    ImGui::Button(revealedLabel, buttonSize);

                                    ImGui::PopStyleColor();

                                }
                                else {
                                    if (ImGui::Button(label, buttonSize) && gameStarted && !gameOver) {
                                        revealed[x][y] = true;
                                    }
                                }

                                if (x < gridSize - 1)
                                    ImGui::SameLine();
                            }
                        }

                        if (ImGui::Button("Start Game", ImGui::GetContentRegionAvail())) {
                            std::srand(static_cast<unsigned>(std::time(0)));
                            mineX = std::rand() % gridSize;
                            mineY = std::rand() % gridSize;
                            gameStarted = true;
                            gameOver = false;
                            for (int y = 0; y < gridSize; ++y)
                                for (int x = 0; x < gridSize; ++x)
                                    revealed[x][y] = false;
                        }
                        ImGui::EndChild();
                    }



                    ImGui::EndChild();
                    ImGui::EndChild();
                    ImGui::End();
                }
                if (DockButtons[2].Enabled)
                {


                    ImGui::SetNextWindowSize({ 322,391 });
                    ImGui::Begin("Preview https://discord.gg/XhVvn7McKj", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar
                        | ImGuiWindowFlags_NoResize);
                    ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x, 25), ChildColor, ChildColor, WindowColor, WindowColor);
                    //ImGui::GetBackgroundDrawList()->AddShadowRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), ThemeColor, 30, { 0,0 });
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1), ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1), AccentColor);
                    if (MenuGlow)
                        ImGui::GetBackgroundDrawList()->AddShadowRect(ImGui::GetWindowPos() + ImVec2(1, 1), ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1), AccentColor, MenuGlowFloat, { 0,0 });
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 1);
                    ImGui::TextColored(AccentColor.Value, "Preview https://discord.gg/XhVvn7McKj");
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x);
                    ImGui::Text("x");
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
                    {
                        DockButtons[2].Enabled = false;
                    }
                    style->Colors[ImGuiCol_Border] = Inline.Value;
                    ImGui::BeginChild("##c", ImGui::GetContentRegionAvail(), true);
                    ImVec2 p = ImGui::GetWindowPos();
                    ImVec2 s = ImGui::GetWindowSize();
                    ImVec2 i = ImVec2(254, 356);
                    ImVec2 pos = p + (s - i) * 0.5f - ImVec2(15, 10);
                    pos = ImVec2(std::floor(pos.x), std::floor(pos.y));
                    ImGui::GetWindowDrawList()->AddRect(p + ImVec2(1, 1), p + s - ImVec2(1, 1), Outline);

                    ImVec2 BoxMin = pos + ImVec2(80, 28);
                    ImVec2 BoxMax = pos + i - ImVec2(50, 17);
                    if (Globals::Esp::Box)
                    {
                        ImGui::GetWindowDrawList()->AddRect(
                            BoxMin + ImVec2(1, 1),
                            BoxMax - ImVec2(1, 1),
                            ImColor(0, 0, 0)
                        );
                        ImGui::GetWindowDrawList()->AddRect(
                            BoxMin - ImVec2(1, 1),
                            BoxMax + ImVec2(1, 1),
                            ImColor(0, 0, 0)
                        );
                        ImGui::GetWindowDrawList()->AddRect(
                            BoxMin,
                            BoxMax,
                            ImColor(Globals::Esp::Colors::Box[0], Globals::Esp::Colors::Box[1], Globals::Esp::Colors::Box[2])
                        );
                    }
                    if (Globals::Esp::BoxGlow)
                    {
                        ImGui::GetWindowDrawList()->AddShadowRect(
                            BoxMin + ImVec2(2, 2),
                            BoxMax - ImVec2(2, 2),
                            ImColor(Globals::Esp::Colors::BoxGlow[0], Globals::Esp::Colors::BoxGlow[1], Globals::Esp::Colors::BoxGlow[2]),
                            50.0f,
                            { 0,0 }
                        );
                    }
                    if (Globals::Esp::Font == 1)
                        ImGui::PushFont(Visitor);
                    if (Globals::Esp::Font == 2)
                        ImGui::PushFont(SmallestPixel);
                    if (Globals::Esp::Font == 3)
                        ImGui::PushFont(BoldFont);
                    if (Globals::Esp::Font == 4)
                        ImGui::PushFont(LatoBold);
                    if (Globals::Esp::Name)
                    {
                        ImVec2 Size = ImGui::CalcTextSize("Bladee");
                        ImVec2 NamePosition = ImVec2(BoxMin.x + (BoxMax.x - BoxMin.x) / 2 - Size.x / 2, BoxMin.y - 1 - Size.y);
                        ImGui::GetWindowDrawList()->AddText(NamePosition, ImColor(Globals::Esp::Colors::Name[0], Globals::Esp::Colors::Name[1], Globals::Esp::Colors::Name[2]), "Bladee");
                    }
                    if (Globals::Esp::Tool)
                    {
                        ImVec2 Size = ImGui::CalcTextSize("Drain gun");
                        ImVec2 TextPosition = ImVec2(BoxMin.x + (BoxMax.x - BoxMin.x) / 2 - Size.x / 2, BoxMax.y + 1);
                        ImGui::GetWindowDrawList()->AddText(TextPosition, ImColor(Globals::Esp::Colors::Tool[0], Globals::Esp::Colors::Tool[1], Globals::Esp::Colors::Tool[2]), "Drain gun");
                    }
                    if (Globals::Esp::Distance)
                    {
                        std::string DistanceText = "100m";
                        ImVec2 Size = ImGui::CalcTextSize(DistanceText.c_str());
                        ImVec2 TextPosition = ImVec2(BoxMin.x + (BoxMax.x - BoxMin.x) / 2 - Size.x / 2, BoxMax.y + 1);

                        if (Globals::Esp::Tool) {
                            TextPosition = ImVec2(BoxMin.x + (BoxMax.x - BoxMin.x) / 2 - Size.x / 2, BoxMax.y + ImGui::CalcTextSize("Drain gun").y + 2);
                        }

                        ImGui::GetWindowDrawList()->AddText(TextPosition, ImColor(Globals::Esp::Colors::Distance[0], Globals::Esp::Colors::Distance[1], Globals::Esp::Colors::Distance[2]), DistanceText.c_str());
                    }
                    int ESPPREVIEWOFFSET = 0;
                    if (Globals::Esp::Team)
                    {



                        ImGui::GetWindowDrawList()->AddText(ImVec2(BoxMax.x + 3, BoxMin.y + ESPPREVIEWOFFSET), ImColor(255, 255, 255), "DrainGang");
                        ESPPREVIEWOFFSET += ImGui::CalcTextSize("DrainGang").y + 2;

                    }
                    if (Globals::Esp::Leaderstats)
                    {



                        ImGui::GetWindowDrawList()->AddText(ImVec2(BoxMax.x + 3, BoxMin.y + ESPPREVIEWOFFSET), ImColor(Globals::Esp::Colors::LeaderStats[0], Globals::Esp::Colors::LeaderStats[1], Globals::Esp::Colors::LeaderStats[2]), "Drained: 100");
                        ESPPREVIEWOFFSET += ImGui::CalcTextSize("Drained: 100").y + 2;

                    }
                    if (Globals::Esp::HealthBar) {
                        float rawHealth = 50;
                        float playerMaxHealth = 100;


                        double playerHealth = 50;
                        double healthPercentage = (playerMaxHealth > 0.0) ? (playerHealth / playerMaxHealth) : 0.0;
                        float healthBarHeight = BoxMax.y - BoxMin.y;
                        float filledHealthBarHeight = healthBarHeight * healthPercentage;
                        float emptyBarHeight = healthBarHeight - filledHealthBarHeight;
                        float filledHealthMaxY = BoxMin.y + emptyBarHeight;
                        ImVec2 health_min = ImVec2(std::floor(BoxMin.x - 5.0f), std::floor(filledHealthMaxY));
                        ImVec2 health_max = ImVec2(std::floor(BoxMin.x - 3.0f), std::floor(BoxMax.y));

                        if (Globals::Esp::HealthBarBackGround) {
                            ImGui::GetWindowDrawList()->AddRectFilled({ health_min.x, BoxMin.y }, { health_max.x, health_max.y }, ImColor(0, 0, 0, 187));
                        }

                        ImColor Color = ImColor(Globals::Esp::Colors::HealthBar[0], Globals::Esp::Colors::HealthBar[1], Globals::Esp::Colors::HealthBar[2]);
                        if (Globals::Esp::HealthBarColorType == 1) {
                            Color = ImColor(94, 235, 52);
                            if (healthPercentage < 0.2f) Color = ImColor(235, 52, 52);
                            else if (healthPercentage < 0.4f) Color = ImColor(235, 168, 52);
                            else if (healthPercentage < 0.7f) Color = ImColor(192, 235, 52);
                        }

                        ImGui::GetWindowDrawList()->AddRectFilled(health_min, health_max, Color);

                        if (Globals::Esp::HealthBarGlow) {
                            ImGui::GetWindowDrawList()->AddShadowRect(health_min, health_max, Color, 50.0f, { 0,0 });
                        }


                        if (Globals::Esp::HealthBarBackGround) {
                            ImGui::GetWindowDrawList()->AddRect({ health_min.x - 1, BoxMin.y - 1 }, { health_max.x + 1, health_max.y + 1 }, ImColor(0, 0, 0));
                        }
                        else {
                            ImGui::GetWindowDrawList()->AddRect(health_min - ImVec2(1, 1), health_max + ImVec2(1, 1), ImColor(0, 0, 0));
                        }


                        if (Globals::Esp::HealthNumber && (int)std::ceil(playerHealth) != playerMaxHealth) {
                            std::string healthText = std::to_string((int)std::ceil(playerHealth)) + "%";
                            ImVec2 healthTextPos = health_min - ImVec2(ImGui::CalcTextSize(healthText.c_str()).x - 2, 4);


                            ImGui::GetWindowDrawList()->AddText(healthTextPos, ImColor(Globals::Esp::Colors::HealthNumber[0], Globals::Esp::Colors::HealthNumber[1], Globals::Esp::Colors::HealthNumber[2]), healthText.c_str());
                        }
                    }
                    if (Globals::Esp::Font != 0)
                        ImGui::PopFont();
                    ImGui::EndChild();
                    ImGui::End();
                }
                if (DockButtons[1].Enabled)
                {
                    ImGui::SetNextWindowSize({ 363,415 });
                    ImGui::Begin("Style https://discord.gg/XhVvn7McKj", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar
                        | ImGuiWindowFlags_NoResize);
                    ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x, 25), ChildColor, ChildColor, WindowColor, WindowColor);
                    //ImGui::GetBackgroundDrawList()->AddShadowRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), ThemeColor, 30, { 0,0 });
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1), ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1), AccentColor);
                    if (MenuGlow)
                        ImGui::GetBackgroundDrawList()->AddShadowRect(ImGui::GetWindowPos() + ImVec2(1, 1), ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1), AccentColor, MenuGlowFloat, { 0,0 });
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 1);
                    ImGui::TextColored(AccentColor.Value, "Style https://discord.gg/XhVvn7McKj");
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x);
                    ImGui::Text("x");
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
                    {
                        DockButtons[1].Enabled = false;
                    }
                    style->Colors[ImGuiCol_Border] = Inline.Value;
                    ImGui::BeginChild("##c", ImGui::GetContentRegionAvail(), true);
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                        ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                        Outline
                    );
                    style->Colors[ImGuiCol_ChildBg] = WindowColor.Value;
                    style->Colors[ImGuiCol_Border] = Outline.Value;
                    ImGui::BeginChild("##b", ImGui::GetContentRegionAvail(), true);
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                        ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                        Inline
                    );
                    style->Colors[ImGuiCol_ChildBg] = ChildColor;
                    style->Colors[ImGuiCol_Border] = Inline.Value;
                    ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 2 }, true);
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                        ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                        Outline
                    );
                    ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                        ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                        AccentColor
                    );
                    ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                        ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                        ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                    );
                    ImGui::TextColored(TextTitle.Value, "Theme");

                    ImGui::Text("Window color"); ImGui::SameLine();
                    ImGui::ColorEdit3("##wc", WindowColorF, ImGuiColorEditFlags_NoInputs);

                    ImGui::Text("Accent color"); ImGui::SameLine();
                    ImGui::ColorEdit3("##ac", AccentColorF, ImGuiColorEditFlags_NoInputs);

                    ImGui::Text("Child color"); ImGui::SameLine();
                    ImGui::ColorEdit3("##cc", ChildColorF, ImGuiColorEditFlags_NoInputs);

                    ImGui::Text("Outline color"); ImGui::SameLine();
                    ImGui::ColorEdit3("##oc", OutlineF, ImGuiColorEditFlags_NoInputs);



                    ImGui::Text("Dark text color"); ImGui::SameLine();
                    ImGui::ColorEdit3("##tc", TextTitleF, ImGuiColorEditFlags_NoInputs);

                    ImGui::Text("Text color"); ImGui::SameLine();
                    ImGui::ColorEdit3("##wttc", TextF, ImGuiColorEditFlags_NoInputs);

                    ImGui::Text("Unsafe text color"); ImGui::SameLine();
                    ImGui::ColorEdit3("##sdfgdfgdfgfd", UnsafeTextF, ImGuiColorEditFlags_NoInputs);

                    ImGui::Text("Inline color"); ImGui::SameLine();
                    ImGui::ColorEdit3("##wtgfdgdfgdfgdftc", InlineF, ImGuiColorEditFlags_NoInputs);


                    ImGui::EndChild();
                    ImGui::BeginChild("##2", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                        ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                        Outline
                    );
                    ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                        ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                        AccentColor
                    );
                    ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                        ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                        ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                    );
                    ImGui::TextColored(TextTitle.Value, "Other");

                    ImGui::Checkbox("Crosshair", &Crosshair);
                    ImGui::Checkbox("Menu Glow", &MenuGlow);
                    ImGui::Text("Glow Size");
                    ImGui::SliderFloat("##menuglowfloat", &MenuGlowFloat, 5.0f, 100.0f);
                    if (ImGui::Button("Import", { ImGui::GetContentRegionAvail().x / 2, 17 }))
                    {
                        const char* clipboardText = ImGui::GetClipboardText();
                        if (clipboardText && strlen(clipboardText) > 0)
                        {
                            try
                            {

                                auto u32_to_rgb = [](ImU32 colorU32, float out[3]) {
                                    ImVec4 color = ImGui::ColorConvertU32ToFloat4(colorU32);
                                    out[0] = color.x;
                                    out[1] = color.y;
                                    out[2] = color.z;
                                    };
                            }
                            catch (const std::exception& e)
                            {
                                //ImGui::InsertNotification({ ImGuiToastType_None, 3000, "Unable to import" });
                            }
                        }
                    }

                    ImGui::SameLine(0.0f, 4.0f);
                    if (ImGui::Button("Export", { ImGui::GetContentRegionAvail().x, 17 }))
                    {


                    }
                    static const char* PresetThemes[] = { "Default", "Cherry", "Blue", "Purplish", "GameSense", "Onetap", "Assembly", "DavidHook" };
                    static int SelectedTheme = 0;
                    ImGui::Text("Preset");
                    ImGui::Combo("##RR", &SelectedTheme, PresetThemes, IM_ARRAYSIZE(PresetThemes));
                    if (ImGui::Button("Load Theme", { ImGui::GetContentRegionAvail().x, 17 }))
                    {
                        if (SelectedTheme == 0)
                        {
                            static float AccentColorFp[3] = { 50.0f / 255.0, 94.0f / 255.0, 252.0f / 255.0 };
                            static float WindowColorFp[3] = { 24.0f / 255.0, 25.0f / 255.0, 28.0f / 255.0 };
                            static float ChildColorFp[3] = { 11.0f / 255.0, 11.0f / 255.0, 13.0f / 255.0 };
                            static float OutlineFp[3] = { 0.0f / 255.0, 0.0f / 255.0, 0.0f / 255.0 };
                            static float TextTitleFp[3] = { 107.0f / 255.0, 107.0f / 255.0, 108.0f / 255.0 };
                            static float TextFp[3] = { 156.0f / 255.0, 156.0f / 255.0, 157.0f / 255.0 };
                            static float UnsafeTextFp[3] = { 156.0f / 255.0, 191.0f / 255.0, 209.0f / 255.0 };
                            static float InlineFp[3] = { 39.0f / 255.0, 39.0f / 255.0, 39.0f / 255.0 };
                            static bool MenuGlowp = false;
                            static float MenuGlowFloatp = 30.0f;
                            for (int i = 0; i < 3; i++) {
                                AccentColorF[i] = AccentColorFp[i];
                                WindowColorF[i] = WindowColorFp[i];
                                ChildColorF[i] = ChildColorFp[i];
                                OutlineF[i] = OutlineFp[i];
                                TextTitleF[i] = TextTitleFp[i];
                                TextF[i] = TextFp[i];
                                UnsafeTextF[i] = UnsafeTextFp[i];
                                InlineF[i] = InlineFp[i];
                            }
                            MenuGlow = MenuGlowp;
                            MenuGlowFloat = MenuGlowFloatp;
                        }
                        if (SelectedTheme == 1)
                        {
                            static float AccentColorFp[3] = { 175.0f / 255.0, 50.0f / 255.0, 100.0f / 255.0 };
                            static float WindowColorFp[3] = { 30.0f / 255.0, 6.0f / 255.0, 16.0f / 255.0 };
                            static float ChildColorFp[3] = { 22.0f / 255.0, 4.0f / 255.0, 12.0f / 255.0 };
                            static float OutlineFp[3] = { 15.0f / 255.0, 2.0f / 255.0, 7.0f / 255.0 };
                            static float TextTitleFp[3] = { 136.0f / 255.0, 136.0f / 255.0, 136.0f / 255.0 };
                            static float TextFp[3] = { 180.0f / 255.0, 180.0f / 255.0, 180.0f / 255.0 };
                            static float UnsafeTextFp[3] = { 174.0f / 255.0, 50.0f / 255.0, 99.0f / 255.0 };
                            static float InlineFp[3] = { 38.0f / 255.0, 9.0f / 255.0, 21.0f / 255.0 };
                            static bool MenuGlowp = false;
                            static float MenuGlowFloatp = 30.0f;
                            for (int i = 0; i < 3; i++) {
                                AccentColorF[i] = AccentColorFp[i];
                                WindowColorF[i] = WindowColorFp[i];
                                ChildColorF[i] = ChildColorFp[i];
                                OutlineF[i] = OutlineFp[i];
                                TextTitleF[i] = TextTitleFp[i];
                                TextF[i] = TextFp[i];
                                UnsafeTextF[i] = UnsafeTextFp[i];
                                InlineF[i] = InlineFp[i];
                            }
                            MenuGlow = MenuGlowp;
                            MenuGlowFloat = MenuGlowFloatp;
                        }
                        if (SelectedTheme == 2)
                        {
                            static float AccentColorFp[3] = { 42.0f / 255.0, 122.0f / 255.0, 222.0f / 255.0 };
                            static float WindowColorFp[3] = { 0.0f / 255.0, 2.0f / 255.0, 23.0f / 255.0 };
                            static float ChildColorFp[3] = { 3.0f / 255.0, 1.0f / 255.0, 18.0f / 255.0 };
                            static float OutlineFp[3] = { 0.0f / 255.0, 0.0f / 255.0, 0.0f / 255.0 };
                            static float TextTitleFp[3] = { 210.0f / 255.0, 210.0f / 255.0, 210.0f / 255.0 };
                            static float TextFp[3] = { 210.0f / 255.0, 210.0f / 255.0, 210.0f / 255.0 };
                            static float UnsafeTextFp[3] = { 42.0f / 255.0, 122.0f / 255.0, 222.0f / 255.0 };
                            static float InlineFp[3] = { 28.0f / 255.0, 41.0f / 255.0, 64.0f / 255.0 };
                            static bool MenuGlowp = false;
                            static float MenuGlowFloatp = 30.0f;
                            for (int i = 0; i < 3; i++) {
                                AccentColorF[i] = AccentColorFp[i];
                                WindowColorF[i] = WindowColorFp[i];
                                ChildColorF[i] = ChildColorFp[i];
                                OutlineF[i] = OutlineFp[i];
                                TextTitleF[i] = TextTitleFp[i];
                                TextF[i] = TextFp[i];
                                UnsafeTextF[i] = UnsafeTextFp[i];
                                InlineF[i] = InlineFp[i];
                            }
                            MenuGlow = MenuGlowp;
                            MenuGlowFloat = MenuGlowFloatp;
                        }

                        if (SelectedTheme == 3)
                        {
                            static float AccentColorFp[3] = { 155.0f / 255.0, 125.0f / 255.0, 175.0f / 255.0 };
                            static float WindowColorFp[3] = { 42.0f / 255.0, 42.0f / 255.0, 56.0f / 255.0 };
                            static float ChildColorFp[3] = { 36.0f / 255.0, 36.0f / 255.0, 48.0f / 255.0 };
                            static float OutlineFp[3] = { 32.0f / 255.0, 32.0f / 255.0, 38.0f / 255.0 };
                            static float TextTitleFp[3] = { 180.0f / 255.0, 180.0f / 255.0, 180.0f / 255.0 };
                            static float TextFp[3] = { 180.0f / 255.0, 180.0f / 255.0, 180.0f / 255.0 };
                            static float UnsafeTextFp[3] = { 155.0f / 255.0, 125.0f / 255.0, 175.0f / 255.0 };
                            static float InlineFp[3] = { 60.0f / 255.0, 55.0f / 255.0, 75.0f / 255.0 };
                            static bool MenuGlowp = true;
                            static float MenuGlowFloatp = 30.0f;
                            for (int i = 0; i < 3; i++) {
                                AccentColorF[i] = AccentColorFp[i];
                                WindowColorF[i] = WindowColorFp[i];
                                ChildColorF[i] = ChildColorFp[i];
                                OutlineF[i] = OutlineFp[i];
                                TextTitleF[i] = TextTitleFp[i];
                                TextF[i] = TextFp[i];
                                UnsafeTextF[i] = UnsafeTextFp[i];
                                InlineF[i] = InlineFp[i];
                            }
                            MenuGlow = MenuGlowp;
                            MenuGlowFloat = MenuGlowFloatp;
                        }

                        if (SelectedTheme == 4)
                        {
                            static float AccentColorFp[3] = { 156.0f / 255.0, 199.0f / 255.0, 40.0f / 255.0 };
                            static float WindowColorFp[3] = { 20.0f / 255.0, 20.0f / 255.0, 20.0f / 255.0 };
                            static float ChildColorFp[3] = { 12.0f / 255.0, 12.0f / 255.0, 12.0f / 255.0 };
                            static float OutlineFp[3] = { 12.0f / 255.0, 12.0f / 255.0, 12.0f / 255.0 };
                            static float TextTitleFp[3] = { 195.0f / 255.0, 195.0f / 255.0, 195.0f / 255.0 };
                            static float TextFp[3] = { 205.0f / 255.0, 205.0f / 255.0, 205.0f / 255.0 };
                            static float UnsafeTextFp[3] = { 172.0f / 255.0, 177.0f / 255.0, 123.0f / 255.0 };
                            static float InlineFp[3] = { 48.0f / 255.0, 48.0f / 255.0, 48.0f / 255.0 };
                            static bool MenuGlowp = false;
                            static float MenuGlowFloatp = 30.0f;
                            for (int i = 0; i < 3; i++) {
                                AccentColorF[i] = AccentColorFp[i];
                                WindowColorF[i] = WindowColorFp[i];
                                ChildColorF[i] = ChildColorFp[i];
                                OutlineF[i] = OutlineFp[i];
                                TextTitleF[i] = TextTitleFp[i];
                                TextF[i] = TextFp[i];
                                UnsafeTextF[i] = UnsafeTextFp[i];
                                InlineF[i] = InlineFp[i];
                            }
                            MenuGlow = MenuGlowp;
                            MenuGlowFloat = MenuGlowFloatp;
                        }
                        if (SelectedTheme == 5)
                        {
                            static float AccentColorFp[3] = { 252.0f / 255.0, 154.0f / 255.0, 29.0f / 255.0 };
                            static float WindowColorFp[3] = { 30.0f / 255.0, 29.0f / 255.0, 34.0f / 255.0 };
                            static float ChildColorFp[3] = { 18.0f / 255.0, 17.0f / 255.0, 22.0f / 255.0 };
                            static float OutlineFp[3] = { 13.0f / 255.0, 12.0f / 255.0, 17.0f / 255.0 };
                            static float TextTitleFp[3] = { 184.0f / 255.0, 183.0f / 255.0, 188.0f / 255.0 };
                            static float TextFp[3] = { 233.0f / 255.0, 232.0f / 255.0, 237.0f / 255.0 };
                            static float UnsafeTextFp[3] = { 250.0f / 255.0, 161.0f / 255.0, 33.0f / 255.0 };
                            static float InlineFp[3] = { 68.0f / 255.0, 67.0f / 255.0, 72.0f / 255.0 };
                            static bool MenuGlowp = false;
                            static float MenuGlowFloatp = 10.0f;
                            for (int i = 0; i < 3; i++) {
                                AccentColorF[i] = AccentColorFp[i];
                                WindowColorF[i] = WindowColorFp[i];
                                ChildColorF[i] = ChildColorFp[i];
                                OutlineF[i] = OutlineFp[i];
                                TextTitleF[i] = TextTitleFp[i];
                                TextF[i] = TextFp[i];
                                UnsafeTextF[i] = UnsafeTextFp[i];
                                InlineF[i] = InlineFp[i];
                            }
                            MenuGlow = MenuGlowp;
                            MenuGlowFloat = MenuGlowFloatp;
                        }
                        if (SelectedTheme == 6)
                        {
                            static float AccentColorFp[3] = { 139.0f / 255.0, 152.0f / 255.0, 199.0f / 255.0 };
                            static float WindowColorFp[3] = { 25.0f / 255.0, 28.0f / 255.0, 37.0f / 255.0 };
                            static float ChildColorFp[3] = { 10.0f / 255.0, 11.0f / 255.0, 16.0f / 255.0 };
                            static float OutlineFp[3] = { 0.0f / 255.0, 0.0f / 255.0, 0.0f / 255.0 };
                            static float TextTitleFp[3] = { 221.0f / 255.0, 234.0f / 255.0, 246.0f / 255.0 };
                            static float TextFp[3] = { 221.0f / 255.0, 234.0f / 255.0, 246.0f / 255.0 };
                            static float UnsafeTextFp[3] = { 139.0f / 255.0, 142.0f / 255.0, 199.0f / 255.0 };
                            static float InlineFp[3] = { 43.0f / 255.0, 48.0f / 255.0, 64.0f / 255.0 };
                            static bool MenuGlowp = true;
                            static float MenuGlowFloatp = 10.0f;
                            for (int i = 0; i < 3; i++) {
                                AccentColorF[i] = AccentColorFp[i];
                                WindowColorF[i] = WindowColorFp[i];
                                ChildColorF[i] = ChildColorFp[i];
                                OutlineF[i] = OutlineFp[i];
                                TextTitleF[i] = TextTitleFp[i];
                                TextF[i] = TextFp[i];
                                UnsafeTextF[i] = UnsafeTextFp[i];
                                InlineF[i] = InlineFp[i];
                            }
                            MenuGlow = MenuGlowp;
                            MenuGlowFloat = MenuGlowFloatp;
                        }
                        if (SelectedTheme == 7)
                        {
                            static float AccentColorFp[3] = { 152.0f / 255.0, 122.0f / 255.0, 173.0f / 255.0 };
                            static float WindowColorFp[3] = { 25.0f / 255.0, 25.0f / 255.0, 25.0f / 255.0 };
                            static float ChildColorFp[3] = { 14.0f / 255.0, 15.0f / 255.0, 14.0f / 255.0 };
                            static float OutlineFp[3] = { 10.0f / 255.0, 10.0f / 255.0, 13.0f / 255.0 };
                            static float TextTitleFp[3] = { 120.0f / 255.0, 121.0f / 255.0, 121.0f / 255.0 };
                            static float TextFp[3] = { 254.0f / 255.0, 255.0f / 255.0, 254.0f / 255.0 };
                            static float UnsafeTextFp[3] = { 162.0f / 255.0, 134.0f / 255.0, 180.0f / 255.0 };
                            static float InlineFp[3] = { 30.0f / 255.0, 30.0f / 255.0, 30.0f / 255.0 };
                            static bool MenuGlowp = true;
                            static float MenuGlowFloatp = 100.0f;
                            for (int i = 0; i < 3; i++) {
                                AccentColorF[i] = AccentColorFp[i];
                                WindowColorF[i] = WindowColorFp[i];
                                ChildColorF[i] = ChildColorFp[i];
                                OutlineF[i] = OutlineFp[i];
                                TextTitleF[i] = TextTitleFp[i];
                                TextF[i] = TextFp[i];
                                UnsafeTextF[i] = UnsafeTextFp[i];
                                InlineF[i] = InlineFp[i];
                            }
                            MenuGlow = MenuGlowp;
                            MenuGlowFloat = MenuGlowFloatp;
                        }
                    }

                    ImGui::EndChild();
                    ImGui::EndChild();
                    ImGui::EndChild();
                    ImGui::End();
                }
            }  
            if (Globals::Esp::KeybindList)
            {
                std::vector<CKeybind> Keybinds = {Globals::Misc::DesyncBind, Globals::Aimbot::AimbotKey, Globals::TriggerBot::Key, Globals::World::Movement::CframeKey, Globals::World::Movement::FlyKey,Globals::World::Movement::FreecamKey,Globals::World::Movement::ThirdPersonKey, Globals::World::ZoomKey };
                ImGui::SetNextWindowSize({ 209.0f, (17.0f * static_cast<float>(Keybinds.size())) + 24.0f + 9.0f});
                style->Colors[ImGuiCol_WindowBg] = style->Colors[ImGuiCol_ChildBg];
                style->Colors[ImGuiCol_Border] = Outline.Value;
                style->Colors[ImGuiCol_Separator] = Inline.Value;
                ImGui::Begin("Keybindslist",nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
                ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                    ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                    Inline
                );
                ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                    ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                    AccentColor
                );
                ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                    ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                    ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                );
                ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Keybinds").x) / 2);
                ImGui::Text("Keybinds");
                ImGui::Separator();
                for (CKeybind Key : Keybinds)
                {
                    
                        bool enabled = Key.enabled;
                        std::string keyType = Key.get_type();
                       
                        if (enabled)
                            ImGui::PushStyleColor(ImGuiCol_Text, AccentColor.Value);
                        ImGui::Text("[ %s ] %s",  keyType.c_str(), Key.name);
                        ImGui::SameLine(ImGui::GetWindowSize().x - ImGui::CalcTextSize(Key.get_key_name().c_str()).x - 5);
                        ImGui::Text(Key.get_key_name().c_str());
                        if (enabled)
                            ImGui::PopStyleColor();
                    
                }
                ImGui::End();
            }
            
            
            static float alpha = 0.0f;
            static bool increasing = true;
            static float angle = 0.0f;
            static float animX = 0.0f;
            static float animY = 0.0f;
            if (Crosshair)
            {
                float dt = ImGui::GetIO().DeltaTime;

                
                float alphaSpeed = 1.44f;
                if (increasing) {
                    alpha += alphaSpeed * dt;
                    if (alpha >= 1.0f) {
                        alpha = 1.0f;
                        increasing = false;
                    }
                }
                else {
                    alpha -= alphaSpeed * dt;
                    if (alpha <= 0.0f) {
                        alpha = 0.0f;
                        increasing = true;
                    }
                }

               
                angle += 4.0f * dt;
                if (angle > IM_PI * 2.0f) angle -= IM_PI * 2.0f;

                
                POINT cursor_point;
                GetCursorPos(&cursor_point);
                ScreenToClient(hwnd, &cursor_point);
                if (Globals::Aimbot::IsThereATarget && Globals::Aimbot::SilentCrosshair && Globals::Aimbot::AimType == 2)
                {
                    
                    
                   
                }
              
                if (animX == 0.0f) animX = cursor_point.x;
                if (animY == 0.0f) animY = cursor_point.y;
               
                float smoothing = 10.0f; 
                float lerpFactor = 1.0f - expf(-smoothing * dt);

                animX += (cursor_point.x - animX) * lerpFactor;
                animY += (cursor_point.y - animY) * lerpFactor;
                if (Globals::Aimbot::IsThereATarget && Globals::Aimbot::SilentCrosshair && Globals::Aimbot::AimType == 2)
                {

                   
                }

                ImDrawList* draw_list = ImGui::GetForegroundDrawList();
                ImVec2 center(animX, animY);
                ImColor crosshairColor = ImColor(255, 255, 255);

                float size = 10.0f;
                float thickness = 1.5f;
                float gap = 5.0f;

               
                ImVec2 lines[4][2] = {
                    { ImVec2(center.x - size - gap, center.y), ImVec2(center.x - gap, center.y) },
                    { ImVec2(center.x + gap, center.y), ImVec2(center.x + size + gap, center.y) },
                    { ImVec2(center.x, center.y - size - gap), ImVec2(center.x, center.y - gap) },
                    { ImVec2(center.x, center.y + gap), ImVec2(center.x, center.y + size + gap) }
                };

               
                float s = sin(angle);
                float c = cos(angle);
                for (int i = 0; i < 4; ++i) {
                    
                    ImVec2 p1 = lines[i][0], p2 = lines[i][1];
                    float p1x = p1.x - center.x;
                    float p1y = p1.y - center.y;
                    float p2x = p2.x - center.x;
                    float p2y = p2.y - center.y;

                    ImVec2 rp1(p1x * c - p1y * s + center.x, p1x * s + p1y * c + center.y);
                    ImVec2 rp2(p2x * c - p2y * s + center.x, p2x * s + p2y * c + center.y);

                   
                    draw_list->AddLine(ImVec2(rp1.x - 1, rp1.y), ImVec2(rp2.x - 1, rp2.y), IM_COL32(0, 0, 0, 255), 1.0f);
                    draw_list->AddLine(ImVec2(rp1.x + 1, rp1.y), ImVec2(rp2.x + 1, rp2.y), IM_COL32(0, 0, 0, 255), 1.0f);
                    draw_list->AddLine(ImVec2(rp1.x, rp1.y - 1), ImVec2(rp2.x, rp2.y - 1), IM_COL32(0, 0, 0, 255), 1.0f);
                    draw_list->AddLine(ImVec2(rp1.x, rp1.y + 1), ImVec2(rp2.x, rp2.y + 1), IM_COL32(0, 0, 0, 255), 1.0f);

                    
                    draw_list->AddLine(rp1, rp2, crosshairColor, thickness);
                }

              
                ImVec2 textSize = ImGui::CalcTextSize("Athena for more sources https://discord.gg/XhVvn7McKj");
                ImVec2 nemesisSize = ImGui::CalcTextSize("Ath");

                draw_list->AddText(ImVec2(center.x - textSize.x / 2, center.y + size + gap + 5.0f), IM_COL32(255, 255, 255, 255), "Ath");

                draw_list->AddText(ImVec2(center.x - textSize.x / 2 + nemesisSize.x, center.y + size + gap + 5.0f), ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, alpha), "ena");
            }
            if (Globals::Watermark)
            {
                style->Colors[ImGuiCol_WindowBg] = WindowColor.Value;
                style->Colors[ImGuiCol_ChildBg] = ChildColor.Value;
                style->Colors[ImGuiCol_Border] = Inline.Value;
                
                std::string WatermarkText = "join https://discord.gg/XhVvn7McKj | " + Globals::Auth::Username + " | Build: Private";
                ImGui::SetNextWindowSize(ImGui::CalcTextSize(WatermarkText.c_str()) + ImVec2(30,30));
                ImGui::Begin("LOLLLLLLLLLXDDDDDDWATERMARK", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
                ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y}, true);
                ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos() + ImVec2(1, 1),
                    ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(1, 1),
                    Outline
                );
                ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 2),
                    ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 2),
                    AccentColor
                );
                ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos() + ImVec2(2, 3),
                    ImGui::GetWindowPos() + ImVec2(ImGui::GetWindowSize().x - 2, 3),
                    ImColor(AccentColor.Value.x, AccentColor.Value.y, AccentColor.Value.z, 0.5f)
                );
                ImGui::SetCursorPos(
                    {
                        (ImGui::GetWindowSize().x - ImGui::CalcTextSize(WatermarkText.c_str()).x) / 2,
                        (ImGui::GetWindowSize().y - ImGui::CalcTextSize(WatermarkText.c_str()).y) / 2
                    }
                );
                ImGui::Text(WatermarkText.c_str());
                ImGui::EndChild();
                ImGui::End();
            }
        }
        if (draw)
        {
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        }
        else
        {
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        }
        if (Globals::Misc::StreamProof)
        {
            SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
        }
        else
        {
            SetWindowDisplayAffinity(hwnd, WDA_NONE);
        }
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0,0,0,0 };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        int vsyncison = 0;
        if (Globals::Vsync)
        {
            vsyncison = 1;
        }
        else
        {
            vsyncison = 0;
        }
        HRESULT hr = g_pSwapChain->Present(vsyncison, 0);
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    exit(0);
}

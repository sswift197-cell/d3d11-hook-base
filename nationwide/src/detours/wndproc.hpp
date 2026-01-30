#pragma once

#include<Windows.h>
#include<d3d11.h>
#include<dxgi.h>

#include"../../deps/imgui/imgui.h"
#include"../../deps/imgui/imgui_impl_dx11.h"
#include"../../deps/imgui/imgui_impl_win32.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND,UINT,WPARAM,LPARAM);
using wndproc_t = LRESULT(*)(HWND,UINT,WPARAM,LPARAM);

static ID3D11Device* m_device=nullptr;
static ID3D11DeviceContext* m_context = nullptr;
static ID3D11RenderTargetView* m_render_target = nullptr;
static HWND m_window = 0;

wndproc_t wndproc_fn = 0;
static bool render_overlay = false;

auto __stdcall WndProcDetour(HWND hWindow, UINT uInt, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    if (render_overlay)
    {
        ImGui_ImplWin32_WndProcHandler(hWindow, uInt, wParam, lParam);
        return TRUE;
    }
    return ::CallWindowProcA(wndproc_fn, hWindow, uInt, wParam, lParam);
}
#pragma once
#include"release.hpp"

using present_t = HRESULT(*)(IDXGISwapChain*,UINT,UINT);
present_t present_fn = 0;

auto __stdcall PresentDetour(IDXGISwapChain* pSwap, UINT uInt, UINT uFlags) -> HRESULT
{
    static bool init_imgui =false;
    static IDXGISwapChain* last_swap = nullptr;

    if (last_swap != pSwap)
    {
        last_swap = pSwap;

        if (m_render_target) {m_render_target->Release(); m_render_target = nullptr;}
        if (m_context) {m_context->Release(); m_context = nullptr;}
        if (m_device) {m_device->Release(); m_device = nullptr;}

        pSwap->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&m_device));
        m_device->GetImmediateContext(&m_context);

        DXGI_SWAP_CHAIN_DESC m_desc = {};
        pSwap->GetDesc(&m_desc);
        m_window = m_desc.OutputWindow;

        ID3D11Texture2D* m_back_buffer = nullptr;
        if(SUCCEEDED(pSwap->GetBuffer(0, IID_PPV_ARGS(&m_back_buffer))))
        {
            D3D11_TEXTURE2D_DESC m_desc = {};
            m_back_buffer->GetDesc(&m_desc);

            D3D11_RENDER_TARGET_VIEW_DESC rtv = {};
            rtv.Format = m_desc.Format;
            rtv.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

            m_device->CreateRenderTargetView(m_back_buffer, &rtv, &m_render_target);
            m_back_buffer->Release();
        }

        if (!init_imgui)
        {
            ImGui::CreateContext();
            ImGui_ImplWin32_Init(m_window);
            ImGui_ImplDX11_Init(m_device, m_context);
            wndproc_fn = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(m_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProcDetour)));
            init_imgui = true;
        }

        ImGui_ImplDX11_CreateDeviceObjects();
    }

    if (!m_render_target) 
        return present_fn(pSwap, uInt, uFlags);

    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX11_NewFrame();
    ImGui::NewFrame();

    if (GetAsyncKeyState(VK_INSERT) & 0x1)
        render_overlay = !render_overlay;

    if (render_overlay) {
        ImGui::Begin("D3D11 Hook");
        ImGui::SetWindowSize(ImVec2(250, 250));
        ImGui::End();
    }

    ImGui::Render();
    m_context->OMSetRenderTargets(1, &m_render_target, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return present_fn(pSwap, uInt, uFlags);
}
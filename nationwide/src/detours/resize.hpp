#pragma once
#include"wndproc.hpp"

using resize_t = HRESULT(*)(IDXGISwapChain*,UINT,UINT,DXGI_FORMAT,UINT);
resize_t resize_fn = 0;

auto __stdcall ResizeDetour(IDXGISwapChain* pSwap, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapchainDesc) -> HRESULT
{
    auto original = resize_fn(pSwap, Width, Height, Format, SwapchainDesc);
    
    ImGui_ImplDX11_InvalidateDeviceObjects();
    if (m_render_target)
    {
        m_render_target->Release();
        m_render_target = nullptr;
    }

    ID3D11Texture2D* m_back_buffer = nullptr;
    if (SUCCEEDED(pSwap->GetBuffer(0, IID_PPV_ARGS(&m_back_buffer))))
    {
        D3D11_TEXTURE2D_DESC m_desc = {};
        m_back_buffer->GetDesc(&m_desc);

        D3D11_RENDER_TARGET_VIEW_DESC rtv = {};
        rtv.Format = m_desc.Format;
        rtv.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

        m_device->CreateRenderTargetView(m_back_buffer, &rtv, &m_render_target);
        m_back_buffer->Release();
    }

    return original;
}
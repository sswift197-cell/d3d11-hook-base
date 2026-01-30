#pragma once
#include"resize.hpp"

using release_t = HRESULT(*)(__int64);
release_t release_fn = 0;

auto __stdcall ReleaseDetour(__int64 a1) -> HRESULT
{
    m_context->OMSetRenderTargets(0, nullptr, nullptr);
    if (m_render_target) m_render_target->Release();
    m_render_target = nullptr;
    m_context->Flush();
    return release_fn(a1);
}
#pragma once

#include"../pattern/pattern.hpp"

namespace nation
{
    class offsets {
    public:
        std::uintptr_t m_present_func;
        std::uintptr_t m_resize_func;
        std::uintptr_t m_release_func;
    public:
        auto intialize() -> void;
    }; inline offsets m_offsets;

    inline auto offsets::intialize() -> void
    {
        this->m_present_func = reinterpret_cast<std::uintptr_t>(m_scanner.find_pattern("GameOverlayRenderer64.dll", 
            "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 41 8B E8"));
        this->m_release_func = reinterpret_cast<std::uintptr_t>(m_scanner.find_pattern("GameOverlayRenderer64.dll", 
            "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8B D9"));
        this->m_resize_func = reinterpret_cast<std::uintptr_t>(m_scanner.find_pattern("GameOverlayRenderer64.dll", 
            "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 54 41 56 41 57 48 83 EC ? 44 8B E2"));
    }
}
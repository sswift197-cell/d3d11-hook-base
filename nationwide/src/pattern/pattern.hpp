#pragma once

#include<vector>
#include<math.h>
#include<cstdint>
#include<Windows.h>
#include<thread>
#include<chrono>

namespace nation
{
    class scanner
    {
    public:
        auto pattern_to_bytes(const char* pattern) -> std::vector<int>;
        auto find_pattern(const char* module_dll, const char* pattern) -> std::uint8_t*;
    }; inline scanner m_scanner;
}
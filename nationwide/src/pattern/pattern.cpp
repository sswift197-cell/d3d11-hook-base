#include"pattern.hpp"

auto nation::scanner::pattern_to_bytes(const char* pattern) -> std::vector<int>
{
    auto bytes = std::vector<int>();
    auto start = const_cast<char*>(pattern);
    auto end = const_cast<char*>(pattern) + strlen(pattern);

    for (auto current = start; current <= end; ++current)
    {
        if (*current == '?')
        {
            ++current;
            if (*current == '?')
                ++current;
            bytes.push_back(-1);
        }
        else
        {
            bytes.push_back(strtoul(current, &current, 16));
        }
    }
    return bytes;
}

auto nation::scanner::find_pattern(const char* module_dll, const char* pattern) -> std::uint8_t*
{
    auto mod_base = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(module_dll));
    auto dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(mod_base);
    auto nt_headers = reinterpret_cast<IMAGE_NT_HEADERS*>(mod_base + dos_header->e_lfanew);

    auto bytes = this->pattern_to_bytes(pattern);
    auto bytes_size = bytes.size();
    auto bytes_data = bytes.data();

    auto scan_bytes = reinterpret_cast<std::uint8_t*>(mod_base);
    auto size = nt_headers->OptionalHeader.SizeOfImage;

    for (auto i = 0ul; i < size - bytes_size; ++i)
    {
        bool found = true;
        for (auto j = 0ul; j < bytes_size; ++j)
        {
            if (scan_bytes[i+j] != bytes_data[j] && bytes_data[j] != -1) {found = false; break;}
        }
        if (found)
            return &scan_bytes[i];
    }
    return nullptr;
}
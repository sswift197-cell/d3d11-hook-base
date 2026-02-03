#include "hook.hpp"

#include <cstring>
#include <algorithm>
#include <Psapi.h>
#include <TlHelp32.h>

#pragma comment(lib, "psapi.lib")
core::X64Hook *core::X64Hook::instance = nullptr;

core::X64Hook::X64Hook(void *target, void **original) : target(target), original(original) {}

core::X64Hook::~X64Hook()
{
    DisableAllHooks();
    RemoveAllHooks();
}

BOOL core::X64Hook::VirtualProtectMemory(void *address, std::size_t size, unsigned long new_protect, unsigned long *old_protect)
{
    return ::VirtualProtect(address, size, new_protect, old_protect);
}

bool core::X64Hook::CreateHook(void *detour, const std::string &name)
{
    if (!target || !detour || !original)
        return false;

    if (this->IsHooked(target))
        return false;

    HookInfo hook;
    hook.target = target;
    hook.detour = detour;
    hook.isActive = false;
    hook.name = name.empty() ? "Unnamed_hook" : name;

    std::printf("hook name: %s\n", hook.name.c_str());
    std::printf("target: 0x%llx\n", hook.target);
    std::printf("detour: 0x%llx\n", hook.detour);

    if (!this->CreateTramp(hook))
    {
        std::fprintf(stderr, "failed to create tramp\n");
        return false;
    }

    *original = hook.gateway;
    hooks.push_back(hook);

    return true;
}

bool core::X64Hook::CreateTramp(HookInfo &hook)
{
    const std::size_t hook_size = 14;
    
    // Save original bytes
    hook.originalBytes.resize(hook_size);
    
    // Read original bytes
    DWORD oldProtect;
    if (!VirtualProtectMemory(hook.target, hook_size, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        std::fprintf(stderr, "CreateTramp: Failed to change protection for reading\n");
        return false;
    }
    
    memcpy(hook.originalBytes.data(), hook.target, hook_size);
    VirtualProtectMemory(hook.target, hook_size, oldProtect, &oldProtect);
    
    hook.stolenSize = hook_size;

    std::printf("Stolen %zu bytes from 0x%p\n", hook_size, hook.target);
    
    // Debug: Print stolen bytes
    std::printf("Original bytes: ");
    for (size_t i = 0; i < hook_size; i++)
    {
        printf("%02X ", hook.originalBytes[i]);
    }
    printf("\n");

    // Allocate space for trampoline (original bytes + jump back)
    // We need enough space for original bytes + 14 byte jump
    std::size_t trampoline_size = hook_size + 14;
    hook.trampolineCode.resize(trampoline_size);
    
    // Copy original bytes to trampoline
    memcpy(hook.trampolineCode.data(), hook.originalBytes.data(), hook_size);

    // Calculate address to jump back to (original + stolen bytes)
    void *continue_addr = reinterpret_cast<BYTE *>(hook.target) + hook_size;
    BYTE *jmp_back_pos = hook.trampolineCode.data() + hook_size;

    // Create absolute jump back to continue address
    // FF 25 00 00 00 00 [64-bit address]
    JMP_REL64 jmp_back;
    jmp_back.opcode = 0xFF;
    jmp_back.modrm = 0x25;
    jmp_back.disp32 = 0;
    jmp_back.target = reinterpret_cast<UINT64>(continue_addr);

    std::printf("Creating jump back to: 0x%p\n", continue_addr);
    
    // Copy the jump structure
    memcpy(jmp_back_pos, &jmp_back, sizeof(jmp_back));
    
    // Allocate executable memory for gateway
    hook.gateway = VirtualAlloc(nullptr, trampoline_size, 
                                MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (!hook.gateway)
    {
        std::fprintf(stderr, "CreateTramp: Failed to allocate gateway memory\n");
        return false;
    }

    std::printf("Allocated gateway at: 0x%p\n", hook.gateway);
    
    // Copy trampoline code to gateway
    memcpy(hook.gateway, hook.trampolineCode.data(), trampoline_size);

    // Debug: Verify trampoline bytes
    std::printf("Trampoline bytes at gateway: ");
    BYTE* gateway_bytes = reinterpret_cast<BYTE*>(hook.gateway);
    for (size_t i = 0; i < trampoline_size; i++)
    {
        printf("%02X ", gateway_bytes[i]);
    }
    printf("\n");

    // Make gateway executable only
    unsigned long old_protect;
    if (!VirtualProtectMemory(hook.gateway, trampoline_size, PAGE_EXECUTE_READ, &old_protect))
    {
        std::fprintf(stderr, "CreateTramp: Failed to protect gateway memory\n");
        VirtualFree(hook.gateway, 0, MEM_RELEASE);
        return false;
    }

    return true;
}

bool core::X64Hook::EnableHook(void *target)
{
   std::printf("\n=== EnableHook called for 0x%p ===\n", target);
    
    auto it = std::find_if(hooks.begin(), hooks.end(),
                           [target](const HookInfo &hook)
                           { return hook.target == target; });

    if (it == hooks.end())
    {
        std::fprintf(stderr, "EnableHook: Hook not found\n");
        return false;
    }

    HookInfo &hook = *it;
    
    if (hook.isActive)
    {
        std::printf("EnableHook: Hook already active\n");
        return true;
    }
    
    unsigned long old_protect;

    // Change memory protection to writable
    if (!VirtualProtectMemory(hook.target, hook.stolenSize, PAGE_EXECUTE_READWRITE, &old_protect))
    {
        std::fprintf(stderr, "EnableHook: Failed to change memory protection\n");
        return false;
    }

    hook.oldProtect = old_protect;
    std::printf("Changed protection for 0x%p (size: %zu)\n", hook.target, hook.stolenSize);

    // Create absolute jump to detour (more reliable than push/ret)
    // FF 25 00 00 00 00 [64-bit address]
    JMP_REL64 jump_to_detour;
    jump_to_detour.opcode = 0xFF;
    jump_to_detour.modrm = 0x25;
    jump_to_detour.disp32 = 0;
    jump_to_detour.target = reinterpret_cast<UINT64>(hook.detour);

    std::printf("Creating jump to detour: 0x%p\n", hook.detour);
    std::printf("Jump bytes: ");
    BYTE* jump_bytes = reinterpret_cast<BYTE*>(&jump_to_detour);
    for (size_t i = 0; i < sizeof(jump_to_detour); i++)
    {
        printf("%02X ", jump_bytes[i]);
    }
    printf("\n");

    // Write the jump
    if (!WriteMem(hook.target, &jump_to_detour, sizeof(jump_to_detour)))
    {
        std::fprintf(stderr, "EnableHook: Failed to write jump to target\n");
        VirtualProtectMemory(hook.target, hook.stolenSize, hook.oldProtect, &old_protect);
        return false;
    }

    // Verify the written bytes
    std::printf("Verifying written bytes at 0x%p: ", hook.target);
    BYTE* target_bytes = reinterpret_cast<BYTE*>(hook.target);
    for (size_t i = 0; i < sizeof(jump_to_detour); i++)
    {
        printf("%02X ", target_bytes[i]);
    }
    printf("\n");

    // Fill remaining bytes with NOPs if needed
    if (hook.stolenSize > sizeof(jump_to_detour))
    {
        std::size_t nop_count = hook.stolenSize - sizeof(jump_to_detour);
        BYTE *nop_pos = reinterpret_cast<BYTE *>(hook.target) + sizeof(jump_to_detour);
        memset(nop_pos, 0x90, nop_count);
        std::printf("Filled %zu bytes with NOPs\n", nop_count);
    }

    // Restore original protection
    if (!VirtualProtectMemory(hook.target, hook.stolenSize, hook.oldProtect, &old_protect))
    {
        std::fprintf(stderr, "EnableHook: Failed to restore protection\n");
    }
    
    // IMPORTANT: Flush instruction cache
    FlushInstructionCache(GetCurrentProcess(), hook.target, hook.stolenSize);
    std::printf("Flushed instruction cache\n");

    hook.isActive = true;
    
    std::printf("=== Hook enabled successfully: %s ===\n\n", hook.name.c_str());
    return true;
}

bool core::X64Hook::DisableHook(void *target)
{
    auto it = std::find_if(hooks.begin(), hooks.end(),
                           [target](const HookInfo &hook)
                           { return hook.target == target; });

    if (it == hooks.end() || !it->isActive)
        return false;

    HookInfo &hook = *it;
    unsigned long old_protect;
    this->VirtualProtectMemory(hook.target, hook.stolenSize,
                               PAGE_EXECUTE_READWRITE, &old_protect);

    this->WriteMem(hook.target, hook.originalBytes.data(), hook.stolenSize);
    this->VirtualProtectMemory(hook.target, hook.stolenSize, hook.oldProtect, &old_protect);

    FlushInstructionCache(GetCurrentProcess(), hook.target, hook.stolenSize);

    return true;
}

bool core::X64Hook::RemoveHook(void *target)
{
    auto it = std::find_if(hooks.begin(), hooks.end(),
                           [target](const HookInfo &hook)
                           { return hook.target == target; });

    if (it == hooks.end())
        return false;

    if (it->isActive)
        this->DisableHook(target);

    hooks.erase(it);
    return true;
}

bool core::X64Hook::WriteMem(void *dest, void *source, std::size_t size)
{
    if (!dest || !source || size == 0)
        return false;

    unsigned long old_protect;
    BOOL status = this->VirtualProtectMemory(dest, size, PAGE_EXECUTE_READWRITE, &old_protect);
    if (status == FALSE)
    {
        std::fprintf(stderr, "184\n");
        return false;
    }

    __try
    {
        memcpy(dest, source, size);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        std::fprintf(stderr, "WriteMem: Exception while writing to 0x%p\n", dest);
        this->VirtualProtectMemory(dest, size, old_protect, &old_protect);
        return false;
    }

    this->VirtualProtectMemory(dest, size, old_protect, &old_protect);
    return true;
}

bool core::X64Hook::BuildAbsJmp(BYTE *buffer, void *target)
{
    buffer[0] = 0xFF;
    buffer[1] = 0x25;
    *(DWORD *)(buffer + 2) = 0;
    *(UINT64 *)(buffer + 6) = (UINT64)target;
    return true;
}

bool core::X64Hook::BuildRelJmp(BYTE *buffer, void *from, void *to)
{
    INT64 displacement = (INT64)to - ((INT64)from + 5);
    if (displacement < INT32_MIN || displacement > INT32_MAX)
        return false;

    buffer[0] = 0xE9;
    *(INT32 *)(buffer + 1) = (INT32)displacement;
    return true;
}

bool core::X64Hook::BuildTrampGateway(BYTE *gateway, void *stolen_bytes, std::size_t stolen_size, void *address)
{
    memcpy(gateway, stolen_bytes, stolen_size);
    BYTE *jmpBack = gateway + stolen_size;
    if (BuildRelJmp(jmpBack, jmpBack, address))
        return true;
    return BuildAbsJmp(jmpBack, address);
}

std::size_t core::X64Hook::GetInstructionLength(void *address, std::size_t max_length)
{
    BYTE *bytes = (BYTE *)address;

    if (max_length < 0)
        switch (bytes[0])
        {
        case 0x90:
        case 0xC3:
        case 0xCC:
        case 0xF4:
            return 1;
        case 0x0F:
            if (max_length < 2)
                return 0;
            switch (bytes[1])
            {
            case 0x1F:
                if (max_length >= 3 && (bytes[2] & 0xC0) == 0x00)
                {
                    return 3 + (bytes[2] & 0x1F);
                }
                break;
            case 0x05:
            case 0x34:
                return 2;
            }
            break;

        case 0xEB:
            return 2;

        case 0xE9:
            return 5;

        case 0xE8:
            return 5;

        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0xB0:
        case 0xB8:
            if (max_length < 2)
                return 0;
            BYTE modRM = bytes[1];
            BYTE mod = (modRM >> 6) & 3;
            BYTE rm = modRM & 7;

            if (mod == 0 && rm == 5)
            {
                return 7;
            }
            else if (mod == 3)
            {
                return 2;
            }
            else if (mod == 1)
            {
                return 3;
            }
            else if (mod == 2)
            {
                return 6;
            }
            break;
        }

    for (auto i = 1; i < max_length; i++)
    {
        if (bytes[i] == 0x90 || bytes[i] == 0xC3 || bytes[i] == 0xCC ||
            bytes[i] == 0xE9 || bytes[i] == 0xE8 || bytes[i] == 0xEB ||
            (bytes[i] == 0x0F && i + 1 < max_length))
            return i;
    }

    return max_length;
}

bool core::X64Hook::EnableAllHooks()
{
    bool success = true;
    for (auto &hook : hooks)
    {
        if (!hook.isActive)
        {
            success = this->EnableHook(hook.target) && success;
        }
    }
    return success;
}

bool core::X64Hook::DisableAllHooks()
{
    bool success = true;
    for (auto &hook : hooks)
    {
        if (hook.isActive)
        {
            success = DisableHook(hook.target) && success;
        }
    }
    return success;
}

bool core::X64Hook::RemoveAllHooks()
{
    bool success = true;
    while (!hooks.empty())
    {
        success = RemoveHook(hooks.back().target) && success;
    }
    return success;
}

bool core::X64Hook::IsHooked(void *target) const
{
    return std::any_of(hooks.begin(), hooks.end(), [target](const HookInfo &hook)
                       { return hook.target == target; });
}

core::X64Hook::HookInfo *core::X64Hook::GetHookInfo(void *target)
{
    auto it = std::find_if(hooks.begin(), hooks.end(),
                           [target](const HookInfo &hook)
                           { return hook.target == target; });
    return it != hooks.end() ? &(*it) : nullptr;
}

std::size_t core::X64Hook::GetHookCount() const
{
    return hooks.size();
}
#include"hook/hook.hpp"
#include"offsets/offsets.hpp"
#include"detours/present.hpp"

auto __stdcall Entry(void* hInstDll) -> void
{
    AllocConsole();
    FILE* in = {};
    freopen_s(&in, "CONOUT$", "w", stdout);
    nation::m_offsets.intialize();

    if (MH_Initialize() != MH_OK)
        std::printf("error here\n");

    if (MH_CreateHook(reinterpret_cast<void*>(nation::m_offsets.m_present_func), 
        reinterpret_cast<void*>(PresentDetour), reinterpret_cast<void**>(&present_fn)) != MH_OK)
        std::printf("failed to create hook on: -> 0x%llx\n", nation::m_offsets.m_present_func);

    if (MH_CreateHook(reinterpret_cast<void*>(nation::m_offsets.m_release_func), 
        reinterpret_cast<void*>(ReleaseDetour), reinterpret_cast<void**>(&release_fn)) != MH_OK)
        std::printf("failed to create hook: on -> 0x%llx\n", nation::m_offsets.m_release_func);

    if (MH_CreateHook(reinterpret_cast<void*>(nation::m_offsets.m_resize_func), 
        reinterpret_cast<void*>(ResizeDetour), reinterpret_cast<void**>(&resize_fn)) != MH_OK)
        std::printf("failed to create hook: on -> 0x%llx\n", nation::m_offsets.m_resize_func);

    MH_EnableHook(MH_ALL_HOOKS);
    while(!(GetAsyncKeyState(VK_END) & 0x1))
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
    
    fclose(in);
    FreeConsole();
    FreeLibraryAndExitThread(reinterpret_cast<HINSTANCE>(hInstDll), 0);
}

auto __stdcall DllMain(void* hInstDll, 
    unsigned long fdwReasons, void* lpReversed) -> bool
{
    switch(fdwReasons) {
        case DLL_PROCESS_ATTACH:
            ::DisableThreadLibraryCalls(reinterpret_cast<HINSTANCE>(hInstDll));
            ::CloseHandle(::CreateThread(nullptr, 0,
                reinterpret_cast<LPTHREAD_START_ROUTINE>(Entry), hInstDll, 0, nullptr));
            break;
    }
    return true;
}
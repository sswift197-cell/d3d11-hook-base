#include"hook.hpp"

nation::hook::hook(void* target, void** original) : target(target), original(original) {};

auto nation::hook::intialize() -> void
{
    if (MH_Initialize() != MH_STATUS::MH_OK) return;
}

auto nation::hook::create_hook(void* detour) -> bool
{
    if (MH_CreateHook(target, detour, original) == MH_STATUS::MH_OK) return true;
    return false;
}

auto nation::hook::remove_hooks() -> void
{
    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
}
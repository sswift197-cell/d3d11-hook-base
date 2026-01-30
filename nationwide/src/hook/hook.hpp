#pragma once
#include"../../deps/minhook/MinHook.h"

namespace nation
{
    class hook{
    public:
        hook(void* target, void** original);
        auto intialize() -> void;
        auto create_hook(void* detour) -> bool;
        auto remove_hooks() -> void;
    private:
        void* target;
        void** original;
    };
}
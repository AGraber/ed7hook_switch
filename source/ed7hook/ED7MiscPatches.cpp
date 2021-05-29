#include "skyline/inlinehook/And64InlineHook.hpp"

#include "ed7hook/ED7Main.hpp"
#include "ed7hook/ED7Pointers.hpp"
#include "ed7hook/ED7Utils.hpp"
#include "ed7hook/ED7Debug.hpp"

// These patches enable CPU boost on some loading zones
// On normal circumstances it's almost unnoticeable
// but I've noticed this helps with custom image decoding

extern "C" {
    void _ZN2nn2oe15SetCpuBoostModeENS0_12CpuBoostModeE(int);
}

bool LoadingBattle1 = false;
bool LoadingBattle2 = false;

int64_t (*LoadBattle_original)(int64_t a1);
int64_t (LoadBattle_hook)(int64_t a1)
{
    _ZN2nn2oe15SetCpuBoostModeENS0_12CpuBoostModeE(1);
    LoadingBattle1 = true;

    int64_t ret = LoadBattle_original(a1);
    return ret;
}

int64_t (*LoadBattle2_original)(int64_t a1, int64_t a2);
int64_t (LoadBattle2_hook)(int64_t a1, int64_t a2)
{
    int64_t ret = LoadBattle2_original(a1, a2);
    if(LoadingBattle1)
    {
        LoadingBattle1 = false;
        if(!ret)
        {
            return ret;
        }

        LoadingBattle2 = true;
        _ZN2nn2oe15SetCpuBoostModeENS0_12CpuBoostModeE(0);
    }
    return ret;
}

int64_t (*LoadBattle3_original)(int64_t a1);
int64_t (LoadBattle3_hook)(int64_t a1)
{
    int64_t ret = LoadBattle3_original(a1);
    if(LoadingBattle2)
    {
        LoadingBattle2 = false;
        _ZN2nn2oe15SetCpuBoostModeENS0_12CpuBoostModeE(0);
    }
    return ret;
}

int64_t (*LoadMap_original)(int64_t a1, int64_t a2);
int64_t (LoadMap_hook)(int64_t a1, int64_t a2)
{
    _ZN2nn2oe15SetCpuBoostModeENS0_12CpuBoostModeE(1);
    int64_t ret = LoadMap_original(a1, a2);
    _ZN2nn2oe15SetCpuBoostModeENS0_12CpuBoostModeE(0);
    return ret;
}

void ED7MiscPatchesInitialize()
{
    MAKE_HOOK(LoadBattle);
    MAKE_HOOK(LoadBattle2);
    MAKE_HOOK(LoadBattle3);
    MAKE_HOOK(LoadMap);
}

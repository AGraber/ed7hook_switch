#include "skyline/inlinehook/And64InlineHook.hpp"

#include "ed7hook/ED7Main.hpp"
#include "ed7hook/ED7Pointers.hpp"
#include "ed7hook/ED7Utils.hpp"
#include "ed7hook/ED7Debug.hpp"

#include "nn/oe.h"

// These patches enable CPU boost on some loading zones
// On normal circumstances it's almost unnoticeable
// but I've noticed this helps with custom image decoding

bool LoadingBattle1 = false;
bool LoadingBattle2 = false;

int64_t (*LoadBattle_original)(int64_t a1);
int64_t (LoadBattle_hook)(int64_t a1)
{
    nn::oe::SetCpuBoostMode(nn::oe::CpuBoostMode::BoostCPU);
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
        nn::oe::SetCpuBoostMode(nn::oe::CpuBoostMode::Disabled);
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
        nn::oe::SetCpuBoostMode(nn::oe::CpuBoostMode::Disabled);
    }
    return ret;
}

int64_t (*LoadMap_original)(int64_t a1, int64_t a2);
int64_t (LoadMap_hook)(int64_t a1, int64_t a2)
{
    nn::oe::SetCpuBoostMode(nn::oe::CpuBoostMode::BoostCPU);
    int64_t ret = LoadMap_original(a1, a2);
    nn::oe::SetCpuBoostMode(nn::oe::CpuBoostMode::Disabled);
    return ret;
}

// GPU boost on low FPS
int Counter = -1;
uint8_t FPS = 0xFF;
float systemtickfrequency = 19200000;
int PerformanceConfig_GPU307mhz = 0x00020003;
int PerformanceConfig_GPU384mhz = 0x00020004;
int PerformanceConfig_GPU460mhz = 0x92220008;

int64_t (*RenderTexture_original)(int64_t a1, int64_t a2, unsigned int a3);
int64_t (RenderTexture_hook)(int64_t a1, int64_t a2, unsigned int a3)
{
    if(FPS <= 57){
        nn::oe::SetPerformanceConfiguration(nn::oe::PerformanceMode::Normal, PerformanceConfig_GPU460mhz);
        Counter = 0;
    }
    if(Counter > -1){
        Counter += 1;
    }
    if(Counter == 450){
        nn::oe::SetPerformanceConfiguration(nn::oe::PerformanceMode::Normal, PerformanceConfig_GPU384mhz);
    }
    if(Counter >= 1200 && FPS >= 59){
        nn::oe::SetPerformanceConfiguration(nn::oe::PerformanceMode::Normal, PerformanceConfig_GPU307mhz);
        Counter = -1;
    }
    return RenderTexture_original(a1, a2, a3);
}

// Adapted from NX-FPS
void (*nvnQueuePresentTexture_original)(void* unk1, void* unk2, void* unk3);
void nvnQueuePresentTexture_hook(void* unk1, void* unk2, void* unk3) {
	static uint8_t FPS_temp = 0;
	static uint64_t starttick = 0;
	static uint64_t endtick = 0;
	static uint64_t deltatick = 0;

	if (starttick == 0) starttick = nn::os::GetSystemTick();
	nvnQueuePresentTexture_original(unk1, unk2, unk3);
	endtick = nn::os::GetSystemTick();
	
	FPS_temp++;
	deltatick = endtick - starttick;
	if (deltatick >= 19200000) {
		starttick = nn::os::GetSystemTick();
		FPS = FPS_temp - 1;
		FPS_temp = 0;
	}
	
	return;
}

// Attempt to set anisotropic filtering to 16.0 always
void (*nvnSamplerBuilderSetMaxAnisotropy_original)(intptr_t sampleBuilder, float anisotropyLevel);
void nvnSamplerBuilderSetMaxAnisotropy_hook(intptr_t sampleBuilder, float anisotropyLevel)
{
    nvnSamplerBuilderSetMaxAnisotropy_original(sampleBuilder, 16.0);
}

void (*nvnLoadCProcs_original)(intptr_t device, intptr_t nvnDeviceGetProcAddress);
void nvnLoadCProcs_hook(intptr_t device, intptr_t nvnDeviceGetProcAddress)
{
    nvnLoadCProcs_original(device, nvnDeviceGetProcAddress);

    *(void**)&nvnSamplerBuilderSetMaxAnisotropy_original = *(void**)ED7Pointers.pfnc_nvnSamplerBuilderSetMaxAnisotropy;
    *(void**)ED7Pointers.pfnc_nvnSamplerBuilderSetMaxAnisotropy = (void*)nvnSamplerBuilderSetMaxAnisotropy_hook;

    *(void**)&nvnQueuePresentTexture_original = *(void**)ED7Pointers.pfnc_nvnQueuePresentTexture;
    *(void**)ED7Pointers.pfnc_nvnQueuePresentTexture = (void*)nvnQueuePresentTexture_hook;
}

void ED7MiscPatchesInitialize()
{
    MAKE_HOOK(nvnLoadCProcs);
    MAKE_HOOK(LoadBattle);
    MAKE_HOOK(LoadBattle2);
    MAKE_HOOK(LoadBattle3);
    MAKE_HOOK(LoadMap);
    MAKE_HOOK(RenderTexture);
}

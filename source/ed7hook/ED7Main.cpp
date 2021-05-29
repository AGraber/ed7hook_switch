/**
 * 
 * "ED7Hook"
 * Patches Trails from Zero/Zero no Kiseki to allow
 * gameplay in English or Japanese
 * 
 */

#include <string.h>
#include "types.h"
#include "nn/settings.h"
#include "skyline/inlinehook/And64InlineHook.hpp"
#include "ed7hook/ED7Main.hpp"
#include "ed7hook/ED7Pointers.hpp"
#include "ed7hook/ED7ExeText.hpp"
#include "ed7hook/ED7FileLoader.hpp"
#include "ed7hook/ED7ImgLoader.hpp"
#include "ed7hook/ED7VWFontFix.hpp"
#include "ed7hook/ED7MiscPatches.hpp"
#include "ed7hook/ED7Utils.hpp"
#include "ed7hook/ED7Debug.hpp"

ED7HookLanguage ED7HookCurrentLanguage;

/**
 * Late entry point for when the game has already initialized some stuff for us
 */
void (*LateInit_original)();
void LateInit_hook()
{
    LateInit_original();
    
    ED7DebugInitialize();
    ED7ExeTextInitialize();
    ED7FileLoaderInitialize();
    ED7ImgLoaderInitialize();
    ED7VWFontFixInitialize();
    ED7MiscPatchesInitialize();
}

/**
 * Entry point for ED7Hook
 */
void ed7hook_main()
{
    // Get current language
    nn::settings::LanguageCode lang;
    nn::settings::GetLanguageCode(&lang);

    if(strcmp(lang.code, "ja") == 0)
    {
        ED7HookCurrentLanguage = ED7HookLanguage::Japanese;
    }
    // Default to Traditional Chinese if any chinese locale is found
    else if(strcmp(lang.code, "zh-CN") == 0 || strcmp(lang.code, "zh-TW") == 0 || strcmp(lang.code, "zh-Hans") == 0 || strcmp(lang.code, "zh-Hant") == 0)
    {
        ED7HookCurrentLanguage = ED7HookLanguage::TraditionalChinese;
    }
    else if(strcmp(lang.code, "ko") == 0)
    {
        ED7HookCurrentLanguage = ED7HookLanguage::Korean;
    }
    // Default to English if nothing else matches matches
    else
    {
        ED7HookCurrentLanguage = ED7HookLanguage::English;
    }

    if(!ED7PointersInitialize())
    {
        // Couldn't detect version
        // Stop any further patching
        return;
    }

    MAKE_HOOK(LateInit);
}

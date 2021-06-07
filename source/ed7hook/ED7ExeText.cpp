#include <stdint.h>
#include <stdarg.h>
#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/inlinehook/memcpy_controlled.hpp"
#include "nn/oe.h"
#include "ed7hook/ED7Main.hpp"
#include "ed7hook/ED7Pointers.hpp"
#include "ed7hook/ED7Utils.hpp"
#include "ed7hook/ED7ExeText_ZeroContents.hpp"
#include "ed7hook/ED7ExeText_AzureContents.hpp"

/**
 * These patches force the game to load the game
 * with Shift-JIS encoding, used both by Japanese
 * and English (alongside UTF-8).
 */

static uint64_t (*SetGameLanguage_original)(uint64_t result);
static uint64_t SetGameLanguage_hook(uint64_t result)
{
    return SetGameLanguage_original(0);
    *(int32_t*)(ED7Pointers.iCurrentLanguage1) = 0;
    *(int32_t*)(ED7Pointers.iCurrentLanguage2) = 0;
}

static int64_t (*GetConsoleDesiredLanguage1_original)(uint64_t a1, uint64_t a2);
static int64_t GetConsoleDesiredLanguage1_hook(uint64_t a1, uint64_t a2)
{
    int64_t ret = GetConsoleDesiredLanguage1_original(a1, a2);
    if(ret == 0)
    {
        return 0;
    }

    *(int32_t*)(a1 + 464) = 0;
    *(int32_t*)(a1 + 468) = 0;
    return 1;
}

static int64_t (*GetConsoleDesiredLanguage2_original)();
static int64_t GetConsoleDesiredLanguage2_hook()
{
    return 0;
}

/**
 * To display text from the executable in multiple
 * languages, the game holds an array which contains
 * each string 3 times, for JP/TC/KR.
 * 
 * Fortunately, this is easy to patch, as the only
 * access to this array is through a single pointer
 * to this array, which is used by every function,
 * rather than every function holding a direct pointer
 * to this array. We can replace it with own array with english
 * text.
 * 
 * This uses the ED7ExeText_[game]_Contents.hpp header generated
 * by a js script. This was done like this because the
 * array contains text in multiple encodings that are
 * expected by some functions. Japanese uses Shift-JIS and
 * UTF-8.
 */

constexpr size_t zero_string_count = sizeof(ZeroExeStrings) / sizeof(ZeroExeStrings[0]);
constexpr size_t zero_array_slot_count = sizeof(ZeroExeStrings[0]);
static_assert(zero_string_count == 1484, "zero_string_count must be 1484");

constexpr size_t azure_string_count = sizeof(AzureExeStrings) / sizeof(AzureExeStrings[0]);
constexpr size_t azure_array_slot_count = sizeof(AzureExeStrings[0]);
static_assert(azure_string_count == 3551, "azure_string_count must be 3551");

// Big enough for both games
static const char* NewTranslationTable[azure_string_count * 3];

template<std::size_t size>
static void SetupEnglishTranslationTable(const char* (&translation_array)[size])
{

    for(size_t i = 0; i != size; ++i)
    {
        for(int j = 0; j != 3; ++j)
        {
            NewTranslationTable[ (i * 3) + j ] = translation_array[i];
        }
    }

    skyline::inlinehook::ControlledPages control(ED7Pointers.TranslationArray, 1 * sizeof(char*));
    control.claim();
    *(const char***)control.rw = NewTranslationTable;
    control.unclaim();
}

// Additional strings that aren't translated between Asian languages
// Mostly some "engrish" stuff that should be sligthly changed
// Full credit to zakaria for finding these out and writing the hooks

static uint64_t (*AdditionalStrings1_original)(uint64_t a1, const char *a2, uint64_t a3, uint64_t a4);
static uint64_t AdditionalStrings1_hook(uint64_t a1, const char *a2, uint64_t a3, uint64_t a4)
{
    if(a2 == ED7Pointers.String_EXP){
        return AdditionalStrings1_original(a1, "Exp:", a3, a4);
    }
    else if(a2 == ED7Pointers.String_Resist){
        return AdditionalStrings1_original(a1, "Resistances:", a3, a4);
    }
    else if(a2 == ED7Pointers.String_Item){
        return AdditionalStrings1_original(a1, "Items:", a3, a4);
    }
    return AdditionalStrings1_original(a1, a2, a3, a4);
}

static uint64_t (*AdditionalStrings2_original)(uint64_t a1, const char *a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7, unsigned int a8);
static uint64_t AdditionalStrings2_hook(uint64_t a1, const char *a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7, unsigned int a8)
{
    if(a2 == ED7Pointers.String_EXP){
        return AdditionalStrings2_original(a1, "Exp:", a3, a4, a5, a6, a7, a8);
    }
    else if(a2 == ED7Pointers.String_Resist){
        return AdditionalStrings2_original(a1, "Resistances:", a3, a4, a5, a6, a7, a8);
    }
    else if(a2 == ED7Pointers.String_Item){
        return AdditionalStrings2_original(a1, "Items:", a3, a4, a5, a6, a7, a8);
    }
    return AdditionalStrings2_original(a1, a2, a3, a4, a5, a6, a7, a8);
}

static uint64_t (*AdditionalStrings3_original)(uint64_t a1, const char *a2, int a3, int a4, char a5, float a6, float a7);
static uint64_t AdditionalStrings3_hook(uint64_t a1, const char *a2, int a3, int a4, char a5, float a6, float a7)
{
    if(a2 == ED7Pointers.String_Support_Member){
        return AdditionalStrings3_original(a1, "Support Members", a3, a4, a5, a6, a7);
    }
    if(a2 == ED7Pointers.String_Attack_Member){
        return AdditionalStrings3_original(a1, "Attack Members", a3, a4, a5, a6, a7);
    }
    return AdditionalStrings3_original(a1, a2, a3, a4, a5, a6, a7);
}

static uint64_t AdditionalStrings4_original(char *a1, const char *a2, ...);
static uint64_t AdditionalStrings4_hook(char *a1, const char *a2, ...)
{
    va_list args;
    va_start(args, a2);
    if(a2 == ED7Pointers.String_PlayTime1){
        int ret = vsprintf(a1, "%3d:%02d:%02d", args);
        va_end(args);
        return ret;
    }
    else if(a2 == ED7Pointers.String_PlayTime2){
        int ret = vsprintf(a1, "PlayTime: %2d:%02d:%02d\n", args);
        va_end(args);
        return ret;
    }
    int ret = vsprintf(a1, a2, args);
    va_end(args);
    return ret;
}

// Fix for status page percentage symbols
static int64_t (*CED6Window__SetText_original)(int64_t a1, const char* a2);
static int64_t CED6Window__SetText_hook(int64_t a1, const char* a2)
{
    static std::string followedBy(") \\.\r\n\t/,&");
    auto text = std::string(a2);
    size_t pos = 0;
    while (true)
    {
        pos = text.find('%', pos);
        if (pos == std::string::npos) break;

        pos++;
        if (text.find_first_of(followedBy, pos) != std::string::npos)
        {
            text.insert(pos, "%");
            pos++;
        }
    }
    return CED6Window__SetText_original(a1, text.c_str());
}

void ED7ExeTextInitialize()
{
    switch(ED7HookCurrentLanguage)
    {
        case ED7HookLanguage::English:
            if(ED7Pointers.IsZero)
            {
                SetupEnglishTranslationTable(ZeroExeStrings);
            }
            else
            {
                SetupEnglishTranslationTable(AzureExeStrings);
            }

            MAKE_HOOK(CED6Window__SetText);

            MAKE_HOOK(AdditionalStrings1);
            MAKE_HOOK(AdditionalStrings2);
            MAKE_HOOK(AdditionalStrings3);
            MAKE_HOOK(AdditionalStrings4);

            // fall through japanese because we also need
            // some japanese patches like sjis encoding, etc.

        case ED7HookLanguage::Japanese:
            // set japanese language
            MAKE_HOOK(SetGameLanguage);
            MAKE_HOOK(GetConsoleDesiredLanguage1);
            MAKE_HOOK(GetConsoleDesiredLanguage2);
            break;

        default:
            return;
    }
}
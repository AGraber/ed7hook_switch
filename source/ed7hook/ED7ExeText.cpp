#include <stdint.h>
#include <stdarg.h>
#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/inlinehook/memcpy_controlled.hpp"
#include "skyline/utils/cpputils.hpp"
#include "nn/oe.h"
#include "ed7hook/ED7Main.hpp"
#include "ed7hook/ED7Pointers.hpp"
#include "ed7hook/ED7Utils.hpp"
#include "ed7hook/ED7VWFontFix.hpp"
#include "ed7hook/ED7Debug.hpp"
#include "nlohmann/json.hpp"

// iconv provided by NintendoSDK from the game, nice
#include <iconv.h>

iconv_t hIcd;

void InitializeUtf8SjisIconv()
{
    hIcd = iconv_open("Shift_JIS", "UTF-8");
}

void FinalizeUtf8SjisIconv()
{
    iconv_close(hIcd);
}

char* ConvertUtf8ToSjis(const char* pszUtf8Text_const)
{
    char* pszUtf8Text = (char*)pszUtf8Text_const;
    char szSjisBuffer[256];
    size_t iSjisBufferSize = sizeof(szSjisBuffer);
    char* pszOutBuffer = szSjisBuffer;
    size_t iUtf8TextLen = strlen(pszUtf8Text);

    iconv(hIcd, &pszUtf8Text, &iUtf8TextLen, &pszOutBuffer, &iSjisBufferSize);

    *pszOutBuffer = 0;

    return strdup(szSjisBuffer);
}

/**
 * These patches force the game to load the game
 * with Shift-JIS encoding, used both by Japanese
 * and English (alongside UTF-8).
 */

static uint64_t (*SetGameLanguage_original)(uint64_t result);
static uint64_t SetGameLanguage_hook(uint64_t result)
{
    *(int32_t*)(ED7Pointers.iCurrentLanguage1) = 0;
    *(int32_t*)(ED7Pointers.iCurrentLanguage2) = 0;
    return SetGameLanguage_original(0);
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

constexpr size_t azure_string_count = 3551;

// Big enough for both games
static const char* NewTranslationTable[azure_string_count * 3];

void ED7ExeText_SetupTranslationTable()
{
    if(ED7HookCurrentLanguage == ED7HookLanguage::English)
    {
        InitializeUtf8SjisIconv();

        char* pszJsonBuffer;
        
        // open file and yolo it without error checking - patch wouldn't work if it errors while reading these files anyways
        nn::fs::FileHandle hFile;
        nn::fs::OpenFile(&hFile, "rom:/data_patch/exe_strings.json", nn::fs::OpenMode_Read);

        s64 iSize;
        nn::fs::GetFileSize(&iSize, hFile);
        
        pszJsonBuffer = (char*)malloc(iSize + 1);
        nn::fs::ReadFile(hFile, 0, pszJsonBuffer, iSize);
        pszJsonBuffer[iSize] = 0;

        nn::fs::CloseFile(hFile);

        auto jsonExeStrings = nlohmann::json::parse(pszJsonBuffer);

        free(pszJsonBuffer);

        for(auto& [strKey, jsonValue] : jsonExeStrings.items())
        {
            int iKey = atoi(strKey.c_str()); // std::stoi does some errno stuff and errno isn't available so it crashes, use atoi instead

            auto jsonString = jsonValue["en"];
            if(!jsonString.is_string())
            {
                jsonString = jsonValue["line"];
            }

            std::string& strExeString = jsonString.get_ref<std::string&>();
            char* pszFinalString;

            if(jsonValue["jis"].get<bool>())
            {
                pszFinalString = ConvertUtf8ToSjis(strExeString.c_str());
            }
            else
            {
                pszFinalString = strdup(strExeString.c_str());
            }

            for(int i = 0; i != 3; ++i)
            {
                NewTranslationTable[ (iKey * 3) + i ] = pszFinalString;
            }
        }

        skyline::inlinehook::ControlledPages control(ED7Pointers.TranslationArray, 1 * sizeof(char*));
        control.claim();
        *(const char***)control.rw = NewTranslationTable;
        control.unclaim();

        FinalizeUtf8SjisIconv();
    }
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

static uint64_t (*AdditionalStrings2_original)(float a1, float a2, float a3, float a4, float a5, __int64 a6, const char *a7, int a8, int a9, int a10, int a11, unsigned int a12, int a13);
static uint64_t AdditionalStrings2_hook(float a1, float a2, float a3, float a4, float a5, __int64 a6, const char *a7, int a8, int a9, int a10, int a11, unsigned int a12, int a13)
{
    if(a7 == ED7Pointers.String_EXP){
        a7 = "Exp:";
    }
    else if(a7 == ED7Pointers.String_Resist){
        a7 = "Resistances:";
    }
    else if(a7 == ED7Pointers.String_Item){
        a7 = "Items:";
    }
    else {
        void* return_address = __builtin_return_address(0);
        if(return_address == ED7Pointers.AdditionalStrings2_MonsterNameRet){
            auto pszTextLoop = a7;
            float fTextWidth = 0.0;
            while(char character = *pszTextLoop++)
            {
                if(IsSJISCharMultibyte(character))
                {
                    fTextWidth += pMultibyteAdvance[character][*pszTextLoop++];
                }
                else
                {
                    fTextWidth += pFontAdvanceTable[character];
                }
            }

            if(fTextWidth > 10.125)
            {
                a5 = 0.9;
            }
        }
    }
    return AdditionalStrings2_original(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
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
template <bool IsZero>
static uint64_t AdditionalStrings4_hook(char *a1, const char *a2, ...)
{
    va_list args;
    va_start(args, a2);
    if(a2 == ED7Pointers.String_PlayTime1){
        a2 = "%3d:%02d:%02d";
    }
    else if(a2 == ED7Pointers.String_PlayTime2){
        a2 = "PlayTime: %2d:%02d:%02d\n";
    }
    else if constexpr(!IsZero) {
        if(a2 == ED7Pointers.String_CarColorFormatting){
            a2 = "\x81\x40%s";
        }
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
            // Not needed in Azure
            if(ED7Pointers.IsZero)
            {
                MAKE_HOOK(CED6Window__SetText);
            }

            MAKE_HOOK(AdditionalStrings1);
            MAKE_HOOK(AdditionalStrings2);
            MAKE_HOOK(AdditionalStrings3);
            MAKE_HOOK_T(AdditionalStrings4);

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

#pragma once

#include <stdint.h>

struct ED7Pointers_Struct
{
    // false: Trails to Azure
    bool IsZero;

    const char* GameName = nullptr;

    // Internal exported function, we can use this to
    // compare it and get what game are we running
    uintptr_t NnInitStartup;

    // Offset to .text region of main executable
    uintptr_t TextRegionOffset;

    // Early init function, but late enough so that the NintendoSDK stuff is already initialized
    void* LateInit;

    // Pointers to functions related to ITP decoding/loading/dumping
    void* CTexMgr__Load;
    void* CTexMgr__Load2;
    void* CTexBase__LoadITP_chunk_IDAT;

    // Array that holds localized strings
    void* TranslationArray;

    // Dialog boxes variable width support hooks
    void* CMessageWindow__PrintText;
    void* CMessageWindow__AnonymousText;
    void* GetItemName;

    // Disable renderer space width override
    void* FontRendererSpaceCheck;

    // Percentage sign fix
    void* CED6Window__SetText;

    // CSafeFile functions
    void* CSafeFile__fopen;
    void* CSafeFile__CSafeFile;
    void* CSafeFile__fclose;
    void* CSafeFileBase__destructor;

    // CFontMgr2
    void* CFontMgr2__GetSizeSJIS;
    void* CFontMgr2__SetFixMode;

    // Book font rendering
    void* BookDrawText;
    void* BookIsKana;
    void* BookGetFontWidth;

    // Notebook font rendering space check fix
    void* NotebookSpaceCheckJump;
    short NotebookSpaceCheckNewOffset;

    // CPU Boost patches
    void* LoadBattle;
    void* LoadBattle2;
    void* LoadBattle3;
    void* LoadMap;

    // Functions that are hooked to replace text
    void* AdditionalStrings1;
    void* AdditionalStrings2;
    void* AdditionalStrings3;
    void* AdditionalStrings4;

    // Internal game language stuff
    void* SetGameLanguage;
    void* GetConsoleDesiredLanguage1;
    void* GetConsoleDesiredLanguage2;

    // Some global variables holding the current language
    void* iCurrentLanguage1;
    void* iCurrentLanguage2;

    // Faster string comparison through constant address
    void* String_PlayTime1; // "%3d:%2d:%2d"
    void* String_PlayTime2; // "PlayTime %2d:%2d:%2d\n"
    void* String_EXP; // "EXP:"
    void* String_Resist; // "Resist:"
    void* String_Item; // "Item:""
    void* String_Support_Member; // "Support Member"
    void* String_Attack_Member; // "Attack Member"

    // These are not used anymore, but are left for reference
    // I used to have them for debugging/dumping purposes
    // But they are not needed for playing the game

    void* CNode__Load;
    void* CTexMgr__UnLoad;
    void* CMessageWindow__PrintText2;
    void* CSafeFile__fseek;

    void* CFontMgr2__DrawFontSJIS;

    void* CFontMgr2__GetSizeMOJICODE;
    void* CFontMgr2__ConvertUTF8toMOJICODE;

    void* GetLanguageForLanguageCode;
};

extern ED7Pointers_Struct ED7Pointers;
bool ED7PointersInitialize();

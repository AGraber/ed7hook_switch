#include "ed7hook/ED7Pointers.hpp"
#include "skyline/utils/cpputils.hpp"
#include "nn/nn.h"

ED7Pointers_Struct ED7Pointers; 

#define EXE_PTR(ptr) (void*)( (uintptr_t)TextRegionOffset + ptr)

bool ED7PointersInitialize()
{
    uintptr_t TextRegionOffset = (uintptr_t)skyline::utils::getRegionAddress(skyline::utils::region::Text);
    uint64_t startup_address = (uint64_t)nninitStartup - (uint64_t)TextRegionOffset;

    ED7Pointers.NnInitStartup = startup_address;
    ED7Pointers.TextRegionOffset = TextRegionOffset;
    switch(startup_address)
    {
        case 0x5f010: // Zero 1.0 - not supported anymore - left for reference
            ED7Pointers.GameName = "Trails from Zero 1.0.0";
            ED7Pointers.IsZero = true;

            ED7Pointers.LateInit = EXE_PTR(0x5E9E0);

            ED7Pointers.CTexMgr__Load = EXE_PTR(0x7850);
            ED7Pointers.CTexMgr__Load2 = EXE_PTR(0x7370);
            ED7Pointers.CTexBase__LoadITP_chunk_IDAT = EXE_PTR(0x62C40);
            
            ED7Pointers.TranslationArray = EXE_PTR(0x3DC568);
            
            ED7Pointers.CMessageWindow__PrintText = EXE_PTR(0x11E190);
            ED7Pointers.CMessageWindow__AnonymousText = EXE_PTR(0x11F4C0);
            ED7Pointers.GetItemName = EXE_PTR(0x171F80);

            ED7Pointers.FontRendererSpaceCheck = EXE_PTR(0x117910);

            ED7Pointers.CED6Window__SetText = EXE_PTR(0x17FD80);

            ED7Pointers.CSafeFile__fopen = EXE_PTR(0x3A5E0);
            ED7Pointers.CSafeFile__CSafeFile = EXE_PTR(0x3C120);
            ED7Pointers.CSafeFile__fclose = EXE_PTR(0x3BAE0);
            ED7Pointers.CSafeFileBase__destructor = EXE_PTR(0x41340);

            ED7Pointers.CFontMgr2__GetSizeSJIS = EXE_PTR(0x43EA0);
            ED7Pointers.CFontMgr2__SetFixMode = EXE_PTR(0x43B00);

            ED7Pointers.BookDrawText = EXE_PTR(0x11B9C0);
            ED7Pointers.BookIsKana = EXE_PTR(0x6E0C0);
            ED7Pointers.BookGetFontWidth = EXE_PTR(0x11FD30);

            ED7Pointers.NotebookSpaceCheckJump = EXE_PTR(0x344134);
            ED7Pointers.NotebookSpaceCheckNewOffset = 0xC7;

            ED7Pointers.LoadBattle = EXE_PTR(0x75EC0);
            ED7Pointers.LoadBattle2 = EXE_PTR(0x72D10);
            ED7Pointers.LoadBattle3 = EXE_PTR(0x7BBE0);
            ED7Pointers.LoadMap = EXE_PTR(0x1F04F0);

            ED7Pointers.AdditionalStrings1 = EXE_PTR(0x47630);
            ED7Pointers.AdditionalStrings2 = EXE_PTR(0x16F820);
            ED7Pointers.AdditionalStrings3 = EXE_PTR(0x17DDC0);
            ED7Pointers.AdditionalStrings4 = EXE_PTR(0x25CA00);

            ED7Pointers.SetGameLanguage = EXE_PTR(0x5B3F0);
            ED7Pointers.GetConsoleDesiredLanguage1 = EXE_PTR(0x5F3E0);
            ED7Pointers.GetConsoleDesiredLanguage2 = EXE_PTR(0x5F440);

            ED7Pointers.iCurrentLanguage1 = EXE_PTR(0x44D5A8);
            ED7Pointers.iCurrentLanguage2 = EXE_PTR(0x44D9AC);

            ED7Pointers.SaveListFontRender = EXE_PTR(0x4D070);

            ED7Pointers.String_PlayTime1 = EXE_PTR(0x338DB3);
            ED7Pointers.String_PlayTime2 = EXE_PTR(0x2F651C);
            ED7Pointers.String_EXP = EXE_PTR(0x2FDC26);
            ED7Pointers.String_Resist = EXE_PTR(0x314278);
            ED7Pointers.String_Item = EXE_PTR(0x32CAAC);
            ED7Pointers.String_Support_Member = EXE_PTR(0x2FDC95);
            ED7Pointers.String_Attack_Member = EXE_PTR(0x32CACC);

            ED7Pointers.nvnLoadCProcs = EXE_PTR(0x210B60);
            ED7Pointers.pfnc_nvnSamplerBuilderSetMaxAnisotropy = EXE_PTR(0x5609EF30);

            // Optional/Unused
            ED7Pointers.CNode__Load = EXE_PTR(0x17640);
            ED7Pointers.CTexMgr__UnLoad = EXE_PTR(0x1C50);
            ED7Pointers.CMessageWindow__PrintText2 = EXE_PTR(0xFA0E0);
            ED7Pointers.CSafeFile__fseek = EXE_PTR(0x3C1C0);
            ED7Pointers.CFontMgr2__DrawFontSJIS = EXE_PTR(0x46B70);
            ED7Pointers.GetLanguageForLanguageCode = EXE_PTR(0x5F480);
            break;

        case 0x5f020: // Zero 1.0.1
            ED7Pointers.GameName = "Trails from Zero 1.0.1";
            ED7Pointers.IsZero = true;

            ED7Pointers.LateInit = EXE_PTR(0x5E9F0);

            ED7Pointers.CTexMgr__Load = EXE_PTR(0x7850);
            ED7Pointers.CTexMgr__Load2 = EXE_PTR(0x7370);
            ED7Pointers.CTexBase__LoadITP_chunk_IDAT = EXE_PTR(0x62C50);

            ED7Pointers.TranslationArray = EXE_PTR(0x3DD570);

            ED7Pointers.CMessageWindow__PrintText = EXE_PTR(0x11E1C0);
            ED7Pointers.CMessageWindow__AnonymousText = EXE_PTR(0x11F4F0);
            ED7Pointers.GetItemName = EXE_PTR(0x171FB0);

            ED7Pointers.FontRendererSpaceCheck = EXE_PTR(0x117940);

            ED7Pointers.CED6Window__SetText = EXE_PTR(0x17FDB0);

            ED7Pointers.CSafeFile__fopen = EXE_PTR(0x3A5E0);
            ED7Pointers.CSafeFile__CSafeFile = EXE_PTR(0x3C120);
            ED7Pointers.CSafeFile__fclose = EXE_PTR(0x3BAE0);
            ED7Pointers.CSafeFileBase__destructor = EXE_PTR(0x41340);

            ED7Pointers.CFontMgr2__GetSizeSJIS = EXE_PTR(0x43EA0);
            ED7Pointers.CFontMgr2__SetFixMode = EXE_PTR(0x43B00);

            ED7Pointers.BookDrawText = EXE_PTR(0x11B9F0);
            ED7Pointers.BookIsKana = EXE_PTR(0x6E0F0);
            ED7Pointers.BookGetFontWidth = EXE_PTR(0x11FD60);

            ED7Pointers.NotebookSpaceCheckJump = EXE_PTR(0x345154);
            ED7Pointers.NotebookSpaceCheckNewOffset = 0xC7;

            ED7Pointers.LoadBattle = EXE_PTR(0x75EF0);
            ED7Pointers.LoadBattle2 = EXE_PTR(0x72D40);
            ED7Pointers.LoadBattle3 = EXE_PTR(0x7BC10);
            ED7Pointers.LoadMap = EXE_PTR(0x1F0810);

            ED7Pointers.AdditionalStrings1 = EXE_PTR(0x47630);
            ED7Pointers.AdditionalStrings2 = EXE_PTR(0x16F850);
            ED7Pointers.AdditionalStrings3 = EXE_PTR(0x17DDF0);
            ED7Pointers.AdditionalStrings4 = EXE_PTR(0x25CD40);

            ED7Pointers.SetGameLanguage = EXE_PTR(0x5B400);
            ED7Pointers.GetConsoleDesiredLanguage1 = EXE_PTR(0x5F3F0);
            ED7Pointers.GetConsoleDesiredLanguage2 = EXE_PTR(0x5F450);

            ED7Pointers.iCurrentLanguage1 = EXE_PTR(0x44E5A8);
            ED7Pointers.iCurrentLanguage2 = EXE_PTR(0x44E9AC);

            ED7Pointers.SaveListFontRender = EXE_PTR(0x4D070);
            ED7Pointers.SaveListFontRender_ReturnAfterDescription = EXE_PTR(0x4D57C);

            ED7Pointers.NotebookDrawText = EXE_PTR(0x1812A0);
            ED7Pointers.NotebookDrawText_ReturnAfterMonstElemQuestionMark = EXE_PTR(0x18DC90);
            ED7Pointers.NotebookDrawText_ReturnAfterCompletedRequestsStr = EXE_PTR(0x1984F0);

            ED7Pointers.NotebookElementBarDraw = EXE_PTR(0x129E80);
            ED7Pointers.NotebookElementBarDraw_ReturnBackground = EXE_PTR(0x18DB94);
            ED7Pointers.NotebookElementBarDraw_ReturnBar = EXE_PTR(0x18DC38);

            ED7Pointers.NotebookDrawNumberRightAligned = EXE_PTR(0x182150);
            ED7Pointers.NotebookDrawNumberRightAligned_ReturnAfterElemEff = EXE_PTR(0x18DC68);

            ED7Pointers.BattleTutorialSBreakBlockedOffsetX = EXE_PTR(0xDD9C8);
            ED7Pointers.BattleTutorialSBreakBlockedOffsetY = EXE_PTR(0xDD9E4);

            ED7Pointers.String_PlayTime1 = EXE_PTR(0x339E02);
            ED7Pointers.String_PlayTime2 = EXE_PTR(0x2F7532);
            ED7Pointers.String_EXP = EXE_PTR(0x02FEC3C);
            ED7Pointers.String_Resist = EXE_PTR(0x3152F3);
            ED7Pointers.String_Item = EXE_PTR(0x32DB33);
            ED7Pointers.String_Support_Member = EXE_PTR(0x2FECAB);
            ED7Pointers.String_Attack_Member = EXE_PTR(0x32DB53);

            ED7Pointers.nvnLoadCProcs = EXE_PTR(0x210EA0);
            ED7Pointers.pfnc_nvnSamplerBuilderSetMaxAnisotropy = EXE_PTR(0x5609FF30);

            // Optional/Unused
            ED7Pointers.CNode__Load = EXE_PTR(0x17640);
            ED7Pointers.CTexMgr__UnLoad = EXE_PTR(0x1C50);
            ED7Pointers.CSafeFile__fseek = EXE_PTR(0x3C1C0);
            ED7Pointers.CFontMgr2__DrawFontSJIS = EXE_PTR(0x46B70);
            ED7Pointers.CFontMgr2__GetSizeMOJICODE = EXE_PTR(0x43BE0);
            ED7Pointers.CFontMgr2__ConvertUTF8toMOJICODE = EXE_PTR(0x443F0);
            ED7Pointers.GetLanguageForLanguageCode = EXE_PTR(0x5F490);
            break;

        // Azure is still unfinished

        case 0x5f200: // Azure 1.0.0
            ED7Pointers.GameName = "Trails to Azure 1.0.0";
            ED7Pointers.IsZero = false;
            ED7Pointers.CTexMgr__Load = EXE_PTR(0x7850);
            ED7Pointers.CTexMgr__Load2 = EXE_PTR(0x7370);
            ED7Pointers.CTexBase__LoadITP_chunk_IDAT = EXE_PTR(0x62AD0);
            ED7Pointers.CSafeFile__fopen = EXE_PTR(0x3A320);
            ED7Pointers.SetGameLanguage = EXE_PTR(0x5B450);
            ED7Pointers.TranslationArray = EXE_PTR(0x4A1588);
            ED7Pointers.CMessageWindow__PrintText = EXE_PTR(0x125FE0);
            ED7Pointers.GetItemName = EXE_PTR(0x180930);

            ED7Pointers.LoadBattle = EXE_PTR(0x766C0);
            ED7Pointers.LoadBattle2 = EXE_PTR(0x736F0);
            ED7Pointers.LoadBattle3 = EXE_PTR(0x7C710);
            ED7Pointers.LoadMap = EXE_PTR(0x21D260);

            ED7Pointers.AdditionalStrings1 = EXE_PTR(0x47380);
            ED7Pointers.AdditionalStrings2 = EXE_PTR(0x17DC10);
            ED7Pointers.AdditionalStrings3 = EXE_PTR(0x18C450);

            ED7Pointers.LateInit = EXE_PTR(0x5EBE0);

            ED7Pointers.GetConsoleDesiredLanguage1 = EXE_PTR(0x5F5D0);
            ED7Pointers.GetConsoleDesiredLanguage2 = EXE_PTR(0x5F630);

            ED7Pointers.iCurrentLanguage1 = EXE_PTR(0x5125A8);
            ED7Pointers.iCurrentLanguage2 = EXE_PTR(0x5129AC);

            // Optional/Unused
            ED7Pointers.CNode__Load = EXE_PTR(0x17560);
            ED7Pointers.CTexMgr__UnLoad = EXE_PTR(0x1C50);
            ED7Pointers.GetLanguageForLanguageCode = EXE_PTR(0x5F670);
            break;


        default:
            R_ERRORONFAIL(startup_address);
            return false;
    }

    return true;
}

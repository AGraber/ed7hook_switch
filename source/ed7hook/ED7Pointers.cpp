#include "ed7hook/ED7Pointers.hpp"
#include "skyline/utils/cpputils.hpp"
#include "nn/nn.h"

ED7Pointers_Struct ED7Pointers{};

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

            ED7Pointers.CTexMgr__Load2 = EXE_PTR(0x7370);
            ED7Pointers.CTexBase__LoadITP_chunk_IDAT = EXE_PTR(0x62C40);
            
            ED7Pointers.TranslationArray = EXE_PTR(0x3DC568);
            
            ED7Pointers.CMessageWindow__PrintText = EXE_PTR(0x11E190);
            ED7Pointers.CMessageWindow__AnonymousText = EXE_PTR(0x11F4C0);
            ED7Pointers.GetItemName = EXE_PTR(0x171F80);

            ED7Pointers.FontRendererSpaceCheck = EXE_PTR(0x117910);

            ED7Pointers.CED6Window__SetText = EXE_PTR(0x17FD80);

            ED7Pointers.CSafeFile__CSafeFile = EXE_PTR(0x3C120);
            ED7Pointers.CSafeFileBase__destructor = EXE_PTR(0x41340);
            ED7Pointers.CSafeFile__fopen = EXE_PTR(0x3A5E0);
            ED7Pointers.CSafeFile__fclose = EXE_PTR(0x3BAE0);

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

            ED7Pointers.RenderTexture = EXE_PTR(0x215040);

            ED7Pointers.nvnLoadCProcs = EXE_PTR(0x210B60);
            ED7Pointers.pfnc_nvnSamplerBuilderSetMaxAnisotropy = EXE_PTR(0x5609EF30);
            ED7Pointers.pfnc_nvnQueuePresentTexture = EXE_PTR(0x5609E980);

            // Optional/Unused
            ED7Pointers.CNode__Load = EXE_PTR(0x17640);
            ED7Pointers.CTexMgr__Load = EXE_PTR(0x7850);
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

            ED7Pointers.CTexMgr__Load2 = EXE_PTR(0x7370);
            ED7Pointers.CTexBase__LoadITP_chunk_IDAT = EXE_PTR(0x62C50);
            ED7Pointers.CTexMgr__LoadITP = EXE_PTR(0x64600);

            ED7Pointers.TranslationArray = EXE_PTR(0x3DD570);

            ED7Pointers.CMessageWindow__PrintText = EXE_PTR(0x11E1C0);
            ED7Pointers.CMessageWindow__AnonymousText = EXE_PTR(0x11F4F0);
            ED7Pointers.GetItemName = EXE_PTR(0x171FB0);

            ED7Pointers.FontRendererSpaceCheck = EXE_PTR(0x117940);

            ED7Pointers.CED6Window__SetText = EXE_PTR(0x17FDB0);

            ED7Pointers.CSafeFile__CSafeFile = EXE_PTR(0x3C120);
            ED7Pointers.CSafeFileBase__destructor = EXE_PTR(0x41340);
            ED7Pointers.CSafeFile__fopen = EXE_PTR(0x3A5E0);
            ED7Pointers.CSafeFile__fclose = EXE_PTR(0x3BAE0);
            ED7Pointers.CSafeFile__fread = EXE_PTR(0x3B7A0);
            ED7Pointers.CSafeFile__fseek = EXE_PTR(0x3C1C0);
            ED7Pointers.CSafeFile__ftel = EXE_PTR(0x3CB10);
            ED7Pointers.CSafeFile__CheckOnMem = EXE_PTR(0x3CA30);
            ED7Pointers.CSafeFile__GetSize = EXE_PTR(0x3CA70);
            ED7Pointers.CSafeFile__LoadCCMem = EXE_PTR(0x3EE40);

            ED7Pointers.CSafeFileBase__open_FileNameCheck = EXE_PTR(0x415B8);

            ED7Pointers.CPU__NewBack = EXE_PTR(0x357C0);

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

            ED7Pointers.SaveOpcodePtrFixInfo = EXE_PTR(0x345814);

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

            ED7Pointers.ExtraMode_RightColumnOffsetX = EXE_PTR(0x1C07A0);

            ED7Pointers.CFontMgr__DrawFontClip = EXE_PTR(0x47580);

            ED7Pointers.SupportRequestDrawFont = EXE_PTR(0x17D990);
            ED7Pointers.SupportRequestDrawFont_ReturnAddr = EXE_PTR(0x19D294);

            ED7Pointers.String_PlayTime1 = EXE_PTR(0x339E02);
            ED7Pointers.String_PlayTime2 = EXE_PTR(0x2F7532);
            ED7Pointers.String_EXP = EXE_PTR(0x02FEC3C);
            ED7Pointers.String_Resist = EXE_PTR(0x3152F3);
            ED7Pointers.String_Item = EXE_PTR(0x32DB33);
            ED7Pointers.String_Support_Member = EXE_PTR(0x2FECAB);
            ED7Pointers.String_Attack_Member = EXE_PTR(0x32DB53);

            ED7Pointers.RenderTexture = EXE_PTR(0x215380);

            ED7Pointers.nvnLoadCProcs = EXE_PTR(0x210EA0);
            ED7Pointers.pfnc_nvnSamplerBuilderSetMaxAnisotropy = EXE_PTR(0x5609FF30);
            ED7Pointers.pfnc_nvnQueuePresentTexture = EXE_PTR(0x5609F980);

            // Optional/Unused
            ED7Pointers.CNode__Load = EXE_PTR(0x17640);
            ED7Pointers.CTexMgr__Load = EXE_PTR(0x7850);
            ED7Pointers.CTexMgr__UnLoad = EXE_PTR(0x1C50);
            ED7Pointers.CFontMgr2__DrawFontSJIS = EXE_PTR(0x46B70);
            ED7Pointers.CFontMgr2__GetSizeMOJICODE = EXE_PTR(0x43BE0);
            ED7Pointers.CFontMgr2__ConvertUTF8toMOJICODE = EXE_PTR(0x443F0);
            ED7Pointers.GetLanguageForLanguageCode = EXE_PTR(0x5F490);
            ED7Pointers.CSafeFile__Init = EXE_PTR(0x3FE80);
            break;

        // Azure is still unfinished

        case 0x5f200: // Azure 1.0.0 / 1.0.1
            ED7Pointers.GameName = "Trails to Azure 1.0.0-1.0.1";
            ED7Pointers.IsZero = false;

            ED7Pointers.LateInit = EXE_PTR(0x5EBE0);

            ED7Pointers.CTexMgr__Load2 = EXE_PTR(0x7370);
            ED7Pointers.CTexBase__LoadITP_chunk_IDAT = EXE_PTR(0x62AD0);
            ED7Pointers.CTexMgr__LoadITP = EXE_PTR(0x64480);

            ED7Pointers.TranslationArray = EXE_PTR(0x4A1588);

            ED7Pointers.CMessageWindow__PrintText = EXE_PTR(0x125FE0);
            ED7Pointers.CMessageWindow__AnonymousText = EXE_PTR(0x127140);
            ED7Pointers.GetItemName = EXE_PTR(0x180930);

            ED7Pointers.FontRendererSpaceCheck = EXE_PTR(0x1219E8);

            // CED6Window_SetText hook not needed in Azure

            ED7Pointers.CSafeFile__CSafeFile = EXE_PTR(0x3BE60);
            ED7Pointers.CSafeFileBase__destructor = EXE_PTR(0x410A0);
            ED7Pointers.CSafeFile__fopen = EXE_PTR(0x3A320);
            ED7Pointers.CSafeFile__fclose = EXE_PTR(0x3B820);
            ED7Pointers.CSafeFile__fread = EXE_PTR(0x3B4E0);
            ED7Pointers.CSafeFile__fseek = EXE_PTR(0x3BF00);
            ED7Pointers.CSafeFile__ftel = EXE_PTR(0x3C850);
            ED7Pointers.CSafeFile__CheckOnMem = EXE_PTR(0x3C770);
            ED7Pointers.CSafeFile__GetSize = EXE_PTR(0x3C7B0);
            ED7Pointers.CSafeFile__LoadCCMem = EXE_PTR(0x3EB80);

            ED7Pointers.CSafeFileBase__open_FileNameCheck = EXE_PTR(0x41318);

            ED7Pointers.CPU__NewBack = EXE_PTR(0x354E0);

            ED7Pointers.CFontMgr2__GetSizeSJIS = EXE_PTR(0x43C00);
            ED7Pointers.CFontMgr2__SetFixMode = EXE_PTR(0x43860);

            ED7Pointers.BookDrawText = EXE_PTR(0x123960);
            ED7Pointers.BookIsKana = EXE_PTR(0x6DCB0);
            ED7Pointers.BookGetFontWidth = EXE_PTR(0x127CB0);

            ED7Pointers.NotebookSpaceCheckJump = EXE_PTR(0x3EDE60);
            ED7Pointers.NotebookSpaceCheckNewOffset = 0xFA;

            ED7Pointers.NotebookSpaceCheckJump2 = EXE_PTR(0x3EDDF5);
            ED7Pointers.NotebookSpaceCheckNewOffset2 = 0x45;

            ED7Pointers.LoadBattle = EXE_PTR(0x766C0);
            ED7Pointers.LoadBattle2 = EXE_PTR(0x736F0);
            ED7Pointers.LoadBattle3 = EXE_PTR(0x7C710);
            ED7Pointers.LoadMap = EXE_PTR(0x21D260);

            ED7Pointers.AdditionalStrings1 = EXE_PTR(0x47380);
            ED7Pointers.AdditionalStrings2 = EXE_PTR(0x17DC10);
            ED7Pointers.AdditionalStrings3 = EXE_PTR(0x18C450);
            ED7Pointers.AdditionalStrings4 = EXE_PTR(0x28A960);

            ED7Pointers.SetGameLanguage = EXE_PTR(0x5B450);
            ED7Pointers.GetConsoleDesiredLanguage1 = EXE_PTR(0x5F5D0);
            ED7Pointers.GetConsoleDesiredLanguage2 = EXE_PTR(0x5F630);

            ED7Pointers.iCurrentLanguage1 = EXE_PTR(0x5125A8);
            ED7Pointers.iCurrentLanguage2 = EXE_PTR(0x5129AC);

            ED7Pointers.SaveListFontRender = EXE_PTR(0x4CEE0);
            ED7Pointers.SaveListFontRender_ReturnAfterDescription = EXE_PTR(0x4D3EC);

            ED7Pointers.SaveOpcodePtrFixInfo = EXE_PTR(0x3EE4A0);

            ED7Pointers.NotebookDrawText = EXE_PTR(0x18F480);
            ED7Pointers.NotebookDrawText_ReturnAfterMonstElemQuestionMark = EXE_PTR(0x19C37C);
            ED7Pointers.NotebookDrawText_ReturnAfterCompletedRequestsStr = EXE_PTR(0x1A695C);

            ED7Pointers.NotebookDrawText_ReturnPrestorySynopsis = EXE_PTR(0x1CC0DC);
            ED7Pointers.NotebookDrawText_ReturnPrestoryTerminology = EXE_PTR(0x1CC16C);
            ED7Pointers.NotebookDrawText_ReturnPrestoryCharacters = EXE_PTR(0x1CC1E0);

            ED7Pointers.NotebookElementBarDraw = EXE_PTR(0x136140);
            ED7Pointers.NotebookElementBarDraw_ReturnBackground = EXE_PTR(0x19C290);
            ED7Pointers.NotebookElementBarDraw_ReturnBar = EXE_PTR(0x19C32C);

            ED7Pointers.NotebookDrawNumberRightAligned = EXE_PTR(0x190920);
            ED7Pointers.NotebookDrawNumberRightAligned_ReturnAfterElemEff = EXE_PTR(0x19C354);

            ED7Pointers.BattleTutorialSBreakBlockedOffsetX = EXE_PTR(0xE39BC);
            ED7Pointers.BattleTutorialSBreakBlockedOffsetY = EXE_PTR(0xE39D8);

            ED7Pointers.ExtraMode_RightColumnOffsetX = EXE_PTR(0x1D5410);

            ED7Pointers.SetStoryText = EXE_PTR(0x1CB870);
            ED7Pointers.SetStoryText_RetTerminology1 = EXE_PTR(0x1CA890);
            ED7Pointers.SetStoryText_RetTerminology2 = EXE_PTR(0x1CA468);
            ED7Pointers.SetStoryText_RetCharacter1 = EXE_PTR(0x1CA7EC);
            ED7Pointers.SetStoryText_RetCharacter2 = EXE_PTR(0x1CA5CC);

            ED7Pointers.OrbmentChangeViewTextXOffset1 = EXE_PTR(0x109038);
            ED7Pointers.OrbmentChangeViewTextXOffset2 = EXE_PTR(0x1C7CC0);
            ED7Pointers.OrbmentChangeViewTextXOffset3 = EXE_PTR(0x1C7D94);
            ED7Pointers.OrbmentChangeViewTextXOffset4 = EXE_PTR(0x1C7DD8);

            ED7Pointers.CFontMgr__DrawFontClip = EXE_PTR(0x472E0);

            ED7Pointers.DrawFontReport = EXE_PTR(0x18C450);
            ED7Pointers.DrawFontReport_ReturnDPAmount = EXE_PTR(0x1AA7B0);
            ED7Pointers.DrawFontReport_ReturnDetectiveRank = EXE_PTR(0x1AA8F4);

            ED7Pointers.SupportRequestDrawFont = EXE_PTR(0x18BFF0);
            ED7Pointers.SupportRequestDrawFont_ReturnAddr = EXE_PTR(0x1AB8F0);

            ED7Pointers.RecipeBookRecipesOwnedNumberOffsetX = EXE_PTR(0x12C778);
            ED7Pointers.FishingBookSpeciesCaughtNumberOffsetX = EXE_PTR(0x163D34);

            ED7Pointers.String_PlayTime1 = EXE_PTR(0x3DD672);
            ED7Pointers.String_PlayTime2 = EXE_PTR(0x36E721);
            ED7Pointers.String_EXP = EXE_PTR(0x37A72B);
            ED7Pointers.String_Resist = EXE_PTR(0x39FE0E);
            ED7Pointers.String_Item = EXE_PTR(0x3C92C6);
            ED7Pointers.String_Support_Member = EXE_PTR(0x37A789);
            ED7Pointers.String_Attack_Member = EXE_PTR(0x3C92D9);

            ED7Pointers.RenderTexture = EXE_PTR(0x242FA0);

            ED7Pointers.nvnLoadCProcs = EXE_PTR(0x23EAC0);
            ED7Pointers.pfnc_nvnSamplerBuilderSetMaxAnisotropy = EXE_PTR(0x62963F50);
            ED7Pointers.pfnc_nvnQueuePresentTexture = EXE_PTR(0x629639A0);

            // Optional/Unused
            ED7Pointers.CNode__Load = EXE_PTR(0x17560);
            ED7Pointers.CTexMgr__Load = EXE_PTR(0x7850);
            ED7Pointers.CTexMgr__UnLoad = EXE_PTR(0x1C50);
            ED7Pointers.CFontMgr2__DrawFontSJIS = EXE_PTR(0x468D0);
            ED7Pointers.GetLanguageForLanguageCode = EXE_PTR(0x5F670);
            break;

        default:
            R_ERRORONFAIL(startup_address);
            return false;
    }

    return true;
}

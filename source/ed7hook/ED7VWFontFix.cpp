#include <stdint.h>

#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/inlinehook/memcpy_controlled.hpp"
#include "skyline/utils/cpputils.hpp"
#include "ed7hook/ED7VWFontFix.hpp"
#include "ed7hook/ED7Main.hpp"
#include "ed7hook/ED7Pointers.hpp"
#include "ed7hook/ED7Debug.hpp"
#include "ed7hook/ED7Utils.hpp"
#include "ed7hook/ED7ExeText.hpp"
#include "ed7hook/ED7MiscPatches.hpp"

/**
 * This file mostly consists of carefully placed hooks that
 * either override width values or feed the game with
 * width values according to the font.
 * 
 * The game's font renderer is actually the same renderer
 * as in more modern games (Ys 8, Cold Steel series), and
 * it's completely variable-width-font aware. Unfortunately,
 * the code that uses this font renderer remains unchanged
 * and assumes monospace. These hooks will override what
 * those functions are using and use the original font
 * renderer instead.
 */

float FontAdvanceTable[256 - 32];
float MultibyteAdvance[(0xEF - 0x81) + 1][256];

struct CMessageWindow
{
  char pad[56];
  float *pfWidthStuff;
};

// Game functions
const char* (*GetItemName)(int64_t _this, int itemIndex);
float (*CFontMgr2__GetSizeSJIS)(int64_t* a1, const char *s, float a3, float *a4, float *a5, float *, float);

template<bool IsZero>
const char* GetItemName_Offset(CMessageWindow* _this, int itemIndex)
{
    return GetItemName(*(int64_t *)&_this->pad[24] + (IsZero ? 696544 : 711744), itemIndex);
}

// Cache all the font advance values on initialization
// on our own buffers for fast access.
// Also need to do this because the game later likes
// to return garbage for these.
void InitializeFontData(int64_t* this_)
{
    for(char i = 32; i < 255; ++i) {
        if(IsSJISCharMultibyte(i)) {
            continue;
        }

        char szTextForCharacterWidth[2] = {i, 0};
        pFontAdvanceTable[i] = CFontMgr2__GetSizeSJIS(this_, szTextForCharacterWidth, 1.0, NULL, NULL, NULL, 1.0);
    }

    // get from 0x81 to 0x9F
    for(char i = 0x81; i <= 0x9F; ++i)
    {
        for(char j = 0; j < 0xFF; ++j)
        {
            char szTextForCharacterWidth[3] = {i, j, 0};
            pMultibyteAdvance[i][j] = CFontMgr2__GetSizeSJIS(this_, szTextForCharacterWidth, 1.0, NULL, NULL, NULL, 1.0);
        }
    }

    // get from 0xE0 to 0xEF
    for(char i = 0xE0; i <= 0xEF; ++i)
    {
        for(char j = 0; j < 0xFF; ++j)
        {
            char szTextForCharacterWidth[3] = {i, j, 0};
            pMultibyteAdvance[i][j] = CFontMgr2__GetSizeSJIS(this_, szTextForCharacterWidth, 1.0, NULL, NULL, NULL, 1.0);
        }
    }

    // romfs already mounted, so call these
    ED7ExeText_SetupTranslationTable();
    ED7MiscPatches_SetupSaveOffsets();
}

inline float GetSJISCharWidth(char byte1, char byte2, float width)
{
    return pMultibyteAdvance[byte1][byte2] * width;
}

// This function is run shortly after initializing the CFontMgr2 class
int64_t (*CFontMgr2__SetFixMode_original)(int64_t* this_, int fixMode);
int64_t CFontMgr2__SetFixMode_hook(int64_t* this_, int fixMode)
{
    int64_t ret = CFontMgr2__SetFixMode_original(this_, fixMode);
    InitializeFontData(this_);
    return ret;
}

// Calculate width for a CMessageWindow dialog box's contents
template<bool IsZero>
short GetWidthForDialogBoxContents(CMessageWindow *_this, const char* text, float currentTextWidth)
{
    float longestWidth = 0.0, currentWidth = 0.0;

    while(true)
    {
        char character = *text;
        switch(character)
        {
            case 0x1F: // Insert item name
            {
                const char* itemName = GetItemName_Offset<IsZero>(_this, *(unsigned short*)(&text[1]));
                while(char itemCharacter = *itemName++)
                {
                    if(IsSJISCharMultibyte(itemCharacter))
                    {
                        currentWidth += GetSJISCharWidth(itemCharacter, *itemName++, currentTextWidth);
                    }
                    else
                    {
                        currentWidth += pFontAdvanceTable[itemCharacter] * currentTextWidth;
                    }
                }

                // Insert space for item icon
                currentWidth += currentTextWidth;

                text += 3;
                continue;
            }

            case 0x06: // not quite sure what this is
                ++text;
                continue;

            case 0x07: // Change color
                text += 2;
                continue;

            case '#':
                ++text;
                while(*text >= '0' && *text <= '9')
                {
                    ++text;
                }

                if(*text == 'S') // Font size
                {
                    constexpr auto vtable_offset = IsZero ? 336LL : 352LL;
                    currentTextWidth =
                        ((*(int (**)(int64_t *, int64_t, int64_t))(*(int64_t*)(_this) + vtable_offset))(
                               (int64_t*)(_this),
                               (text[-1] - 48),
                               *(unsigned int *)( ((int64_t*)(_this))[7] + 88LL))) / 2.0;

                }
                else if(*text == 'I') // Show sprite
                {
                    currentWidth += currentTextWidth;
                }
                ++text;
                continue;

            default:
                if(character < 32)
                {
                    if(currentWidth > longestWidth)
                    {
                        longestWidth = currentWidth;
                    }

                    if(character == 0) {
                        return (short)longestWidth;
                    }

                    currentWidth = 0.0;
                    ++text;
                }
                else
                {
                    if(IsSJISCharMultibyte(character))
                    {
                        currentWidth += GetSJISCharWidth(character, text[1], currentTextWidth);
                        text += 2;
                    }
                    else
                    {
                        currentWidth += pFontAdvanceTable[character] * currentTextWidth;
                        ++text;
                    }
                }
                continue;
        }
    }
}

bool CorrectFixedWidthValue = false;
short OriginalFixedWidth;

// Inserts custom width values on CMessageWindow
char* (*CMessageWindow__PrintText_original)(CMessageWindow *_this, char *pszWindowContent, int64_t a3, int a4);
template<bool IsZero>
char* CMessageWindow__PrintText_hook(CMessageWindow *_this, char *pszWindowContent, int64_t a3, int a4)
{
    if(CorrectFixedWidthValue)
    {
        auto v37 = *((int64_t *)_this + 7);
        auto v38 = *(float *)(v37 + 104);
        auto v39 = *(int32_t *)(v37 + 388);
        int result = v38 + v39;
        auto ret = CMessageWindow__PrintText_original(_this, pszWindowContent, a3, a4);

        // kai does some weird scaling multiplying by 0.84

        // We will replicate PC, which does this:
        // this->ced6window0.menusub->gap_FC = a5;
        // this->ced6window0.menusub->field_112 = Src->ced6window0.menusub->gap_FC * (v42 / 2);
        
        // we need to divide by half and then an extra * 1.08 because of Kai dalog box scaling
        // being smaller than usual

        *(int16_t *)(*((int64_t *)_this + 7) + 358LL) = OriginalFixedWidth;
        *(int16_t *)(*((int64_t *)_this + 7) + 382) = (OriginalFixedWidth * result * 0.5) * 1.13;
        return ret;
    }

    auto ret = CMessageWindow__PrintText_original(_this, pszWindowContent, a3, a4);

    float nameCharacterWidth = _this->pfWidthStuff[26] > 10.0 ? _this->pfWidthStuff[26] : 10.0;
    auto nameWidth = GetWidthForDialogBoxContents<IsZero>(_this, (char*)( ((int64_t*)(_this))[20] + 1080), nameCharacterWidth) + pFontAdvanceTable[' '] * nameCharacterWidth;
    if(nameWidth < (nameCharacterWidth * 4) ) // Set a minimum in case the dialog box is too small
    {
        nameWidth = (nameCharacterWidth * 4);
    }

    auto textBoxWidth = GetWidthForDialogBoxContents<IsZero>(_this, pszWindowContent, _this->pfWidthStuff[26]) + pFontAdvanceTable[' '] * _this->pfWidthStuff[26];

    // Override width
    *(int16_t *)(*((int64_t *)_this + 7) + 382) = nameWidth > textBoxWidth ? nameWidth : textBoxWidth;

    return ret;
}


// If fixed width value is found, set global variable so CMessageWindow::PrintText hook can handle it
char* (*CMessageWindow__AnonymousText_original)(CMessageWindow *a1, uint8_t *a2, int iWidthOverride, int a4, char *pszDialogueText, int a6, float a7, float a8);
char* CMessageWindow__AnonymousText_hook(CMessageWindow *a1, uint8_t *a2, int iWidthOverride, int a4, char *pszDialogueText, int a6, float a7, float a8)
{
    if(iWidthOverride != 0 && iWidthOverride != -1)
    {
        CorrectFixedWidthValue = true;
        OriginalFixedWidth = iWidthOverride;
    }
    else
    {
        if(iWidthOverride == -1)
        {
            int32_t* v25 = (int32_t *)*((int64_t *)a1 + 20);
            if(v25)
            {
                if(v25[285] != 0 && v25[285] != 0xFFFF && v25[285] != -1)
                {
                    CorrectFixedWidthValue = true;
                    OriginalFixedWidth = v25[285];
                    v25[285] = 0xFFFF;
                }
            }
        }
    }

    auto ret = CMessageWindow__AnonymousText_original(a1, a2, -1, a4, pszDialogueText, a6, a7, a8);
    CorrectFixedWidthValue = false;
    return ret;
}

bool DrawingBookText = false;
bool IsKanaBookCheckDone = false;
bool IsKanaBookCheckResult;
char IsKanaBookCheckCharacter;

bool GetWidthDoneOnce = false;

void (*BookDrawText_original)(int *a1, int a2, float a3, float a4, float a5, int a6, int a7, int a8);
void BookDrawText_hook(int *a1, int a2, float a3, float a4, float a5, int a6, int a7, int a8)
{
    DrawingBookText = true;
    BookDrawText_original(a1, a2, a3, a4, a5, a6, a7, a8);
    DrawingBookText = false;
    IsKanaBookCheckDone = false;
}

bool (*BookIsKana_original)(char character);
bool BookIsKana_hook(char character)
{
    if(DrawingBookText)
    {
        IsKanaBookCheckDone = true;
        GetWidthDoneOnce = false;
        IsKanaBookCheckCharacter = character;
        return IsKanaBookCheckResult = BookIsKana_original(character);
    }

    return BookIsKana_original(character);
}

float (*BookGetFontWidth_original)(int64_t a1, int64_t iFontType);
float BookGetFontWidth_hook(int64_t a1, int64_t iFontType)
{
    float result;
    if(DrawingBookText && IsKanaBookCheckDone)
    {
        IsKanaBookCheckDone = false;
        if(IsKanaBookCheckResult)
        {
            result = BookGetFontWidth_original(a1, iFontType);
        }
        else
        {
            result = BookGetFontWidth_original(a1, iFontType);
            // If it's multibyte, just let the default monospace as it's probably some JP character
            // otherwise we get it from the font advance table
            if(!IsSJISCharMultibyte(IsKanaBookCheckCharacter))
            {
                result = pFontAdvanceTable[IsKanaBookCheckCharacter] * (result * 2);
            }
        }

        if(GetWidthDoneOnce)
        {
            IsKanaBookCheckDone = false;
            GetWidthDoneOnce = false;
        }
        else
        {
            GetWidthDoneOnce = true;
        }
    }
    else
    {
        result = BookGetFontWidth_original(a1, iFontType);
    }

    return result;
}

bool HookFontHorizontalScale = false;

void (*SaveListFontRender_original)(int64_t *this_, const char *pszText, int color, float posX, float posY, float fUnknown, float fUnknown2, float fUnknown3, float fUnknown4);
template<bool IsZero>
void SaveListFontRender_hook(int64_t *this_, const char *pszText, int color, float posX, float posY, float fUnknown, float fUnknown2, float fUnknown3, float fUnknown4)
{
    if(__builtin_return_address(0) == ED7Pointers.SaveListFontRender_ReturnAfterDescription)
    {
        // Align save description text to the right to prevent overflow
        auto pszTextLoop = pszText;
        float fCorrectedXOffset = 800.0;
        float fSubstractXOffset = 0.0;

        constexpr float fFontWidth = 24.0;
        while(char character = *pszTextLoop++)
        {
            if(IsSJISCharMultibyte(character))
            {
                fSubstractXOffset += pMultibyteAdvance[character][*pszTextLoop++] * fFontWidth;
            }
            else
            {
                fSubstractXOffset += pFontAdvanceTable[character] * fFontWidth;
            }
        }

        if constexpr(!IsZero) // not needed on zero
        {
            if(fSubstractXOffset > 380.0)
            {
                HookFontHorizontalScale = true;
                fSubstractXOffset *= 0.9;
            }
        }


        fCorrectedXOffset -= fSubstractXOffset;

        SaveListFontRender_original(this_, pszText, color, fCorrectedXOffset, posY, fUnknown, fUnknown2, fUnknown3, fUnknown4);
    }
    else
    {
        SaveListFontRender_original(this_, pszText, color, posX, posY, fUnknown, fUnknown2, fUnknown3, fUnknown4);
    }
}

int64_t (*CFontMgr__DrawFontClip_original)(int64_t* this_, int a2, int a3, char *a4, int a5, int a6, int a7, int a8, int a9, float a10, float a11, float a12, float a13, int a14);
int64_t CFontMgr__DrawFontClip_hook(int64_t* this_, int a2, int a3, char *a4, int a5, int a6, int a7, int a8, int a9, float a10, float a11, float a12, float a13, int a14)
{
    if(HookFontHorizontalScale)
    {
        a13 = 0.9;
        HookFontHorizontalScale = false;
    }

    return CFontMgr__DrawFontClip_original(this_, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}

void (*DrawFontReport_original)(int64_t *a1, const char *a2, int a3, int a4, char a5, float a6, float a7, float a8);
void DrawFontReport_hook(int64_t *a1, const char *a2, int a3, int a4, char a5, float a6, float a7, float a8)
{
    void* return_address = __builtin_return_address(0);
    if(return_address == ED7Pointers.DrawFontReport_ReturnDPAmount)
    {
        a6 += 4.0;
    }
    else if(return_address == ED7Pointers.DrawFontReport_ReturnDetectiveRank)
    {
        a6 += 4.0;
        a7 -= 2.0;
    }
    else if(return_address == ED7Pointers.DrawFontReport_ReturnPromotionReward) // override "centered" value
    {
        a6 = 232.0;
    }
    DrawFontReport_original(a1, a2, a3, a4, a5, a6, a7, a8);
}

int64_t (*NotebookDrawText_original)(int64_t *a1, const char *pszText, int a3, float fOffsetX, float fOffsetY, float a6, float a7);
template<bool IsZero>
int64_t NotebookDrawText_hook(int64_t *a1, const char *pszText, int a3, float fOffsetX, float fOffsetY, float a6, float a7)
{
    void* return_address = __builtin_return_address(0); // lmao
    if(return_address == ED7Pointers.NotebookDrawText_ReturnAfterMonstElemQuestionMark)
    {
        return NotebookDrawText_original(a1, pszText, a3, fOffsetX + 20.0, fOffsetY, a6, a7);
    }
    if(return_address == ED7Pointers.NotebookDrawText_ReturnAfterCompletedRequestsStr)
    {
        return NotebookDrawText_original(a1, pszText, a3, fOffsetX - 14.5, fOffsetY, a6, a7);
    }
    if constexpr(!IsZero)
    {
        if(return_address == ED7Pointers.NotebookDrawText_ReturnPrestorySynopsis) // Synopsis
        {
            return NotebookDrawText_original(a1, pszText, a3, 206.0, fOffsetY, a6, a7);
        }
        if(return_address == ED7Pointers.NotebookDrawText_ReturnPrestoryTerminology) // Terminology
        {
            return NotebookDrawText_original(a1, pszText, a3, 190.0, fOffsetY, a6, a7);
        }
        if(return_address == ED7Pointers.NotebookDrawText_ReturnPrestoryCharacters) // Characters
        {
            return NotebookDrawText_original(a1, pszText, a3, 197.0, fOffsetY, a6, a7);
        }
    }
    return NotebookDrawText_original(a1, pszText, a3, fOffsetX, fOffsetY, a6, a7);
}

int64_t (*NotebookElementBarDraw_original)(int64_t result, unsigned int a2, int a3, int a4, int a5, int a6, unsigned int iColor, unsigned int a8, float a9, float a10, float a11, float a12, float a13);
int64_t NotebookElementBarDraw_hook(int64_t result, unsigned int a2, int a3, int a4, int a5, int a6, unsigned int iColor, unsigned int a8, float a9, float a10, float a11, float a12, float a13)
{
    void* retAddr = __builtin_return_address(0);
    if(retAddr == ED7Pointers.NotebookElementBarDraw_ReturnBackground)
    {
        NotebookElementBarDraw_original(result, a2, a3, a4 + 11, a5, 5, iColor, a8, a9, 0.75 - (5.0 / 256.0), a11, a12, a13);
        return NotebookElementBarDraw_original(result, a2, a3 + 47 + 20, a4, 160 - 47 - 20, 16 - 5, iColor, a8, (47.0 / 256.0) + 0.25, a10, a11, 0.75 - (5.0 / 256.0), a13);
    }
    else if(retAddr == ED7Pointers.NotebookElementBarDraw_ReturnBar)
    {
        register unsigned int w28 asm("w28"); // lmfao

        unsigned int width = w28;
        return NotebookElementBarDraw_original(result, a2, a3 + 20, a4, width * 0.445, a6, iColor, a8, a9, a10, a11, a12, a13);
    }
    return NotebookElementBarDraw_original(result, a2, a3, a4, a5, a6, iColor, a8, a9, a10, a11, a12, a13);
}

int64_t (*NotebookDrawNumberRightAligned_original)(int64_t *a1, int a2, int a3, unsigned int a4, int a5, float a6, float a7, int64_t a8, int a9);
int64_t NotebookDrawNumberRightAligned_hook(int64_t *a1, int a2, int a3, unsigned int a4, int a5, float a6, float a7, int64_t a8, int a9)
{
    if(__builtin_return_address(0) == ED7Pointers.NotebookDrawNumberRightAligned_ReturnAfterElemEff)
    {
        return NotebookDrawNumberRightAligned_original(a1, a2 + 20, a3, a4, a5, a6, a7, a8, a9);
    }
    return NotebookDrawNumberRightAligned_original(a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

char* (*SetStoryText_original)(int64_t *a1, int16_t a2, int16_t a3, int a4, int a5, char *a6, int a7, char a8);
char* SetStoryText_hook(int64_t *a1, int16_t a2, int16_t a3, int a4, int a5, char *a6, int a7, char a8)
{
    char* ret = SetStoryText_original(a1, a2, a3, a4, a5, a6, a7, a8);
    void* retAddr = __builtin_return_address(0);
    if(retAddr == ED7Pointers.SetStoryText_RetTerminology1 || retAddr == ED7Pointers.SetStoryText_RetTerminology2) // gordaso 
    {
        *(int16_t *)(*((int64_t *)a1 + 7) + 382) = 400;
    }
    else if(retAddr == ED7Pointers.SetStoryText_RetCharacter1 || retAddr == ED7Pointers.SetStoryText_RetCharacter2) // personaje
    {
        *(int16_t *)(*((int64_t *)a1 + 7) + 382) = 230;
    }
    return ret;
}

void (*SupportRequestDrawFont_original)(int64_t *a1, int a2, int a3, int a4, int a5, char *s, int a7, float a8);
void SupportRequestDrawFont_hook(int64_t *a1, int a2, int a3, int a4, int a5, char *s, int a7, float a8)
{
    if(__builtin_return_address(0) == ED7Pointers.SupportRequestDrawFont_ReturnAddr)
    {
        a2 -= 10;
    }
    SupportRequestDrawFont_original(a1, a2, a3, a4, a5, s, a7, a8);
}

void ED7VWFontFixInitialize()
{
    *(void**)&GetItemName = ED7Pointers.GetItemName;
    *(void**)&CFontMgr2__GetSizeSJIS = ED7Pointers.CFontMgr2__GetSizeSJIS;


    if(ED7HookCurrentLanguage != ED7HookLanguage::English)
    {
        return;
    }

    MAKE_HOOK_T(CMessageWindow__PrintText);

    if(!ED7Pointers.IsZero)
    {
        MAKE_HOOK(SetStoryText);
        MAKE_HOOK(DrawFontReport);
        MAKE_HOOK(CFontMgr__DrawFontClip);
    }

    MAKE_HOOK(CFontMgr2__SetFixMode);
    MAKE_HOOK(BookDrawText);
    MAKE_HOOK(BookIsKana);
    MAKE_HOOK(BookGetFontWidth);
    MAKE_HOOK(CMessageWindow__AnonymousText);
    MAKE_HOOK_T(SaveListFontRender);
    MAKE_HOOK_T(NotebookDrawText);
    MAKE_HOOK(SupportRequestDrawFont);
    MAKE_HOOK(NotebookElementBarDraw);
    MAKE_HOOK(NotebookDrawNumberRightAligned);

    // Removes space check messing up font rendering on notebook
    skyline::inlinehook::ControlledPages control( (void*)(ED7Pointers.NotebookSpaceCheckJump), 1 * sizeof(short));
    control.claim();
    *(short*)control.rw = ED7Pointers.NotebookSpaceCheckNewOffset;
    control.unclaim();

    static constexpr unsigned char SpaceCheckInstructionReplace_w8[4] = {0x1F, 0x01, 0x00, 0x71}; // cmp w8, #0x00

    static constexpr unsigned char BattleTutorialSBreakBlockedOffsetX[4] = {0x34, 0x15, 0x80, 0x52}; // mov w20, #169
    static constexpr unsigned char BattleTutorialSBreakBlockedOffsetY[4] = {0x95, 0x03, 0x80, 0x52}; // mov w21, #28

    static constexpr unsigned char ExtraMode_RightColumnOffsetX[4] = {0x56, 0x03, 0x80, 0x52}; // mov w22, #26

    static constexpr unsigned char ShopPriceWindow_NotForSaleOffsetX[4] = {0x00, 0xB0, 0x26, 0x1E}; // FMOV S0, #21.0

    static constexpr unsigned char DetectiveNotebookRequestStatus_w26[4] = {0x48, 0x3F, 0x05, 0x11}; // ADD W8, W26, #335
    static constexpr unsigned char DetectiveNotebookRequestStatus_w25[4] = {0x28, 0x3F, 0x05, 0x11}; // ADD W8, W25, #335

    // Invalidate space check that changes spaces width to another value
    sky_memcpy(ED7Pointers.FontRendererSpaceCheck, SpaceCheckInstructionReplace_w8, 4);

    // Change instructions that load X/Y values for tutorial icons in registers
    sky_memcpy(ED7Pointers.BattleTutorialSBreakBlockedOffsetX, BattleTutorialSBreakBlockedOffsetX, 4);
    sky_memcpy(ED7Pointers.BattleTutorialSBreakBlockedOffsetY, BattleTutorialSBreakBlockedOffsetY, 4);

    if(!ED7Pointers.IsZero)
    {
        // Removes space check messing up font rendering on notebook, again for Azure, on this other function
        skyline::inlinehook::ControlledPages control( (void*)(ED7Pointers.NotebookSpaceCheckJump2), 1 * sizeof(char));
        control.claim();
        *(char*)control.rw = ED7Pointers.NotebookSpaceCheckNewOffset2;
        control.unclaim();

        static constexpr unsigned char OrbmentChangeViewTextXOffsetW1[4] = {0x01, 0x32, 0x80, 0x52}; // mov w1, #400
        static constexpr unsigned char OrbmentChangeViewTextXOffsetW9[4] = {0x09, 0x32, 0x80, 0x52}; // mov w9, #400

        sky_memcpy(ED7Pointers.OrbmentChangeViewTextXOffset1, OrbmentChangeViewTextXOffsetW1, 4);
        sky_memcpy(ED7Pointers.OrbmentChangeViewTextXOffset2, OrbmentChangeViewTextXOffsetW9, 4);
        sky_memcpy(ED7Pointers.OrbmentChangeViewTextXOffset3, OrbmentChangeViewTextXOffsetW1, 4);
        sky_memcpy(ED7Pointers.OrbmentChangeViewTextXOffset4, OrbmentChangeViewTextXOffsetW1, 4);

        // Recipe & fishing book
        static constexpr unsigned char RecipeBookRecipesOwnedNumberOffsetX[4] = {0x41, 0x83, 0x05, 0x11}; // add w1, w26, #352
        static constexpr unsigned char FishingBookSpeciesCaughtNumberOffsetX[4] = {0x41, 0x9B, 0x05, 0x11}; // add w1, w26, #358
        sky_memcpy(ED7Pointers.RecipeBookRecipesOwnedNumberOffsetX, RecipeBookRecipesOwnedNumberOffsetX, 4);
        sky_memcpy(ED7Pointers.FishingBookSpeciesCaughtNumberOffsetX, FishingBookSpeciesCaughtNumberOffsetX, 4);

    }

    // Change instruction that loads offset X for some Extra Mode Text
    sky_memcpy(ED7Pointers.ExtraMode_RightColumnOffsetX, ExtraMode_RightColumnOffsetX, 4);

    // Change X offset of "Not for Sale" text
    sky_memcpy(ED7Pointers.ShopPriceWindow_NotForSaleOffsetX, ShopPriceWindow_NotForSaleOffsetX, 4);

    // Change X offset of request status on detective notebook to prevent overlapping with long request names
    sky_memcpy(ED7Pointers.DetectiveNotebookRequestStatus, ED7Pointers.IsZero ? DetectiveNotebookRequestStatus_w25 : DetectiveNotebookRequestStatus_w26, 4);
}

#pragma once

void ED7VWFontFixInitialize();

extern float FontAdvanceTable[256 - 32];
extern float MultibyteAdvance[(0xEF - 0x81) + 1][256];

// fast direct access through character as index
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
constexpr float* pFontAdvanceTable = FontAdvanceTable - 32;
constexpr auto* pMultibyteAdvance = MultibyteAdvance - 0x81;
#pragma GCC diagnostic pop

inline bool IsSJISCharMultibyte(unsigned char character)
{
    return (character >= 0x81 && character <= 0x9F) || (character >= 0xE0 && character <= 0xEF);
}

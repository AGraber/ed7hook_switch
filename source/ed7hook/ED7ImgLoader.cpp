/**
 * Image loader for ED7
 *
 * Supports PNG and WebP - can easily be
 * extended to use other formats but these
 * are arguably the best already.
 * 
 * Image buffer replacement has to be done
 * in the middle of the ITP buffer replacement
 * and with a "donor" ITP. This is because
 * the actual process of getting textures
 * into nvn is inlined so I can't go and
 * just call some functions like in the PC
 * version.
 */

#include <stdint.h>
#include <time.h>

#include <sys/time.h>
#include <sys/resource.h>

#include "skyline/utils/cpputils.hpp"
#include "skyline/inlinehook/And64InlineHook.hpp"
#include "ed7hook/ED7Pointers.hpp"
#include "ed7hook/ED7ImgLoader.hpp"
#include "ed7hook/ED7Debug.hpp"
#include "ed7hook/ED7Utils.hpp"

#include "nn/oe.h"

#include "webp/decode.h"

#include "stb_image.h"
#include "stb_sprintf.h"

struct CSafeFile
{
    char pad[168];
};

enum TextureType
{
    Unknown,
    Png,
    Webp,
};

static bool AlreadyHookingIT3 = false; // Don't process ITPLoad hook if already hooked by CTexMgr::Load2

static unsigned char StaticFileBuffer[2048 * 2048]; // Buffer for loading files into
static size_t StaticFileBufferSize;
static TextureType StaticTextureType = TextureType::Unknown;

// CSafeFile constructor
static void (*CSafeFile__CSafeFile)(CSafeFile* this_);

// CSafeFile destructor
static void (*CSafeFileBase__destructor)(CSafeFile* this_);

// Open file
static int64_t (*CSafeFile__fopen)(CSafeFile *_this, const char *fileName, const char *mode, unsigned int a4, unsigned int a5, int64_t a6, unsigned int a7);

// Close a file
static void (*CSafeFile__fclose)(CSafeFile* this_);

// Read file contents
static void (*CSafeFile__fread)(CSafeFile* pSafeFile, void* pBuffer, long size, long count, int unk1, int unk2);

// Seek through the file - identical to POSIX fseek
static int64_t (*CSafeFile__fseek)(CSafeFile* pSafeFile, long offset, int origin);

// Get current position
static int64_t (*CSafeFile__ftel)(CSafeFile* pSafeFile);

// Check if a file has been cached on memory
static int64_t (*CSafeFile__CheckOnMem)(CSafeFile* pSafeFile);

// Get a file's size
static int64_t (*CSafeFile__GetSize)(CSafeFile* pSafeFile);

// Cache a file's contents on memory through its path and a provided buffer
// The buffer will be freed by the engine's file system eventually
static int64_t (*CSafeFile__LoadCCMem)(const char *pPath, void* pContentsBuffer, int iSize, char a4);

// Allocate memory using the game's memory allocator
static void* (*CPU__NewBack)(ulong iBytes, const char *pszMemLabel);

// Returns TextureType for a file header
static inline TextureType GetTextureTypeByHeader(unsigned int iHeader)
{
    switch(iHeader)
    {
        case 'GNP\x89':
            return TextureType::Png;
        case 'FFIR': // aggresive assumption - should work anyways
            return TextureType::Webp;
        default:
            return TextureType::Unknown;
    }
}

// Loads a texture from a WEBP file.
static inline bool LoadWebpTexture(unsigned char* pFileBuffer, size_t iFileSize, unsigned char* pBGRAOutBuffer, size_t iBGRABufPixelCount, int iWidth)
{
    return WebPDecodeBGRAInto(pFileBuffer, iFileSize, pBGRAOutBuffer, iBGRABufPixelCount * 4, iWidth * 4) != NULL;
}

// Loads a texture from a PNG file
static bool LoadPngTexture(unsigned char* pFileBuffer, size_t iFileSize, unsigned char* pBGRAOutBuffer, size_t iBGRABufPixelCount)
{
    int dummy;
    unsigned char* pngBuffer = stbi_load_from_memory(pFileBuffer, iFileSize, &dummy, &dummy, NULL, 4);

    if(!pngBuffer)
    {
        return false;
    }

    for(size_t i = 0; i != iBGRABufPixelCount; ++i) {
        unsigned int pixel = ((int*)(pngBuffer))[i];
        ((int*)(pBGRAOutBuffer))[i] = (pixel & 0xFF00FF00) | ( (pixel & 0x000000FF) << 16) | ( (pixel & 0x00FF0000) >> 16);
    }

    free(pngBuffer);

    return true;
}

// Load texture of a specific, already known type into a buffer
inline static bool LoadCustomTexture(unsigned char* pFileBuffer, size_t iFileSize, unsigned char* pBGRAOutBuffer, size_t iBGRABufPixelCount, TextureType eTextureType, int iWidth)
{
    switch(eTextureType)
    {
        case TextureType::Png:
            return LoadPngTexture(pFileBuffer, iFileSize, pBGRAOutBuffer, iBGRABufPixelCount);
        case TextureType::Webp:
            return LoadWebpTexture(pFileBuffer, iFileSize, pBGRAOutBuffer, iBGRABufPixelCount, iWidth);
        default:
            return false;
    }
}

static inline bool GetWebpTextureDimensions(unsigned char* pFileBuffer, size_t iFileSize, int* piOutWidth, int* piOutHeight)
{
    return WebPGetInfo(pFileBuffer, iFileSize, piOutWidth, piOutHeight);
}

static inline bool GetPngTextureDimensions(unsigned char* pFileBuffer, size_t iFileSize, int* piOutWidth, int* piOutHeight)
{
    return stbi_info_from_memory(pFileBuffer, iFileSize, piOutWidth, piOutHeight, NULL);
}

static bool GetCustomTextureDimensions(unsigned char* pFileBuffer, size_t iFileSize, TextureType eTexType, int* piOutWidth, int* piOutHeight)
{
    switch(eTexType)
    {
        case TextureType::Png:
            return GetPngTextureDimensions(pFileBuffer, iFileSize, piOutWidth, piOutHeight);
        case TextureType::Webp:
            return GetWebpTextureDimensions(pFileBuffer, iFileSize, piOutWidth, piOutHeight);
        default:
            return false;
    }
}

// Loads a custom texture into our buffer, for decoding later.
static bool InitCustomTextureFromFileName(char* pszFileName, unsigned char* pFileBuffer, int* piOutWidth, int* piOutHeight)
{
    nn::fs::FileHandle handle;
    if(R_FAILED(nn::fs::OpenFile(&handle, pszFileName, nn::fs::OpenMode_Read))) {
        return false;
    }

    size_t iFileSize;
    nn::fs::GetFileSize((s64*)&iFileSize, handle);
    nn::fs::ReadFile(handle, 0, pFileBuffer, iFileSize);
    nn::fs::CloseFile(handle);

    TextureType eTexType = GetTextureTypeByHeader(*(unsigned int*)pFileBuffer);
    if(eTexType == TextureType::Unknown)
    {
        return false;
    }

    GetCustomTextureDimensions(pFileBuffer, iFileSize, eTexType, piOutWidth, piOutHeight);

    StaticTextureType = eTexType;
    StaticFileBufferSize = iFileSize;
    return true;
}

// Get a cached in memory file from the engine
CSafeFile* GetCachedFile(const char* pszPath)
{
    CSafeFile* pSafeFile = new CSafeFile;
    CSafeFile__CSafeFile(pSafeFile);

    CSafeFile__fopen(pSafeFile, pszPath, "rb", 0, 0, 0, 0);
    if(!CSafeFile__CheckOnMem(pSafeFile))
    {
        int64_t iFileSize = CSafeFile__GetSize(pSafeFile);
        void* pBuffer = CPU__NewBack(iFileSize, "ed7hk_itp");
        CSafeFile__fread(pSafeFile, pBuffer, iFileSize, 1, 0, 0);
        CSafeFile__fclose(pSafeFile);

        CSafeFile__LoadCCMem(pszPath, pBuffer, iFileSize, 1);
        CSafeFile__fopen(pSafeFile, pszPath, "rb", 0, 0, 0, 0);
    }

    return pSafeFile;
}

int64_t (*CTexMgr__LoadITP_original)(unsigned int *this_, unsigned int iTextureId, CSafeFile *pFile, int64_t a4, int64_t a5, int64_t a6, unsigned int itpPixelFormatType);
int64_t CTexMgr__LoadITP_hook(unsigned int *this_, unsigned int iTextureId, CSafeFile *pFile, int64_t a4, int64_t a5, int64_t a6, unsigned int itpPixelFormatType)
{
    if(AlreadyHookingIT3)
    {
        return CTexMgr__LoadITP_original(this_, iTextureId, pFile, a4, a5, a6, itpPixelFormatType);
    }

    int64_t iPreviousOffset = CSafeFile__ftel(pFile);

    unsigned char bHeader[4];
    CSafeFile__fread(pFile, bHeader, 4, 1, 0, 0);

    TextureType eTexType = GetTextureTypeByHeader(*(unsigned int*)bHeader);
    if(eTexType == TextureType::Unknown)
    {
        CSafeFile__fseek(pFile, iPreviousOffset, SEEK_SET);
        return CTexMgr__LoadITP_original(this_, iTextureId, pFile, a4, a5, a6, itpPixelFormatType);
    }

    CSafeFile__fseek(pFile, iPreviousOffset, SEEK_SET);
    int64_t iFileSize = CSafeFile__GetSize(pFile);
    CSafeFile__fread(pFile, StaticFileBuffer, iFileSize, 1, 0, 0);

    int pixel_width, pixel_height;
    if(!GetCustomTextureDimensions(StaticFileBuffer, iFileSize, eTexType, &pixel_width, &pixel_height))
    {
        CSafeFile__fseek(pFile, iPreviousOffset, SEEK_SET);
        return CTexMgr__LoadITP_original(this_, iTextureId, pFile, a4, a5, a6, itpPixelFormatType);
    }

    char filename_buffer[32] = "pom:/data_patch/";
    stbsp_sprintf(&filename_buffer[16], "%dx%d.itp", pixel_width, pixel_height);
    CSafeFile* pDummyFile = GetCachedFile(filename_buffer);

    StaticTextureType = eTexType;
    StaticFileBufferSize = iFileSize;

    auto ret = CTexMgr__LoadITP_original(this_, iTextureId, pDummyFile, a4, a5, a6, itpPixelFormatType);

    CSafeFile__fclose(pDummyFile);
    CSafeFileBase__destructor(pDummyFile);
    delete pDummyFile;

    StaticTextureType = TextureType::Unknown;

    return ret;
}

// Intercept ITP decoding and load our own buffer after it's done decoding the dummy texture
static int64_t (*CTexBase__LoadITP_chunk_IDAT_original)(uint32_t *a1, uint32_t *a2, int64_t a3, bool *a4, unsigned int *pBGRABuffer, unsigned int iWidth, unsigned int iHeight, uint32_t *a8, uint32_t *a9, uint32_t *a10);
static int64_t CTexBase__LoadITP_chunk_IDAT_hook(uint32_t *a1, uint32_t *a2, int64_t a3, bool *a4, unsigned int *pBGRABuffer, unsigned int iWidth, unsigned int iHeight, uint32_t *a8, uint32_t *a9, uint32_t *a10)
{
    TextureType eTexType = StaticTextureType;

    if(eTexType != TextureType::Unknown)
    {
        ed7_debug("Replacing texture: %dx%d\n", iWidth, iHeight);
        int64_t ret = CTexBase__LoadITP_chunk_IDAT_original(a1, a2, a3, a4, pBGRABuffer, iWidth, iHeight, a8, a9, a10);
        LoadCustomTexture(StaticFileBuffer, StaticFileBufferSize, (unsigned char*)pBGRABuffer, iWidth * iHeight, eTexType, iWidth);
        StaticTextureType = TextureType::Unknown;
        return ret;
    }

    return CTexBase__LoadITP_chunk_IDAT_original(a1, a2, a3, a4, pBGRABuffer, iWidth, iHeight, a8, a9, a10);
}

// Patch loading ITP textures from IT3 files
// Don't report success through piSuccess, so that the game
// fseeks to the next file in the IT3 container instead
// the game will use the returned texture index regardless
static int (*CTexMgr__Load2_original)(int64_t this_, const char *pszTextureName, int64_t a3, CSafeFile *pFile, int* piSuccess, unsigned int a6, unsigned int a7);
static int CTexMgr__Load2_hook(int64_t this_, const char *pszTextureName, int64_t a3, CSafeFile *pFile, int* piSuccess, unsigned int a6, unsigned int a7)
{
    char OutputString[260];
    stbsp_snprintf(OutputString, 260, "rom:/data_en/map/%s/%s.itp", (const char *)pFile, pszTextureName);

    int iWidth, iHeight;
    if(!InitCustomTextureFromFileName(OutputString, StaticFileBuffer, &iWidth, &iHeight))
    {
        int ret = CTexMgr__Load2_original(this_, pszTextureName, a3, pFile, piSuccess, a6, a7);
        return ret;
    }

    char filename_buffer[32] = "pom:/data_patch/";
    stbsp_sprintf(&filename_buffer[16], "%dx%d.itp", iWidth, iHeight);
    ed7_debug("CTexMgr::Load2 - %s - %s - Going to load %s for replacement for %s\n", (const char *)pFile, pszTextureName, filename_buffer, OutputString);

    CSafeFile* pDummyFile = GetCachedFile(filename_buffer);

    AlreadyHookingIT3 = true;

    int dummy;
    int ret = CTexMgr__Load2_original(this_, pszTextureName, a3, pDummyFile, &dummy, a6, a7);
    StaticTextureType = TextureType::Unknown;

    AlreadyHookingIT3 = false;

    CSafeFile__fclose(pDummyFile);
    CSafeFileBase__destructor(pDummyFile);
    delete pDummyFile;

    return ret;
}

void ED7ImgLoaderInitialize()
{
    *(void**)&CSafeFile__CSafeFile = ED7Pointers.CSafeFile__CSafeFile;
    *(void**)&CSafeFile__fclose = ED7Pointers.CSafeFile__fclose;
    *(void**)&CSafeFileBase__destructor = ED7Pointers.CSafeFileBase__destructor;

    *(void**)&CSafeFile__GetSize = ED7Pointers.CSafeFile__GetSize;
    *(void**)&CSafeFile__fseek = ED7Pointers.CSafeFile__fseek;
    *(void**)&CSafeFile__ftel = ED7Pointers.CSafeFile__ftel;
    *(void**)&CSafeFile__fread = ED7Pointers.CSafeFile__fread;
    *(void**)&CSafeFile__fopen = ED7Pointers.CSafeFile__fopen;
    *(void**)&CSafeFile__LoadCCMem = ED7Pointers.CSafeFile__LoadCCMem;
    *(void**)&CSafeFile__CheckOnMem = ED7Pointers.CSafeFile__CheckOnMem;
    *(void**)&CPU__NewBack = ED7Pointers.CPU__NewBack;

    MAKE_HOOK(CTexBase__LoadITP_chunk_IDAT);
    MAKE_HOOK(CTexMgr__Load2);
    MAKE_HOOK(CTexMgr__LoadITP);
}

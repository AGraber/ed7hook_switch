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

static bool InterceptOpenFile = false; // Decides whether we should intercept the call
static bool OpenFileIsRetrying = false; // Tells if this is a retry of OpenFile

static unsigned char StaticFileBuffer[2048 * 2048]; // Buffer for loading files into
static size_t StaticFileBufferSize;
static TextureType StaticTextureType = TextureType::Unknown;

static void (*CSafeFile__CSafeFile)(CSafeFile* this_);
static void (*CSafeFile__fclose)(CSafeFile* this_);
static void (*CSafeFileBase__destructor)(CSafeFile* this_);

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

    TextureType texType = GetTextureTypeByHeader(*(unsigned int*)pFileBuffer);
    if(texType == TextureType::Unknown)
    {
        return false;
    }

    GetCustomTextureDimensions(pFileBuffer, iFileSize, texType, piOutWidth, piOutHeight);

    StaticTextureType = texType;
    StaticFileBufferSize = iFileSize;
    return true;
}

// Intercept file-based texture loading and signal other hooks through variables.
static int (*CTexMgr__Load_original)(int64_t a1, const char *pszFileName, int64_t a3, int64_t a4, unsigned int a5, unsigned int a6);
static int CTexMgr__Load_hook(int64_t a1, const char *pszFileName, int64_t a3, int64_t a4, unsigned int a5, unsigned int a6)
{
    // Sometimes the game will try to load textures that has cached
    // in memory. We will only intercept them if this is an actual
    // file in the romfs

    if(
        strncmp(pszFileName, "rom:/data", 9) != 0 ||
        ( (pszFileName[9] != '/' || strchr(pszFileName + 10, '/') == NULL) &&
        (pszFileName[10] != 't' || pszFileName[10] != 'k' || strchr(pszFileName + 13, '/') == NULL) )
    )
    {
        int ret = CTexMgr__Load_original(a1, pszFileName, a3, a4, a5, a6);
        return ret;
    }

    InterceptOpenFile = true;
    OpenFileIsRetrying = false;

    int ret = CTexMgr__Load_original(a1, pszFileName, a3, a4, a5, a6);

    InterceptOpenFile = false;
    OpenFileIsRetrying = false;
    StaticTextureType = TextureType::Unknown;

    return ret;
}

// Intercept file loading with our own files.
static int64_t (*CSafeFile__fopen_original)(CSafeFile *_this, char *fileName, char *mode, unsigned int a4, unsigned int a5, int64_t a6, unsigned int a7);
static int64_t CSafeFile__fopen_hook(CSafeFile *_this, char *fileName, char *mode, unsigned int a4, unsigned int a5, int64_t a6, unsigned int a7)
{
    if(InterceptOpenFile)
    {
        nn::fs::FileHandle handle;
        if(R_FAILED(nn::fs::OpenFile(&handle, fileName, nn::fs::OpenMode_Read))) {
            if(OpenFileIsRetrying)
            {
                InterceptOpenFile = false;
            }
            else
            {
                OpenFileIsRetrying = true;
            }
            return 0;
        } else {
            unsigned char header[4]; 
            nn::fs::ReadFile(handle, 0, header, 4);

            TextureType texType = GetTextureTypeByHeader(*(unsigned int*)header);
            if(texType == TextureType::Unknown)
            {
                nn::fs::CloseFile(handle);
                InterceptOpenFile = false;
                OpenFileIsRetrying = false;
                return CSafeFile__fopen_original(_this, fileName, mode, a4, a5, a6, a7);
            }
            

            size_t iFileSize;
            nn::fs::GetFileSize((s64*)&iFileSize, handle);
            nn::fs::ReadFile(handle, 0, StaticFileBuffer, iFileSize);
            nn::fs::CloseFile(handle);

            int pixel_width, pixel_height;
            if(!GetCustomTextureDimensions(StaticFileBuffer, iFileSize, texType, &pixel_width, &pixel_height))
            {
                InterceptOpenFile = false;
                OpenFileIsRetrying = false;
                return CSafeFile__fopen_original(_this, fileName, mode, a4, a5, a6, a7);
            }

            StaticTextureType = texType;
            StaticFileBufferSize = iFileSize;

            // Use a dummy itp texture with desired resolution
            // to replace the BGRA buffer of it.
            char filename_buffer[32] = "pom:/data_patch/";
            stbsp_sprintf(&filename_buffer[16], "%dx%d.itp", pixel_width, pixel_height);
            int64_t ret = CSafeFile__fopen_original(_this, filename_buffer, mode, a4, a5, a6, a7);
            return ret;
        }
    }
    return CSafeFile__fopen_original(_this, fileName, mode, a4, a5, a6, a7);
}

// Intercept ITP decoding and load our own buffer after it's done decoding the dummy texture
static int64_t (*CTexBase__LoadITP_chunk_IDAT_original)(uint32_t *a1, uint32_t *a2, int64_t a3, bool *a4, unsigned int *pBGRABuffer, unsigned int iWidth, unsigned int iHeight, uint32_t *a8, uint32_t *a9, uint32_t *a10);
static int64_t CTexBase__LoadITP_chunk_IDAT_hook(uint32_t *a1, uint32_t *a2, int64_t a3, bool *a4, unsigned int *pBGRABuffer, unsigned int iWidth, unsigned int iHeight, uint32_t *a8, uint32_t *a9, uint32_t *a10)
{
    TextureType texType = StaticTextureType;

    if(texType != TextureType::Unknown)
    {
        ed7_debug("Replacing texture: %dx%d\n", iWidth, iHeight);
        int64_t ret = CTexBase__LoadITP_chunk_IDAT_original(a1, a2, a3, a4, pBGRABuffer, iWidth, iHeight, a8, a9, a10);
        LoadCustomTexture(StaticFileBuffer, StaticFileBufferSize, (unsigned char*)pBGRABuffer, iWidth * iHeight, texType, iWidth);
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

    CSafeFile hCustomFile;
    CSafeFile__CSafeFile(&hCustomFile);
    CSafeFile__fopen_original(&hCustomFile, filename_buffer, "rb", 0, 0, 0, 0);

    int dummy;
    int ret = CTexMgr__Load2_original(this_, pszTextureName, a3, &hCustomFile, &dummy, a6, a7);
    StaticTextureType = TextureType::Unknown;

    CSafeFile__fclose(&hCustomFile);
    CSafeFileBase__destructor(&hCustomFile);
    return ret;
}

void ED7ImgLoaderInitialize()
{
    *(void**)&CSafeFile__CSafeFile = ED7Pointers.CSafeFile__CSafeFile;
    *(void**)&CSafeFile__fclose = ED7Pointers.CSafeFile__fclose;
    *(void**)&CSafeFileBase__destructor = ED7Pointers.CSafeFileBase__destructor;

    MAKE_HOOK(CTexMgr__Load);
    MAKE_HOOK(CSafeFile__fopen);
    MAKE_HOOK(CTexBase__LoadITP_chunk_IDAT);
    MAKE_HOOK(CTexMgr__Load2);
}

#include "nn/fs.h"
#include "skyline/inlinehook/And64InlineHook.hpp"
#include "ed7hook/ED7Main.hpp"
#include "ed7hook/ED7Debug.hpp"
#include "ed7hook/ED7Pointers.hpp"
#include "ed7hook/ED7FileLoader.hpp"
#include "skyline/inlinehook/memcpy_controlled.hpp"

static char pathBufferEnglish[512] = "rom:/data_en/";
static char pathBufferJapanese[512] = "rom:/data/";

static char pathBypassBuffer[512] = "rom:/";

static Result (*nn__fs__OpenFile_original)(nn::fs::FileHandle* handle, char const* path, s32 unk);
template<bool hook_data_en, bool hook_data_jp>
static Result nn__fs__OpenFile_hook(nn::fs::FileHandle* handle, char const* path, s32 unk)
{
    // pom:/ is a way to bypass a filename checker that's built into
    // the game's file loader class (CSafeFile). The game has a list of
    // files and will refuse to load any of them if one of the file's not
    // on it, but it won't check files that don't start exactly with
    // rom:/.
    if(strncmp(path, "pom:/", 5) == 0)
    {
        strcpy(&pathBypassBuffer[5], path + 5);
        return nn__fs__OpenFile_original(handle, pathBypassBuffer, unk);
    }

    // Try to access data_en/ (english assets) first
    if constexpr(hook_data_en)
    {
        if(strncmp(path, "rom:/data", 9) == 0)
        {
            switch(path[9])
            {
                case '/': // japanese -> english
                {
                    strcpy(&pathBufferEnglish[13], path + 10);
                    Result res = nn__fs__OpenFile_original(handle, pathBufferEnglish, unk);
                    if(R_SUCCEEDED(res))
                    {
                        return res;
                    }
                    break;
                }

                case '_': // sometimes the game will attempt to open chinese or korean files anyways
                {
                    strcpy(&pathBufferEnglish[13], path + 13);
                    Result res = nn__fs__OpenFile_original(handle, pathBufferEnglish, unk);
                    if(R_SUCCEEDED(res))
                    {
                        return res;
                    }
                    break;
                }
            }
        }
    }

    // Try to access data/ (japanese assets) first
    if constexpr(hook_data_jp)
    {
        if(strncmp(path, "rom:/data", 9) == 0)
        {
            if(path[9] == '_')
            {
                strcpy(&pathBufferJapanese[10], path + 13);
                Result res = nn__fs__OpenFile_original(handle, pathBufferJapanese, unk);
                if(R_SUCCEEDED(res))
                {
                    return res;
                }
            }
        }
    }
    return nn__fs__OpenFile_original(handle, path, unk);
}

void ED7FileLoaderInitialize()
{
    void *nn__fs__OpenFile_ptr =
        ED7HookCurrentLanguage == ED7HookLanguage::English ?
            reinterpret_cast<void*>(nn__fs__OpenFile_hook<true, false>)
        :
            (ED7HookCurrentLanguage == ED7HookLanguage::Japanese ?
                reinterpret_cast<void*>(nn__fs__OpenFile_hook<false, true>)
            :
                reinterpret_cast<void*>(nn__fs__OpenFile_hook<false, false>)
            )
    ;

    A64HookFunction(
        (void*)nn::fs::OpenFile,
        nn__fs__OpenFile_ptr,
        (void**)&nn__fs__OpenFile_original
    );

    // Skip file name check in some instances
    static constexpr unsigned char SkipFileNameCheck[4] = {0x08, 0x00, 0x00, 0x14}; // b #0x28
    sky_memcpy(ED7Pointers.CSafeFileBase__open_FileNameCheck, SkipFileNameCheck, 4);
}

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "nn/fs.h"
#include "skyline/utils/cpputils.hpp"
#include "ed7hook/ED7Debug.hpp"
#include "ed7hook/ED7Main.hpp"
#include "ed7hook/ED7Pointers.hpp"
#include "stb_sprintf.h"

bool Initialized = false;

void ED7DebugInitialize()
{
#ifdef ED7HOOK_DEBUG
    if(Initialized)
    {
        return;
    }
    Initialized = true;

    Result rc = nn::fs::MountSdCardForDebug("sd");
    R_ERRORONFAIL(rc);

    nn::fs::FileHandle handle;
    if(R_SUCCEEDED(nn::fs::OpenFile(&handle, "sd:/ed7hook_output.txt", nn::fs::OpenMode_ReadWrite)))
    {
        nn::fs::CloseFile(handle);
        nn::fs::DeleteFile("sd:/ed7hook_output.txt");
    }

    nn::settings::LanguageCode lang;
    nn::settings::GetLanguageCode(&lang);

    ed7_debug("-----------------------------------------\n");
    ed7_debug("ed7hook debug log initialized\n");
    ed7_debug("ver. " ED7HOOK_VERSION "\n");
    ed7_debug("Built on " __DATE__ " " __TIME__ "\n");
    ed7_debug("Game: %s\n", ED7Pointers.GameName);
    ed7_debug("Console Language: %s\n", lang.code);
    ed7_debug("ED7HookCurrentLanguage: %d\n", ED7HookCurrentLanguage);
    ed7_debug("nninitStartup Address: %" PRIx64 "\n", ED7Pointers.NnInitStartup);
    ed7_debug("TextRegionOffset Address: %" PRIx64 "\n", ED7Pointers.TextRegionOffset);
    ed7_debug("-----------------------------------------\n");
#endif
}

#ifdef ED7HOOK_DEBUG
static size_t offset = 0;
static char format_buffer[1000];

void _ed7_debug(const char* text, ...)
{
    if(!Initialized)
    {
        ED7DebugInitialize();
    }

    va_list args;
    va_start(args, text);
    stbsp_vsprintf(format_buffer, text, args);
    va_end(args);
    size_t len = strlen(format_buffer);
    skyline::utils::writeFile("sd:/ed7hook_output.txt", offset, (void*)&format_buffer, len);
    offset += len;
}
#endif

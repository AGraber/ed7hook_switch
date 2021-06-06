#pragma once

#define ED7HOOK_VERSION "v1.0.0"

enum ED7HookLanguage
{
    Japanese,
    TraditionalChinese,
    Korean,
    English,
};

extern ED7HookLanguage ED7HookCurrentLanguage;

void ed7hook_main();

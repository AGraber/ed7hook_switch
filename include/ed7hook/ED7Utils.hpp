#pragma once

#define MAKE_HOOK(hook_name) A64HookFunction(ED7Pointers.hook_name, reinterpret_cast<void*>(hook_name ## _hook), (void**)&hook_name ## _original)

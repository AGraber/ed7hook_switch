#pragma once

#define MAKE_HOOK(hook_name) A64HookFunction(ED7Pointers.hook_name, reinterpret_cast<void*>(hook_name ## _hook), (void**)&hook_name ## _original)
#define MAKE_HOOK_T(hook_name) A64HookFunction(ED7Pointers.hook_name, reinterpret_cast<void*>(ED7Pointers.IsZero ? hook_name ## _hook<true> : hook_name ## _hook<false>), (void**)&hook_name ## _original)

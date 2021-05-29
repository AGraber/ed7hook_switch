#pragma once
void ED7DebugInitialize();

#ifdef ED7HOOK_DEBUG

void _ed7_debug(const char* text, ...);
#define ed7_debug(...); _ed7_debug(__VA_ARGS__);

#else

#define ed7_debug(...);

#endif

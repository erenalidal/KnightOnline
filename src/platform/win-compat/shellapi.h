// macOS port compat — see docs/MAC_PORT_ROADMAP.md
// shellapi.h: ShellExecute (URL/dosya açma). Stub — TODO: macOS `open` komutu.
#ifndef KO_PLATFORM_SHELLAPI_H_
#define KO_PLATFORM_SHELLAPI_H_
#pragma once
#if !defined(_WIN32)
#include "win_user32.h"
#define SW_SHOW        5
#define SW_SHOWNORMAL  1
#define SW_HIDE        0
inline HINSTANCE ShellExecuteA(HWND, const char*, const char*, const char*, const char*, int) { return nullptr; }
#ifndef ShellExecute
#define ShellExecute ShellExecuteA
#endif
#endif
#endif

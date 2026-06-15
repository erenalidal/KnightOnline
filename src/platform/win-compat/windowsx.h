// macOS port compat — see docs/MAC_PORT_ROADMAP.md
// windowsx.h: yalnızca GET_X_LPARAM / GET_Y_LPARAM makroları kullanılıyor.
#ifndef KO_PLATFORM_WINDOWSX_H_
#define KO_PLATFORM_WINDOWSX_H_
#pragma once
#if !defined(_WIN32)
#include "win_types.h"
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int) (short) LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int) (short) HIWORD(lp))
#endif
#endif
#endif

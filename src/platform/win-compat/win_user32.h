// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// win_user32.h — Win32 pencere/mesaj (user32) API'lerinin stub'ları.
//
// N3UIEdit gizli bir Win32 "EDIT" kontrolü ile metin girişi yapar; N3UIButton/
// N3UIBase de pencere yardımcıları kullanır. Bunların tamamı macOS'ta GERÇEK
// DEĞİL — Faz 2/4'te SDL metin girişi/olay sistemiyle değişecek. Burada yalnızca
// derlemeyi geçirecek güvenli stub'lar var.

#ifndef KO_PLATFORM_WIN_USER32_H_
#define KO_PLATFORM_WIN_USER32_H_

#pragma once

#if !defined(_WIN32)

#include "win_types.h"

// --- WPARAM/LPARAM/LRESULT (tek tanım noktası) ---
#ifndef KO_WPARAM_DEFINED
#define KO_WPARAM_DEFINED
typedef uintptr_t WPARAM;
typedef intptr_t  LPARAM;
typedef intptr_t  LRESULT;
typedef intptr_t  LONG_PTR;
#endif

#ifndef HMODULE
typedef void* HMODULE;
typedef void* HMENU;
typedef void* HICON;
typedef void* HBRUSH;
typedef void* HIMC;
typedef void* HCURSOR;
typedef void* HACCEL;
typedef void* HKL_;
#endif

// Custom mesaj tabanı (APISocket WM_SOCKETMSG = WM_USER+1).
#ifndef WM_USER
#define WM_USER 0x0400
#endif

// Cursor stub'ları (gerçek cursor SDL ile gelecek).
inline HCURSOR LoadCursorA(HMODULE, const char*) { return nullptr; }
#ifndef LoadCursor
#define LoadCursor LoadCursorA
#endif
inline HCURSOR SetCursor(HCURSOR) { return nullptr; }
inline HCURSOR GetCursor() { return nullptr; }
inline int     ShowCursor(BOOL) { return 0; }
inline UINT    GetDoubleClickTime() { return 500; }
inline BOOL    GetWindowRect(HWND, RECT* r) { if (r) { r->left = r->top = r->right = r->bottom = 0; } return TRUE; }
inline BOOL    GetCursorPos(POINT* p) { if (p) { p->x = 0; p->y = 0; } return TRUE; }
inline BOOL    SetCursorPos(int, int) { return TRUE; }
inline BOOL    ClientToScreen(HWND, POINT*) { return TRUE; }
inline BOOL    ScreenToClient(HWND, POINT*) { return TRUE; }

// --- Display mode (fullscreen geçişi — Faz 2'de SDL ile değişecek) ---
typedef struct _devicemodeA
{
	char  dmDeviceName[32];
	WORD  dmSpecVersion, dmDriverVersion, dmSize, dmDriverExtra;
	DWORD dmFields;
	int   dmPositionX, dmPositionY;
	DWORD dmDisplayOrientation, dmDisplayFixedOutput;
	short dmColor, dmDuplex, dmYResolution, dmTTOption, dmCollate;
	char  dmFormName[32];
	WORD  dmLogPixels;
	DWORD dmBitsPerPel, dmPelsWidth, dmPelsHeight;
	DWORD dmDisplayFlags, dmDisplayFrequency;
} DEVMODEA, DEVMODE, *LPDEVMODEA, *LPDEVMODE;

#define DM_BITSPERPEL       0x00040000
#define DM_PELSWIDTH        0x00080000
#define DM_PELSHEIGHT       0x00100000
#define DM_DISPLAYFREQUENCY 0x00400000
#define CDS_FULLSCREEN      0x00000004
#define DISP_CHANGE_SUCCESSFUL 0
#define ENUM_CURRENT_SETTINGS  ((DWORD) -1)

inline LONG ChangeDisplaySettingsA(DEVMODE*, DWORD) { return DISP_CHANGE_SUCCESSFUL; }
inline BOOL EnumDisplaySettingsA(const char*, DWORD, DEVMODE*) { return FALSE; }
#ifndef ChangeDisplaySettings
#define ChangeDisplaySettings ChangeDisplaySettingsA
#define EnumDisplaySettings   EnumDisplaySettingsA
#endif

// Dil kimliği — varsayılan İngilizce (US).
inline WORD GetUserDefaultLangID() { return 0x0409; }
inline WORD GetSystemDefaultLangID() { return 0x0409; }

// WNDPROC: pencere mesaj işleyici imzası.
typedef LRESULT (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);

// --- Pencere stilleri / mesajları / sabitler ---
#define WS_CHILD       0x40000000
#define WS_VISIBLE     0x10000000
#define WS_TABSTOP     0x00010000
#define WS_POPUP       0x80000000
#define WS_SYSMENU     0x00080000
#define WS_CLIPCHILDREN 0x02000000
#define WS_CLIPSIBLINGS 0x04000000
#define WS_CAPTION     0x00C00000
#define WS_BORDER      0x00800000
#define WS_OVERLAPPED  0x00000000
#define WS_OVERLAPPEDWINDOW 0x00CF0000
#define WS_EX_TOPMOST  0x00000008
#define CW_USEDEFAULT  ((int) 0x80000000)
#define SW_HIDE        0
#define SW_SHOWNORMAL  1
#define SW_SHOW        5

#ifndef MAKEINTRESOURCE
#define MAKEINTRESOURCE(i) ((const char*) (uintptr_t) ((WORD) (i)))
#endif
#define ES_LEFT      0x0000
#define ES_AUTOHSCROLL 0x0080
#define ES_PASSWORD  0x0020
#define ES_WANTRETURN 0x1000

#define GWLP_WNDPROC (-4)
#define GWLP_USERDATA (-21)

#define WM_CREATE                0x0001
#define WM_SETFONT               0x0030
#define WM_KEYDOWN               0x0100
#define WM_CHAR                  0x0102
#define WM_IME_STARTCOMPOSITION  0x010D
#define WM_IME_ENDCOMPOSITION    0x010E
#define WM_IME_NOTIFY            0x0282
#define WM_INPUTLANGCHANGE       0x0051

#define EM_GETSEL    0x00B0
#define EM_SETSEL    0x00B1
#define EM_LIMITTEXT 0x00C5

#define IMN_CLOSESTATUSWINDOW 0x0001
#define IMN_OPENSTATUSWINDOW  0x0002

// --- Mesaj makroları ---
#ifndef LOWORD
#define LOWORD(l) ((WORD) (((uintptr_t) (l)) & 0xffff))
#define HIWORD(l) ((WORD) ((((uintptr_t) (l)) >> 16) & 0xffff))
#endif
#ifndef MAKELPARAM
#define MAKELPARAM(lo, hi) ((LPARAM) ((((uint32_t) (hi)) << 16) | ((uint16_t) (lo))))
#define MAKEWPARAM(lo, hi) ((WPARAM) ((((uint32_t) (hi)) << 16) | ((uint16_t) (lo))))
#endif

// --- Pencere fonksiyonu stub'ları ---
inline HWND CreateWindowA(const char*, const char*, DWORD, int, int, int, int,
	HWND, HMENU, HMODULE, void*) { return nullptr; }
#ifndef CreateWindow
#define CreateWindow CreateWindowA
#endif

inline LONG_PTR SetWindowLongPtrA(HWND, int, LONG_PTR) { return 0; }
inline LONG_PTR GetWindowLongPtrA(HWND, int) { return 0; }
#ifndef SetWindowLongPtr
#define SetWindowLongPtr SetWindowLongPtrA
#define GetWindowLongPtr GetWindowLongPtrA
#endif

inline LRESULT CallWindowProcA(WNDPROC, HWND, UINT, WPARAM, LPARAM) { return 0; }
inline LRESULT DefWindowProcA(HWND, UINT, WPARAM, LPARAM) { return 0; }
inline LRESULT SendMessageA(HWND, UINT, WPARAM, LPARAM) { return 0; }
#ifndef CallWindowProc
#define CallWindowProc CallWindowProcA
#define DefWindowProc  DefWindowProcA
#define SendMessage    SendMessageA
#endif

inline BOOL SetWindowTextA(HWND, const char*) { return TRUE; }
inline int  GetWindowTextA(HWND, char* buf, int n) { if (buf && n > 0) buf[0] = '\0'; return 0; }
inline int  GetWindowTextLengthA(HWND) { return 0; }
#ifndef SetWindowText
#define SetWindowText       SetWindowTextA
#define GetWindowText       GetWindowTextA
#define GetWindowTextLength GetWindowTextLengthA
#endif

inline BOOL MoveWindow(HWND, int, int, int, int, BOOL) { return TRUE; }
inline HWND SetFocus(HWND) { return nullptr; }
inline HWND SetActiveWindow(HWND) { return nullptr; }
inline BOOL SetForegroundWindow(HWND) { return TRUE; }
inline HWND GetForegroundWindow() { return nullptr; }
inline BOOL ShowWindow(HWND, int) { return TRUE; }
inline BOOL UpdateWindow(HWND) { return TRUE; }
inline BOOL DestroyWindow(HWND) { return TRUE; }
inline BOOL InvalidateRect(HWND, const RECT*, BOOL) { return TRUE; }

// --- Virtual key kodları (klavye + fare) ---
#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_MBUTTON 0x04
#define VK_BACK    0x08
#define VK_TAB     0x09
#define VK_RETURN  0x0D
#define VK_SHIFT   0x10
#define VK_CONTROL 0x11
#define VK_ESCAPE  0x1B
#define VK_SPACE   0x20
#define VK_LEFT    0x25
#define VK_UP      0x26
#define VK_RIGHT   0x27
#define VK_DOWN    0x28
#define VK_DELETE  0x2E
#define VK_MENU    0x12
#define VK_F1      0x70

// WM_ACTIVATE wParam değerleri.
#define WA_INACTIVE    0
#define WA_ACTIVE      1
#define WA_CLICKACTIVE 2

// Edit notification (WM_COMMAND HIWORD) + mousewheel.
#define EN_CHANGE 0x0300
#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wParam) ((short) HIWORD(wParam))
#define WHEEL_DELTA 120
#endif

// --- DC / GDI ek stub'ları ---
#define TRANSPARENT     1
#define OPAQUE          2
#define R2_COPYPEN      13
#define R2_XORPEN       7
#define ANSI_FIXED_FONT 11
#define SYSTEM_FONT     13

inline HDC     GetDC(HWND) { return nullptr; }
inline int     ReleaseDC(HWND, HDC) { return 1; }
inline int     SetBkMode(HDC, int) { return 0; }
inline int     SetROP2(HDC, int) { return 0; }
inline HGDIOBJ GetStockObject(int) { return nullptr; }

// --- HKL / ek IME stub'ları ---
typedef void* HKL;
inline HKL  GetKeyboardLayout(DWORD) { return nullptr; }
inline BOOL ImmIsIME(HKL) { return FALSE; }
inline BOOL ImmSetStatusWindowPos(HIMC, POINT*) { return TRUE; }

// --- IME stub'ları ---
inline HIMC ImmGetContext(HWND) { return nullptr; }
inline BOOL ImmReleaseContext(HWND, HIMC) { return TRUE; }

// --- Pencere mesajları (WM_*) ---
#define WM_DESTROY     0x0002
#define WM_SIZE        0x0005
#define WM_ACTIVATE    0x0006
#define WM_CLOSE       0x0010
#define WM_QUIT        0x0012
#define WM_KEYUP       0x0101
#define WM_COMMAND     0x0111
#define WM_MOUSEWHEEL  0x020A
#define WS_GROUP       0x00020000
#define NULL_BRUSH     5

// --- Pencere sınıfı / yaşam döngüsü (Faz 2'de SDL ile değişecek) ---
typedef struct tagWNDCLASSEXA
{
	UINT      cbSize, style;
	WNDPROC   lpfnWndProc;
	int       cbClsExtra, cbWndExtra;
	HMODULE   hInstance;
	HICON     hIcon;
	HCURSOR   hCursor;
	HBRUSH    hbrBackground;
	const char* lpszMenuName;
	const char* lpszClassName;
	HICON     hIconSm;
} WNDCLASSEXA, *LPWNDCLASSEXA;
typedef WORD ATOM;

inline ATOM RegisterClassExA(const WNDCLASSEXA*) { return 1; }
inline BOOL UnregisterClassA(const char*, HMODULE) { return TRUE; }
inline HWND CreateWindowExA(DWORD, const char*, const char*, DWORD, int, int, int, int,
	HWND, HMENU, HMODULE, void*) { return nullptr; }
inline HICON LoadIconA(HMODULE, const char*) { return nullptr; }
inline void  PostQuitMessage(int) {}
inline BOOL  GetMessageA(MSG*, HWND, UINT, UINT) { return FALSE; }
inline BOOL  AdjustWindowRect(RECT*, DWORD, BOOL) { return TRUE; }
inline BOOL  AdjustWindowRectEx(RECT*, DWORD, BOOL, DWORD) { return TRUE; }
#ifndef RegisterClassEx
#define RegisterClassEx RegisterClassExA
#define UnregisterClass UnregisterClassA
#define CreateWindowEx  CreateWindowExA
#define LoadIcon        LoadIconA
#define GetMessage      GetMessageA
#endif

// --- Registry (kayıtlı ayarlar — STUB: başarısız → client default kullanır) ---
#ifndef HKEY
typedef void* HKEY;
typedef HKEY* PHKEY;
#endif
#define HKEY_CLASSES_ROOT   ((HKEY) (uintptr_t) 0x80000000)
#define HKEY_CURRENT_USER   ((HKEY) (uintptr_t) 0x80000001)
#define HKEY_LOCAL_MACHINE  ((HKEY) (uintptr_t) 0x80000002)
#define ERROR_SUCCESS       0L
#define REG_SZ              1
#define REG_BINARY          3
#define REG_DWORD           4

inline LONG RegOpenKeyA(HKEY, const char*, PHKEY) { return 1; }   // != ERROR_SUCCESS
inline LONG RegCreateKeyA(HKEY, const char*, PHKEY) { return 1; }
inline LONG RegCloseKey(HKEY) { return ERROR_SUCCESS; }
inline LONG RegQueryValueExA(HKEY, const char*, DWORD*, DWORD*, BYTE*, DWORD*) { return 1; }
inline LONG RegSetValueExA(HKEY, const char*, DWORD, DWORD, const BYTE*, DWORD) { return ERROR_SUCCESS; }
#ifndef RegOpenKey
#define RegOpenKey      RegOpenKeyA
#define RegCreateKey    RegCreateKeyA
#define RegQueryValueEx RegQueryValueExA
#define RegSetValueEx   RegSetValueExA
#endif

// --- PtInRect (RECT, POINT dxvk'dan gelir) ---
inline BOOL PtInRect(const RECT* r, POINT p)
{
	return (r != nullptr && p.x >= r->left && p.x < r->right
		&& p.y >= r->top && p.y < r->bottom) ? TRUE : FALSE;
}

#endif // !defined(_WIN32)

#endif // KO_PLATFORM_WIN_USER32_H_

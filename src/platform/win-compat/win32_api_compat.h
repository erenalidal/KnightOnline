// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// win32_api_compat.h — N3Base'in kullandığı, dxvk-native'in SAĞLAMADIĞI Win32
// API yüzeyinin (GDI bitmap struct'ları, kernel32 global bellek, user32 stub'ları,
// input stub'ları) macOS/clang karşılıkları.
//
// Strateji: gerçek davranış gerektirmeyenler stub'lanır (ileride SDL/Metal ile
// gerçeklenecek); saf veri/bellek olanlar POSIX'e map'lenir. Tümü `#if !defined(_WIN32)`
// altında ve `#ifndef` korumalı (dxvk ya da başka shim tanımladıysa çakışmaz).

#ifndef KO_PLATFORM_WIN32_API_COMPAT_H_
#define KO_PLATFORM_WIN32_API_COMPAT_H_

#pragma once

#if !defined(_WIN32)

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <ctime>
#include <unistd.h>
#include <mach/mach_time.h>
#include <cerrno>

// CopyMemory / MoveMemory — Win32 makroları (ZeroMemory win_types.h'da tanımlı).
#ifndef CopyMemory
#define CopyMemory(dest, src, len) ::memcpy((dest), (src), (len))
#endif
#ifndef MoveMemory
#define MoveMemory(dest, src, len) ::memmove((dest), (src), (len))
#endif

#include "win_types.h" // BOOL/DWORD/WORD/HWND + QPC; d3d9.h üzerinden LONG vb.

// LONG — dxvk genelde sağlar; sağlamadıysa fallback.
#ifndef KO_LONG_DEFINED
#define KO_LONG_DEFINED
#ifndef LONG
typedef int32_t LONG;
#endif
#endif

//
// --- GDI bitmap struct'ları (BitMapFile.* için) ----------------------------
//
#ifndef KO_BITMAP_STRUCTS_DEFINED
#define KO_BITMAP_STRUCTS_DEFINED
#pragma pack(push, 2)
typedef struct tagBITMAPFILEHEADER
{
	WORD  bfType;
	DWORD bfSize;
	WORD  bfReserved1;
	WORD  bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER;
#pragma pack(pop)

typedef struct tagBITMAPINFOHEADER
{
	DWORD biSize;
	LONG  biWidth;
	LONG  biHeight;
	WORD  biPlanes;
	WORD  biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG  biXPelsPerMeter;
	LONG  biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER;

typedef struct tagRGBQUAD
{
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
} RGBQUAD;

#ifndef BI_RGB
#define BI_RGB 0
#endif
#endif // KO_BITMAP_STRUCTS_DEFINED

//
// --- kernel32 global bellek (GlobalAlloc/GlobalFree) -----------------------
//
#ifndef KO_GLOBALMEM_DEFINED
#define KO_GLOBALMEM_DEFINED
#ifndef HGLOBAL
typedef void* HGLOBAL;
#endif
#ifndef GMEM_FIXED
#define GMEM_FIXED    0x0000
#define GMEM_ZEROINIT 0x0040
#define GPTR          (GMEM_FIXED | GMEM_ZEROINIT)
#endif

inline HGLOBAL GlobalAlloc(unsigned int uFlags, size_t dwBytes)
{
	void* p = ::malloc(dwBytes);
	if (p != nullptr && (uFlags & GMEM_ZEROINIT) != 0)
		::memset(p, 0, dwBytes);
	return p;
}

inline HGLOBAL GlobalFree(HGLOBAL hMem)
{
	::free(hMem);
	return nullptr;
}
#endif // KO_GLOBALMEM_DEFINED

//
// --- user32 stub'ları ------------------------------------------------------
//
// Bu fonksiyonlar pencere/mesaj sistemiyle ilgili; macOS'ta render katmanı SDL'e
// taşınana dek güvenli no-op döndürürler.
//
#ifndef KO_USER32_STUBS_DEFINED
#define KO_USER32_STUBS_DEFINED

#ifndef MB_OK
#define MB_OK              0x00000000
#define MB_ICONERROR       0x00000010
#define MB_ICONINFORMATION 0x00000040
#define MB_ICONWARNING     0x00000030
#define MB_YESNO           0x00000004
#define IDOK               1
#define IDYES              6
#define IDNO               7
#endif

inline HWND GetActiveWindow() { return nullptr; }

// RECT yardımcıları (GDI).
inline BOOL SetRect(RECT* r, int l, int t, int rt, int b)
{ if (!r) return FALSE; r->left = l; r->top = t; r->right = rt; r->bottom = b; return TRUE; }
inline BOOL SetRectEmpty(RECT* r) { return SetRect(r, 0, 0, 0, 0); }
inline BOOL OffsetRect(RECT* r, int dx, int dy)
{ if (!r) return FALSE; r->left += dx; r->right += dx; r->top += dy; r->bottom += dy; return TRUE; }
inline BOOL EqualRect(const RECT* a, const RECT* b)
{ return (a && b && a->left == b->left && a->top == b->top && a->right == b->right && a->bottom == b->bottom) ? TRUE : FALSE; }
inline BOOL IsRectEmpty(const RECT* r)
{ return (!r || (r->right <= r->left || r->bottom <= r->top)) ? TRUE : FALSE; }

inline int MessageBoxA(HWND, const char*, const char*, unsigned int) { return IDOK; }
#ifndef MessageBox
#define MessageBox MessageBoxA
#endif

// Input: gerçek implementasyon SDL ile gelecek (Faz 4). Şimdilik "basılı değil".
inline int16_t GetAsyncKeyState(int) { return 0; }
inline int16_t GetKeyState(int) { return 0; }

// MessageBoxW (geniş karakter varyantı) — BitMapFile vb. L"..." kullanır.
inline int MessageBoxW(HWND, const wchar_t*, const wchar_t*, unsigned int) { return IDOK; }

#endif // KO_USER32_STUBS_DEFINED

//
// --- INT/UINT yardımcı tipler ----------------------------------------------
//
#ifndef KO_INT_TYPES_DEFINED
#define KO_INT_TYPES_DEFINED
#ifndef INT
typedef int INT;
#endif
#ifndef UINT
typedef unsigned int UINT;
#endif
#endif

//
// --- String API'leri (kernel32/user32) -------------------------------------
//
#ifndef KO_STRING_API_DEFINED
#define KO_STRING_API_DEFINED
inline int   lstrlenA(const char* s) { return s ? static_cast<int>(::strlen(s)) : 0; }
#ifndef lstrlen
#define lstrlen lstrlenA
#endif

// CharLowerA: yerinde küçük harfe çevirir, başlangıç işaretçisini döndürür.
inline char* CharLowerA(char* s)
{
	if (s != nullptr)
		for (char* p = s; *p; ++p)
			*p = static_cast<char>(::tolower(static_cast<unsigned char>(*p)));
	return s;
}
#ifndef CharLower
#define CharLower CharLowerA
#endif

// _strlwr / _strupr — yerinde küçük/büyük harf (CRT).
inline char* _strlwr(char* s) { return CharLowerA(s); }
inline char* _strupr(char* s) { if (s) for (char* p = s; *p; ++p) *p = static_cast<char>(::toupper(static_cast<unsigned char>(*p))); return s; }

// _strnicmp / _stricmp — büyük/küçük harf duyarsız karşılaştırma (CRT).
#ifndef _strnicmp
#define _strnicmp strncasecmp
#endif
#ifndef _stricmp
#define _stricmp strcasecmp
#endif

// SetCurrentDirectory / GetCurrentDirectory — chdir/getcwd.
inline BOOL SetCurrentDirectoryA(const char* p) { return (p && ::chdir(p) == 0) ? TRUE : FALSE; }
inline DWORD GetCurrentDirectoryA(DWORD n, char* buf) { return ::getcwd(buf, n) ? static_cast<DWORD>(::strlen(buf)) : 0; }
#ifndef SetCurrentDirectory
#define SetCurrentDirectory SetCurrentDirectoryA
#define GetCurrentDirectory GetCurrentDirectoryA
#endif

// lstrcpy / lstrcat — strcpy/strcat.
inline char* lstrcpyA(char* d, const char* s) { return ::strcpy(d, s ? s : ""); }
inline char* lstrcatA(char* d, const char* s) { return ::strcat(d, s ? s : ""); }
inline char* lstrcpynA(char* d, const char* s, int n) { ::strncpy(d, s ? s : "", n > 0 ? n - 1 : 0); if (n > 0) d[n - 1] = '\0'; return d; }
#ifndef lstrcpy
#define lstrcpy  lstrcpyA
#define lstrcat  lstrcatA
#define lstrcpyn lstrcpynA
#endif

// GetPrivateProfileString/Int — INI okuma. STUB: varsayılanı döndürür.
// TODO(mac-port): gerçek INI parse (config dosyaları için).
inline DWORD GetPrivateProfileStringA(const char*, const char*, const char* def,
	char* ret, DWORD nSize, const char*)
{
	const char* v = def ? def : "";
	::strncpy(ret, v, nSize);
	if (nSize > 0) ret[nSize - 1] = '\0';
	return static_cast<DWORD>(::strlen(ret));
}
inline UINT GetPrivateProfileIntA(const char*, const char*, int def, const char*) { return static_cast<UINT>(def); }
inline BOOL WritePrivateProfileStringA(const char*, const char*, const char*, const char*) { return TRUE; }
#ifndef GetPrivateProfileString
#define GetPrivateProfileString GetPrivateProfileStringA
#define GetPrivateProfileInt    GetPrivateProfileIntA
#define WritePrivateProfileString WritePrivateProfileStringA
#endif
#endif // KO_STRING_API_DEFINED

//
// --- SYSTEMTIME + GetLocalTime (LogWriter için) ----------------------------
//
#ifndef KO_SYSTEMTIME_DEFINED
#define KO_SYSTEMTIME_DEFINED
typedef struct _SYSTEMTIME
{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME, *LPSYSTEMTIME;

inline void GetLocalTime(SYSTEMTIME* st)
{
	if (st == nullptr)
		return;
	::time_t t = ::time(nullptr);
	::tm lt;
	::localtime_r(&t, &lt);
	st->wYear         = static_cast<WORD>(lt.tm_year + 1900);
	st->wMonth        = static_cast<WORD>(lt.tm_mon + 1);
	st->wDayOfWeek    = static_cast<WORD>(lt.tm_wday);
	st->wDay          = static_cast<WORD>(lt.tm_mday);
	st->wHour         = static_cast<WORD>(lt.tm_hour);
	st->wMinute       = static_cast<WORD>(lt.tm_min);
	st->wSecond       = static_cast<WORD>(lt.tm_sec);
	st->wMilliseconds = 0;
}
#endif // KO_SYSTEMTIME_DEFINED

//
// --- GDI font API'leri (DFont için) ----------------------------------------
//
// DFont, Windows GDI ile font'u bir DC'ye rasterize edip glyph bitmap'leri alır.
// macOS'ta bu yol GERÇEK DEĞİL: handle'lar opak, fonksiyonlar no-op döner.
// Metin render'ı düzgün çalışana dek (Faz 6) yer tutucudur — derlemeyi geçirir.
//
#ifndef KO_GDI_FONT_DEFINED
#define KO_GDI_FONT_DEFINED
#ifndef HGDIOBJ
typedef void* HGDIOBJ;
#endif
#ifndef HFONT
typedef void* HFONT;
#endif
#ifndef HBITMAP
typedef void* HBITMAP;
#endif

// GDI font/DC sabitleri.
#define LOGPIXELSY          90
#define DEFAULT_CHARSET     1
#define OUT_DEFAULT_PRECIS  0
#define CLIP_DEFAULT_PRECIS 0
#define DEFAULT_QUALITY     0
#define ANTIALIASED_QUALITY 4
#define PROOF_QUALITY       2
#define DEFAULT_PITCH       0
#define VARIABLE_PITCH      2
#define FIXED_PITCH         1
#define FF_DONTCARE         0
#define FW_NORMAL           400
#define FW_BOLD             700

inline int     MulDiv(int a, int b, int c) { return (c != 0) ? static_cast<int>((static_cast<long long>(a) * b) / c) : 0; }
inline HDC     CreateCompatibleDC(HDC) { return nullptr; }
inline BOOL    DeleteDC(HDC) { return TRUE; }
inline int     GetDeviceCaps(HDC, int) { return LOGPIXELSY; }
inline HGDIOBJ SelectObject(HDC, HGDIOBJ) { return nullptr; }
inline BOOL    DeleteObject(HGDIOBJ) { return TRUE; }

inline HFONT CreateFontA(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD,
	DWORD, DWORD, DWORD, DWORD, const char*) { return nullptr; }
#ifndef CreateFont
#define CreateFont CreateFontA
#endif

// --- GDI metin/DIB API'leri (DFont glyph rasterizer) ---
#ifndef HANDLE
typedef void* HANDLE;
#endif
typedef DWORD COLORREF;
#ifndef RGB
#define RGB(r, g, b) ((COLORREF) (((BYTE) (r)) | (((WORD) (BYTE) (g)) << 8) | (((DWORD) (BYTE) (b)) << 16)))
#endif

// SIZE dxvk-native tarafından sağlanır; yeniden tanımlamıyoruz.

typedef struct tagBITMAPINFO
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD          bmiColors[1];
} BITMAPINFO, *LPBITMAPINFO;

#define MM_TEXT        1
#define TA_TOP         0
#define TA_LEFT        0
#define ETO_OPAQUE     0x0002
#define ETO_CLIPPED    0x0004
#define DIB_RGB_COLORS 0

inline int      SetMapMode(HDC, int) { return 0; }
inline COLORREF SetTextColor(HDC, COLORREF) { return 0; }
inline COLORREF SetBkColor(HDC, COLORREF) { return 0; }
inline UINT     SetTextAlign(HDC, UINT) { return 0; }

inline BOOL GetTextExtentPoint32A(HDC, const char*, int, SIZE* sz)
{
	if (sz != nullptr) { sz->cx = 0; sz->cy = 0; }
	return TRUE;
}
#ifndef GetTextExtentPoint32
#define GetTextExtentPoint32 GetTextExtentPoint32A
#endif

inline BOOL ExtTextOutA(HDC, int, int, UINT, const RECT*, const char*, UINT, const int*) { return TRUE; }
#ifndef ExtTextOut
#define ExtTextOut ExtTextOutA
#endif

// CreateDIBSection: glyph bitmap'i için DIB. Stub: bit buffer'ı yok (nullptr).
// Gerçek metin rasterizasyonu Faz 6'da (cross-platform font) gelecek.
inline HBITMAP CreateDIBSection(HDC, const BITMAPINFO*, UINT, void** ppvBits, HANDLE, DWORD)
{
	if (ppvBits != nullptr) *ppvBits = nullptr;
	return nullptr;
}

// --- GDI palette/DIB API'leri (JpegFile DIB yolu) ---
// Asıl JPEG decode libjpeg ile yapılır; bu Windows DIB/palette yolu stub'lanır.
#ifndef HPALETTE
typedef void* HPALETTE;
#endif
#ifndef LPBYTE
typedef BYTE* LPBYTE;
#endif

typedef struct tagLOGPALETTE
{
	WORD         palVersion;
	WORD         palNumEntries;
	PALETTEENTRY palPalEntry[1];
} LOGPALETTE, *LPLOGPALETTE;

#define NUMCOLORS   24
#define BITSPIXEL   12
#define RASTERCAPS  38
#define RC_PALETTE  0x0100
#define SIZEPALETTE 104
#define PLANES      14

inline HPALETTE CreatePalette(const LOGPALETTE*) { return nullptr; }
inline HPALETTE SelectPalette(HDC, HPALETTE, BOOL) { return nullptr; }
inline UINT     RealizePalette(HDC) { return 0; }
inline UINT     GetSystemPaletteEntries(HDC, UINT, UINT, PALETTEENTRY*) { return 0; }
inline int      GetDIBits(HDC, HBITMAP, UINT, UINT, void*, BITMAPINFO*, UINT) { return 0; }
inline int      SetDIBits(HDC, HBITMAP, UINT, UINT, const void*, const BITMAPINFO*, UINT) { return 0; }
inline HBITMAP  CreateDIBitmap(HDC, const BITMAPINFOHEADER*, DWORD, const void*, const BITMAPINFO*, UINT) { return nullptr; }
inline HBITMAP  CreateCompatibleBitmap(HDC, int, int) { return nullptr; }
inline int      GetObjectA(HGDIOBJ, int, void*) { return 0; }
#ifndef GetObject
#define GetObject GetObjectA
#endif
#endif // KO_GDI_FONT_DEFINED

//
// --- lstrcmpi (büyük/küçük harf duyarsız karşılaştırma) --------------------
//
#ifndef KO_LSTRCMPI_DEFINED
#define KO_LSTRCMPI_DEFINED
inline int lstrcmpiA(const char* a, const char* b)
{
	return ::strcasecmp(a ? a : "", b ? b : "");
}
#ifndef lstrcmpi
#define lstrcmpi lstrcmpiA
#endif
inline int lstrcmpA(const char* a, const char* b)
{
	return ::strcmp(a ? a : "", b ? b : "");
}
#ifndef lstrcmp
#define lstrcmp lstrcmpA
#endif
#endif

//
// --- CRT path API'leri + process/sleep (N3Eng init için) -------------------
//
#ifndef KO_PATH_PROC_DEFINED
#define KO_PATH_PROC_DEFINED
#include <unistd.h>
#include <mach-o/dyld.h>
#include <libgen.h>

#ifndef _MAX_PATH
#define _MAX_PATH   260
#define _MAX_DRIVE  3
#define _MAX_DIR    256
#define _MAX_FNAME  256
#define _MAX_EXT    256
#endif

// _splitpath: bir yolu sürücü/dizin/dosya/uzantı parçalarına ayırır.
// macOS'ta sürücü kavramı yok → drive boş bırakılır; dir, yol ayracına kadar olan kısım.
inline void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext)
{
	if (drive != nullptr) drive[0] = '\0';
	if (dir   != nullptr) dir[0]   = '\0';
	if (fname != nullptr) fname[0] = '\0';
	if (ext   != nullptr) ext[0]   = '\0';
	if (path == nullptr) return;

	const char* slash = ::strrchr(path, '/');
	const char* base  = slash ? slash + 1 : path;
	if (dir != nullptr && slash != nullptr)
	{
		size_t n = static_cast<size_t>(base - path);
		::memcpy(dir, path, n);
		dir[n] = '\0';
	}
	const char* dot = ::strrchr(base, '.');
	if (fname != nullptr)
	{
		size_t n = dot ? static_cast<size_t>(dot - base) : ::strlen(base);
		::memcpy(fname, base, n);
		fname[n] = '\0';
	}
	if (ext != nullptr && dot != nullptr)
		::strcpy(ext, dot);
}

// _makepath: parçalardan yol birleştirir (_splitpath'in tersi).
inline void _makepath(char* path, const char* drive, const char* dir,
	const char* fname, const char* ext)
{
	if (path == nullptr) return;
	path[0] = '\0';
	if (drive != nullptr && drive[0] != '\0') ::strcat(path, drive);
	if (dir   != nullptr && dir[0]   != '\0') ::strcat(path, dir);
	if (fname != nullptr) ::strcat(path, fname);
	if (ext   != nullptr && ext[0]   != '\0')
	{
		if (ext[0] != '.') ::strcat(path, ".");
		::strcat(path, ext);
	}
}

// GetModuleFileName: çalışan binary'nin tam yolunu döndürür (macOS: _NSGetExecutablePath).
inline DWORD GetModuleFileNameA(void* /*hModule*/, char* lpFilename, DWORD nSize)
{
	if (lpFilename == nullptr || nSize == 0) return 0;
	uint32_t size = nSize;
	if (_NSGetExecutablePath(lpFilename, &size) != 0)
	{
		lpFilename[0] = '\0';
		return 0;
	}
	return static_cast<DWORD>(::strlen(lpFilename));
}
#ifndef GetModuleFileName
#define GetModuleFileName GetModuleFileNameA
#endif

inline void Sleep(DWORD ms) { ::usleep(static_cast<useconds_t>(ms) * 1000); }

// GetTickCount / GetTickCount64 — açılıştan beri ms (mmsystem.h timeGetTime ile aynı kaynak).
inline uint64_t KO_TickMs()
{
	static mach_timebase_info_data_t tb = {0, 0};
	if (tb.denom == 0) mach_timebase_info(&tb);
	return (mach_absolute_time() * tb.numer / tb.denom) / 1000000ULL;
}
inline DWORD    GetTickCount()   { return static_cast<DWORD>(KO_TickMs()); }
inline uint64_t GetTickCount64() { return KO_TickMs(); }

// GetLastError / SetLastError — errno tabanlı.
inline DWORD GetLastError() { return static_cast<DWORD>(errno); }
inline void  SetLastError(DWORD e) { errno = static_cast<int>(e); }

#ifndef LPWORD
typedef WORD* LPWORD;
#endif

inline BOOL GetClientRect(HWND, RECT* rc)
{
	if (rc != nullptr) { rc->left = 0; rc->top = 0; rc->right = 0; rc->bottom = 0; }
	return TRUE;
}
#endif // KO_PATH_PROC_DEFINED

//
// --- Win32 mesaj döngüsü stub'ları (N3Eng::WaitForDeviceRestoration) -------
//
// Gerçek mesaj pompası Faz 2'de SDL olay döngüsüyle değişecek. Şimdilik
// "mesaj yok" döndürerek döngülerin güvenle ilerlemesini sağlarız.
//
#ifndef KO_MSGLOOP_DEFINED
#define KO_MSGLOOP_DEFINED
#ifndef KO_WPARAM_DEFINED
#define KO_WPARAM_DEFINED
typedef uintptr_t WPARAM;
typedef intptr_t  LPARAM;
typedef intptr_t  LRESULT;
typedef intptr_t  LONG_PTR;
#endif
#ifndef PM_REMOVE
#define PM_NOREMOVE 0x0000
#define PM_REMOVE   0x0001
#endif
typedef struct tagMSG
{
	HWND   hwnd;
	UINT   message;
	WPARAM wParam;
	LPARAM lParam;
	DWORD  time;
	POINT  pt;
} MSG, *LPMSG;

inline BOOL PeekMessageA(MSG*, HWND, UINT, UINT, UINT) { return FALSE; }
#ifndef PeekMessage
#define PeekMessage PeekMessageA
#endif
inline BOOL TranslateMessage(const MSG*) { return FALSE; }
inline LRESULT DispatchMessageA(const MSG*) { return 0; }
#ifndef DispatchMessage
#define DispatchMessage DispatchMessageA
#endif
#endif // KO_MSGLOOP_DEFINED

#endif // !defined(_WIN32)

#endif // KO_PLATFORM_WIN32_API_COMPAT_H_

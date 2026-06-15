// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// windows.h (JpegFile'a özel YALIN şim) — JpegFile.cpp libjpeg + Windows GDI
// karışımıdır. dxvk-native'in windows.h'ı libjpeg'in `boolean { FALSE, TRUE }`
// enum'uyla çakıştığı için, burada SADECE JpegFile'ın ihtiyaç duyduğu GDI
// tip/fonksiyon/sabitlerini sağlıyoruz ve TRUE/FALSE'u TANIMLAMIYORUZ (libjpeg'e
// bırakıyoruz). Yalnızca JpegFile kütüphanesi derlenirken include path'in BAŞINDA
// yer alır; dxvk windows.h bu TU'ya hiç girmez.
//
// GDI fonksiyonları stub'tır — JpegFile'ın GERÇEK işi (ekran görüntüsü/DIB) macOS'ta
// Windows'a özgüdür ve oynamak için gerekli değildir. Asıl JPEG decode (libjpeg)
// çalışır.

#ifndef KO_JPEG_WINDOWS_SHIM_H_
#define KO_JPEG_WINDOWS_SHIM_H_

#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>

typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t  LONG;
typedef int      BOOL;
typedef unsigned int UINT;
typedef char*    LPSTR;
typedef const char* LPCSTR;
typedef void*    LPVOID;
typedef void*    HANDLE;
typedef void*    HBITMAP;
typedef void*    HPALETTE;
typedef void*    HDC;
typedef void*    HWND;
typedef void*    HGDIOBJ;
typedef BYTE*    LPBYTE;

// NOT: Bu şim YALNIZCA stub JpegFile_mac.cpp tarafından kullanılır (libjpeg YOK),
// bu yüzden TRUE/FALSE'u güvenle tanımlayabiliriz.
#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

typedef struct tagRECT { LONG left, top, right, bottom; } RECT, *LPRECT;
typedef struct tagPOINT { LONG x, y; } POINT;

typedef struct tagRGBQUAD { BYTE rgbBlue, rgbGreen, rgbRed, rgbReserved; } RGBQUAD;
typedef struct tagRGBTRIPLE { BYTE rgbtBlue, rgbtGreen, rgbtRed; } RGBTRIPLE;
typedef struct tagPALETTEENTRY { BYTE peRed, peGreen, peBlue, peFlags; } PALETTEENTRY;

typedef struct tagBITMAP
{
	LONG   bmType, bmWidth, bmHeight, bmWidthBytes;
	WORD   bmPlanes, bmBitsPixel;
	LPVOID bmBits;
} BITMAP;

#pragma pack(push, 2)
typedef struct tagBITMAPFILEHEADER
{
	WORD bfType; DWORD bfSize; WORD bfReserved1, bfReserved2; DWORD bfOffBits;
} BITMAPFILEHEADER;
#pragma pack(pop)

typedef struct tagBITMAPINFOHEADER
{
	DWORD biSize; LONG biWidth, biHeight; WORD biPlanes, biBitCount;
	DWORD biCompression, biSizeImage; LONG biXPelsPerMeter, biYPelsPerMeter;
	DWORD biClrUsed, biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER;

typedef struct tagBITMAPCOREHEADER
{
	DWORD bcSize; WORD bcWidth, bcHeight, bcPlanes, bcBitCount;
} BITMAPCOREHEADER, *LPBITMAPCOREHEADER;

typedef struct tagBITMAPINFO
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD          bmiColors[1];
} BITMAPINFO, *LPBITMAPINFO;

typedef struct tagLOGPALETTE
{
	WORD palVersion, palNumEntries;
	PALETTEENTRY palPalEntry[1];
} LOGPALETTE, *LPLOGPALETTE;

// Sabitler
#define BI_RGB          0
#define DIB_RGB_COLORS  0
#define BITSPIXEL       12
#define PLANES          14
#define NUMCOLORS       24
#define RASTERCAPS      38
#define RC_PALETTE      0x0100
#define SIZEPALETTE     104
#define SRCCOPY         0x00CC0020
#define DEFAULT_PALETTE 15
#define GMEM_FIXED      0x0000
#define GMEM_MOVEABLE   0x0002
#define GMEM_ZEROINIT   0x0040
#define GHND            (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define GPTR            (GMEM_FIXED | GMEM_ZEROINIT)

// Global bellek (malloc tabanlı; handle == pointer).
inline HANDLE GlobalAlloc(UINT f, size_t n) { void* p = ::malloc(n); if (p && (f & GMEM_ZEROINIT)) ::memset(p, 0, n); return p; }
inline LPVOID GlobalLock(HANDLE h) { return h; }
inline BOOL   GlobalUnlock(HANDLE) { return 0; }
inline HANDLE GlobalFree(HANDLE h) { ::free(h); return nullptr; }

// GDI stub'ları (ekran görüntüsü/DIB yolu — macOS'ta no-op).
inline HDC      GetDC(HWND) { return nullptr; }
inline int      ReleaseDC(HWND, HDC) { return 1; }
inline int      GetDeviceCaps(HDC, int) { return 0; }
inline HPALETTE CreatePalette(const LOGPALETTE*) { return nullptr; }
inline HPALETTE SelectPalette(HDC, HPALETTE, BOOL) { return nullptr; }
inline UINT     RealizePalette(HDC) { return 0; }
inline UINT     GetSystemPaletteEntries(HDC, UINT, UINT, PALETTEENTRY*) { return 0; }
inline int      GetDIBits(HDC, HBITMAP, UINT, UINT, void*, BITMAPINFO*, UINT) { return 0; }
inline HDC      CreateCompatibleDC(HDC) { return nullptr; }
inline HBITMAP  CreateCompatibleBitmap(HDC, int, int) { return nullptr; }
inline HGDIOBJ  SelectObject(HDC, HGDIOBJ) { return nullptr; }
inline BOOL     BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD) { return 0; }
inline BOOL     DeleteDC(HDC) { return 1; }
inline BOOL     DeleteObject(HGDIOBJ) { return 1; }
inline BOOL     GetWindowRect(HWND, RECT* r) { if (r) { r->left = r->top = r->right = r->bottom = 0; } return 1; }
inline int      GetObjectA(HGDIOBJ, int, void*) { return 0; }
#ifndef GetObject
#define GetObject GetObjectA
#endif

#endif // KO_JPEG_WINDOWS_SHIM_H_

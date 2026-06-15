// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// d3dx9_tex_shim.h — N3Base'in kullandigi D3DX9 yardimci fonksiyonlarinin shim'i.
//
// dxvk-native cekirdek D3D9'u (d3d9.h / d3d9types.h, IDirect3DTexture9,
// IDirect3DSurface9, D3DFMT_*, vb.) saglar AMA D3DX9 yardimci kutuphanesini
// (d3dx9.h) saglamaz. N3Base yalnizca asagidaki kucuk D3DX9 yuzeyini kullanir;
// bu bashlik o yuzeyi (imzalar + sabitler + yapilar) tanimlar.
//
// Gercek implementasyon d3dx9_tex_shim.cpp icinde TODO olarak isaretlidir.
//
// Cakisma notu: Bu bashlik gercek d3dx9.h ile AYNI ANDA dahil EDILMEMELIDIR.
// Yalnizca _WIN32 disinda (gercek D3DX9 yokken) devreye girer.

#ifndef KO_PLATFORM_D3DX9_TEX_SHIM_H_
#define KO_PLATFORM_D3DX9_TEX_SHIM_H_

#pragma once

#if !defined(_WIN32)

// D3D9 cekirdek tipleri (LPDIRECT3DDEVICE9, LPDIRECT3DTEXTURE9,
// LPDIRECT3DSURFACE9, D3DFORMAT, D3DPOOL, HRESULT, PALETTEENTRY ...)
// dxvk-native tarafindan saglanir.
#include <d3d9.h>

#include "../win-compat/win_types.h"

//
// --- D3DX sabitleri --------------------------------------------------------
//
// D3DX_DEFAULT: "varsayilan deger kullan" niteleyicisi (boyut/mip seviyesi vb.).
//
#ifndef D3DX_DEFAULT
#define D3DX_DEFAULT ((UINT) -1)
#endif

#ifndef D3DX_DEFAULT_NONPOW2
#define D3DX_DEFAULT_NONPOW2 ((UINT) -2)
#endif

//
// D3DX_FILTER_* — yuzey/mipmap olcekleme filtre bayraklari.
// N3Base'de kullanilanlar: NONE, TRIANGLE, MIRROR (bit-OR ile birlestirilir).
//
#ifndef D3DX_FILTER_NONE
#define D3DX_FILTER_NONE     (1 << 0)
#define D3DX_FILTER_POINT    (2 << 0)
#define D3DX_FILTER_LINEAR   (3 << 0)
#define D3DX_FILTER_TRIANGLE (4 << 0)
#define D3DX_FILTER_BOX      (5 << 0)

#define D3DX_FILTER_MIRROR_U (1 << 16)
#define D3DX_FILTER_MIRROR_V (2 << 16)
#define D3DX_FILTER_MIRROR_W (4 << 16)
#define D3DX_FILTER_MIRROR   (7 << 16)
#define D3DX_FILTER_DITHER   (1 << 19)
#endif

//
// --- D3DXIMAGE_INFO --------------------------------------------------------
//
// D3DXCreateTextureFromFileEx tarafindan doldurulur: yuklenen goruntunun
// orijinal metadata'sini tasir. Win32 d3dx9.h'taki ile alan-uyumlu olmalidir.
//
#ifndef _D3DXIMAGE_FILEFORMAT_DEFINED
#define _D3DXIMAGE_FILEFORMAT_DEFINED
typedef enum _D3DXIMAGE_FILEFORMAT
{
	D3DXIFF_BMP         = 0,
	D3DXIFF_JPG         = 1,
	D3DXIFF_TGA         = 2,
	D3DXIFF_PNG         = 3,
	D3DXIFF_DDS         = 4,
	D3DXIFF_PPM         = 5,
	D3DXIFF_DIB         = 6,
	D3DXIFF_HDR         = 7,
	D3DXIFF_PFM         = 8,
	D3DXIFF_FORCE_DWORD = 0x7fffffff
} D3DXIMAGE_FILEFORMAT;
#endif

#ifndef _D3DXIMAGE_INFO_DEFINED
#define _D3DXIMAGE_INFO_DEFINED
typedef struct _D3DXIMAGE_INFO
{
	UINT Width;
	UINT Height;
	UINT Depth;
	UINT MipLevels;
	D3DFORMAT Format;
	D3DRESOURCETYPE ResourceType;
	D3DXIMAGE_FILEFORMAT ImageFileFormat;
} D3DXIMAGE_INFO;
#endif

//
// --- Fonksiyon imzalari ----------------------------------------------------
//
// Imzalar, Win32 d3dx9.h'taki orijinallerle ve N3Texture.cpp'deki cagrilarla
// birebir uyumludur.
//

// Bir goruntu dosyasindan (PNG/TGA/BMP/DDS ...) doku olusturur.
// N3Texture::LoadFromFile icinde kullanilir.
HRESULT WINAPI D3DXCreateTextureFromFileEx(
	LPDIRECT3DDEVICE9 pDevice,
	const char* pSrcFile,
	UINT Width,
	UINT Height,
	UINT MipLevels,
	DWORD Usage,
	D3DFORMAT Format,
	D3DPOOL Pool,
	DWORD Filter,
	DWORD MipFilter,
	D3DCOLOR ColorKey,
	D3DXIMAGE_INFO* pSrcInfo,
	PALETTEENTRY* pPalette,
	LPDIRECT3DTEXTURE9* ppTexture);

// Bir yuzeyden digerine (gerekirse olcekleyerek/format cevirerek) kopyalar.
// Mipmap uretimi ve format donusumunde kullanilir.
HRESULT WINAPI D3DXLoadSurfaceFromSurface(
	LPDIRECT3DSURFACE9 pDestSurface,
	const PALETTEENTRY* pDestPalette,
	const RECT* pDestRect,
	LPDIRECT3DSURFACE9 pSrcSurface,
	const PALETTEENTRY* pSrcPalette,
	const RECT* pSrcRect,
	DWORD Filter,
	D3DCOLOR ColorKey);

// HRESULT -> insan-okuyabilir hata metni.
// Not: orijinal D3DX9 hem ANSI (A) hem Unicode (W) varyant saglar; burada
// projede kullanilan ANSI varyantini tanimliyoruz.
const char* WINAPI D3DXGetErrorStringA(HRESULT hr, char* pBuffer, UINT BufferLen);

#ifndef D3DXGetErrorString
#define D3DXGetErrorString D3DXGetErrorStringA
#endif

#endif // !defined(_WIN32)

#endif // KO_PLATFORM_D3DX9_TEX_SHIM_H_

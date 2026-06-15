// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// d3dx9_tex_shim.cpp — D3DX9 yardimci fonksiyon shim'inin iskelet (TODO)
// implementasyonu.
//
// Bu dosya yalnizca _WIN32 disinda derlenir. Gercek D3DX9 yoktur; cagrilari
// dxvk-native'in D3D9 cekirdegi + bir goruntu cozucusu (orn. stb_image /
// DirectXTex) uzerine oturmak GEREKIR. Su an tum fonksiyonlar TODO'dur ve
// guvenli bir "basarisiz" deger dondurur.

#if !defined(_WIN32)

#include "d3dx9_tex_shim.h"

#include <cstdio>
#include <cstring>

// D3D9 standart sonuc kodlari (dxvk-native saglar).
#ifndef D3D_OK
#define D3D_OK 0
#endif
#ifndef E_NOTIMPL
#define E_NOTIMPL ((HRESULT) 0x80004001L)
#endif

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
	LPDIRECT3DTEXTURE9* ppTexture)
{
	(void) pDevice;
	(void) pSrcFile;
	(void) Width;
	(void) Height;
	(void) MipLevels;
	(void) Usage;
	(void) Format;
	(void) Pool;
	(void) Filter;
	(void) MipFilter;
	(void) ColorKey;
	(void) pPalette;

	// TODO(macos-port):
	//  1. pSrcFile'i diskten oku (PNG/TGA/BMP/DDS). Onerilen: stb_image veya
	//     DirectXTex (DDS icin). Goruntuyu 32-bit RGBA'ya cevir.
	//  2. pDevice->CreateTexture(...) ile MANAGED bir doku olustur; D3DX_DEFAULT
	//     gelen boyut/mip degerlerini kaynak goruntuden tureterek coz.
	//  3. LockRect / UnlockRect ile piksel verisini (gerekirse Filter ile mipmap
	//     uretimi yaparak) doku seviyelerine kopyala.
	//  4. pSrcInfo doldur (Width/Height/Format/MipLevels/ImageFileFormat).

	if (ppTexture != nullptr)
		*ppTexture = nullptr;

	if (pSrcInfo != nullptr)
		std::memset(pSrcInfo, 0, sizeof(*pSrcInfo));

	return E_NOTIMPL;
}

HRESULT WINAPI D3DXLoadSurfaceFromSurface(
	LPDIRECT3DSURFACE9 pDestSurface,
	const PALETTEENTRY* pDestPalette,
	const RECT* pDestRect,
	LPDIRECT3DSURFACE9 pSrcSurface,
	const PALETTEENTRY* pSrcPalette,
	const RECT* pSrcRect,
	DWORD Filter,
	D3DCOLOR ColorKey)
{
	(void) pDestSurface;
	(void) pDestPalette;
	(void) pDestRect;
	(void) pSrcSurface;
	(void) pSrcPalette;
	(void) pSrcRect;
	(void) Filter;
	(void) ColorKey;

	// TODO(macos-port):
	//  1. Hem kaynak hem hedef yuzeyin GetDesc() ile format/boyutunu al.
	//  2. Ikisini de LockRect ile kilitle.
	//  3. Format ayniysa ve boyut esitse: dogrudan satir-satir kopyala
	//     (pitch'e dikkat).
	//  4. Boyut farkliysa: Filter (TRIANGLE/NONE) ile yeniden olcekle.
	//  5. Format farkliysa: piksel format donusumu uygula (orn. A8R8G8B8 ->
	//     A1R5G5B5 / A4R4G4B4). N3Texture::Save bu yolu kullanir.
	//  6. UnlockRect.

	return E_NOTIMPL;
}

const char* WINAPI D3DXGetErrorStringA(HRESULT hr, char* pBuffer, UINT BufferLen)
{
	// TODO(macos-port): Bilinen D3D/D3DX HRESULT degerlerini metne esle.
	// Simdilik sadece sayisal degeri yaziyoruz.
	if (pBuffer != nullptr && BufferLen > 0)
		std::snprintf(pBuffer, BufferLen, "HRESULT 0x%08X", static_cast<unsigned int>(hr));

	return pBuffer;
}

#endif // !defined(_WIN32)

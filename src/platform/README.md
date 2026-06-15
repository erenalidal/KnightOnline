<!-- macOS port compat layer — see docs/MAC_PORT_ROADMAP.md -->

# macOS / clang Platform Compat Layer

Bu dizin, OpenKO Knight Online istemcisinin macOS (clang) altinda derlenebilmesi
icin gereken *minimal* uyumluluk (compat) katmanini icerir. Amac, mevcut Win32 /
Direct3D 9 bagimli kaynak kodu **mumkun oldugunca degistirmeden** baska bir
platformda derlenebilir ve calistirilabilir hale getirmektir.

> NOT: Bu katman bir taslaktir. Gercek implementasyonlar TODO olarak isaretlidir.
> Yol haritasi ve genel strateji icin `docs/MAC_PORT_ROADMAP.md` dosyasina bakin.

## Strateji

Direct3D 9 API'sinin kendisi (`d3d9.h`, `d3d9types.h`, COM arayuzleri, `D3DFMT_*`,
`D3DFVF_*`, `_D3DCOLORVALUE`, `_D3DMATERIAL9`, `_D3DLIGHT9`, `IDirect3DTexture9`,
`IDirect3DSurface9` vb.) **dxvk-native** tarafindan saglanir. dxvk-native, D3D9
cagrilarini Vulkan/MoltenVK uzerine ceviren ve beraberinde kendi minimal Windows
"base" shim'ini (temel Win32 tip ve makrolar) getiren bir kutuphanedir.

Bu nedenle bu compat katmani **yalnizca dxvk-native'in doldurmadigi bosluklari**
kapsar ve iki alt parcaya ayrilir:

### 1. `win-compat/` — Kucuk Win32 tip/makro bosluklari

dxvk-native kendi Windows base shim'inde `RECT`, `POINT`, `BOOL`, `DWORD`, `HWND`
gibi cogu temel tipi zaten saglar. Buradaki amac **cakismadan** yalnizca eksik
kalan parcalari tamamlamaktir:

- Tum tanimlar `#if !defined(_WIN32)` altindadir (gercek Windows'ta hicbir sey
  yapmaz).
- Her tip/makro `#ifndef` ile korunur; boylece dxvk-native (veya baska bir shim)
  ayni sembolu zaten tanimladiysa **yeniden tanimlama / cakisma olmaz**.
- `QueryPerformanceCounter` / `QueryPerformanceFrequency` icin macOS'a ozgu
  `mach_absolute_time()` tabanli inline bir implementasyon saglanir (dxvk bunlari
  garanti etmez; N3Base zamanlama icin bunlara guvenir).

### 2. `d3dx9-compat/` — D3DX9 yardimci fonksiyon shim'i

dxvk-native cekirdek D3D9'u saglar ama **D3DX9 yardimci kutuphanesini (`d3dx9.h`)
saglamaz**. N3Base yalnizca birkac D3DX9 fonksiyonu kullanir (asagidaki listeye
bakin). Bu alt katman, o fonksiyonlarin imzalarini ve `D3DX_FILTER_*` /
`D3DXIMAGE_INFO` gibi gerekli sabit ve yapilari saglar.

Gercek kullanilan D3DX9 yuzeyi (N3Texture.cpp icinde):

- `D3DXCreateTextureFromFileEx` — dosyadan doku yukleme (PNG/TGA/BMP vb.).
- `D3DXLoadSurfaceFromSurface` — yuzeyler arasi olceklemeli kopyalama (mipmap
  uretimi ve format donusumu icin).
- `D3DXGetErrorString` — HRESULT -> insan-okuyabilir hata metni.
- `D3DXIMAGE_INFO` yapisi ve `D3DX_FILTER_*` / `D3DX_DEFAULT` sabitleri.

Implementasyon ileride stb_image / DirectXTex benzeri bir kutuphane ya da dogrudan
D3D9 yuzey kilitleme ile doldurulacaktir (TODO).

## dxvk-native ile cakisma onleme

- Bu katman D3D9 cekirdek tiplerini (`_D3DCOLORVALUE`, `IDirect3D*` vb.) **asla**
  yeniden tanimlamaz; bunlar dxvk-native'den gelir.
- Tum Win32 tip/makro tanimlari `#ifndef` korumalidir, bu yuzden dxvk shim'i ile
  ikili tanim olusmaz.
- Include sirasi: dxvk-native bashliklari once gelmeli (`<d3d9.h>` vb.), ardindan
  bu compat bashliklari "delik doldurma" amaciyla dahil edilmelidir.

## Dosya yapisi

```
src/platform/
├── README.md                       (bu dosya)
├── win-compat/
│   └── win_types.h                 Win32 tip/makro bosluklari + QPC shim
└── d3dx9-compat/
    ├── d3dx9_tex_shim.h            D3DX9 imza + sabit + yapi tanimlari
    └── d3dx9_tex_shim.cpp          TODO'lu iskelet implementasyon
```

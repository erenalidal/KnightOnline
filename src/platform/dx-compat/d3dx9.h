// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// d3dx9.h — umbrella başlığı.
//
// N3Base kaynak dosyaları `#include <d3dx9.h>` yapar. Windows'ta bu, Microsoft
// DirectX SDK'sının d3dx9.h'ını bulur. macOS/clang'da ise bu dizin
// (src/platform/dx-compat) include path'e eklenir ve <d3dx9.h> buraya çözülür.
//
// Buradan dxvk-native'in çekirdek D3D9 başlığı + bizim D3DX9 yardımcı shim'imiz
// dahil edilir. Böylece kaynak kodda hiçbir #include değişikliği gerekmez.

#ifndef KO_PLATFORM_DXCOMPAT_D3DX9_H_
#define KO_PLATFORM_DXCOMPAT_D3DX9_H_

#pragma once

// dxvk-native d3d9.h (+ windows base shim) ve D3DX9 fonksiyon/sabit/yapı şimi.
#include "../d3dx9-compat/d3dx9_tex_shim.h"

#endif // KO_PLATFORM_DXCOMPAT_D3DX9_H_

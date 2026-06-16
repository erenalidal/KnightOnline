# macOS port — harici bağımlılık yamaları

## dxvk-native-macos.patch

dxvk-native'i (Direct3D 9 → Vulkan) macOS/Apple Silicon'da MoltenVK ile derleyip
çalıştırmak için gereken yamalar. Üst repo (`doitsujin/dxvk`) macOS'u resmi
desteklemediğinden bu yamalar gereklidir.

**Temel alınan dxvk:** `doitsujin/dxvk` master, v2.7.1 (commit 760ec20 civarı).

### Uygulama
```bash
# 1) dxvk + alt modülleri klonla
git clone --depth 1 https://github.com/doitsujin/dxvk.git deps/dxvk-native-src
cd deps/dxvk-native-src
git submodule update --init --depth 1 \
  include/native/directx include/spirv include/vulkan \
  subprojects/dxbc-spirv subprojects/libdisplay-info
git -C subprojects/dxbc-spirv submodule update --init --depth 1

# 2) Yamaları uygula (ana repo + dxbc-spirv submodule)
git apply ../../docs/patches/dxvk-native-macos.patch
git -C subprojects/dxbc-spirv apply ../../../../docs/patches/dxbc-spirv-macos.patch

# 3) Derle (gerekli brew paketleri: meson glslang molten-vk vulkan-headers
#    vulkan-loader sdl2)
meson setup build-mac --buildtype release \
  -Dnative_sdl2=enabled -Dnative_sdl3=disabled -Dnative_glfw=disabled
ninja -C build-mac src/d3d9/libdxvk_d3d9.0.dylib
```

### Yamaların özeti
- `util_win32_compat.h`: `__unix__` → `+ __APPLE__` (LoadLibrary→dlopen macOS'ta da aktif).
- `util_env.cpp`: macOS getExePath (`_NSGetExecutablePath`) + `pthread_setname_np` (1 arg).
- `vulkan_loader.cpp`: macOS Vulkan lib isimleri + Homebrew tam yol fallback.
- `wsi/sdl2/wsi_platform_sdl2.cpp`: SDL2 dylib tam yol fallback (SIP DYLD strip eder).
- `dxvk_instance.cpp`: `VK_KHR_portability_enumeration` + portability flag (MoltenVK şart).
- `dxvk_device_info.cpp`: Apple'da eksik core feature → reddetme yerine uyar (geometryShader
  vb. Metal'de yok; D3D9 gerektirmez).
- `d3d9/meson.build`: macOS'ta `--version-script` atla (ld64 desteklemez).
- `dxvk_context.cpp`: boş vertex binding'e Apple'da dummy buffer (MoltenVK nullDescriptor
  desteklemediğinden VK_NULL_HANDLE çökmeye yol açıyordu).
- `meson.build` + `src/d3d9/shaders/d3d9_fixed_function_frag.glsl`: **renk düzeltmesi.**
  FF shader'ı `t2d`'yi hem `sampler2DShadow` (depth-compare) hem `sampler2D` (renk)
  olarak kullanıyordu; MoltenVK/SPIRV-Cross shadow kullanımını görünce texture'ı
  `depth2d`'ye terfi ettirip renk `sample()`'ını skaler yapıyor → her şey gri.
  macOS'ta `-DDXVK_MACOS` ile shadow yolu kapatıldı, texture renk olarak kalıyor.

## dxbc-spirv-macos.patch (submodule)

- `sm3/sm3_resources.cpp`: yukarıdakinin SM1-3 pixel shader karşılığı — legacy sampler
  lowering'inde Dref (depth-compare) dalını Apple'da emit etme (aynı `depth2d` terfi
  sorununu önler).

Çalıştırma için bkz. `docs/MAC_PORT_ROADMAP.md` ve `scripts/run-mac-client.sh`.

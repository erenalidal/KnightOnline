# OpenKO — macOS Native Port Yol Haritası

> Bu doküman, WarFare client'ını + N3Base motorunu macOS'ta (Apple Silicon, Metal)
> native çalıştırmak için teknik plandır. Server tarafı zaten Mac'te derleniyor;
> bu harita yalnızca **client + motor** portunu kapsar.

## İLERLEME GÜNLÜĞÜ

### 2026-06-15 (5) — 🎉🎉🎉 CLIENT MAC'TE ÇALIŞIYOR — Metal'e render ediyor
**Knight Online client'ı macOS'ta (Apple Silicon) native, dxvk-native→MoltenVK→Metal ile
çalışıyor, stabil, login ekranına ulaşıyor, frame present ediyor.**

dxvk-native macOS portu (deps/dxvk-native-src, yamalar):
- `src/util/util_win32_compat.h`: `__unix__` → `+ __APPLE__` (LoadLibrary→dlopen).
- `src/util/util_env.cpp`: macOS getExePath (_NSGetExecutablePath) + pthread_setname_np (1 arg).
- `src/vulkan/vulkan_loader.cpp`: macOS lib isimleri + Homebrew tam yol fallback.
- `src/wsi/sdl2/wsi_platform_sdl2.cpp`: SDL2 dylib tam yol fallback (SIP DYLD strip).
- `src/dxvk/dxvk_instance.cpp`: VK_KHR_portability_enumeration + flag (MoltenVK şart).
- `src/dxvk/dxvk_device_info.cpp`: Apple'da eksik core feature'ları reddetme yerine uyar
  (geometryShader vb. Metal'de yok; D3D9 gerektirmez).
- `src/d3d9/meson.build`: macOS'ta --version-script atla (ld64 desteklemez).
- `src/dxvk/dxvk_context.cpp`: boş vertex binding'e Apple'da dummy buffer (MoltenVK
  nullDescriptor desteklemediğinden VK_NULL_HANDLE MVKBuffer'da çökerttiriyordu).

Client tarafı son düzeltmeler:
- `src/FileIO/FileReader.cpp` + `FileWriter.cpp`: KO yollarındaki '\' → '/' normalize
  (asset/tmp dosyaları açılabilsin; .tbl tablo şifre-çöz tmp yazma/okuma tutarlılığı).
- `src/platform/win-compat/wincrypt_compat.cpp`: gerçek RC4 + SHA-1 (CommonCrypto) —
  texture şifre çözme.
- `src/platform/mac-entry/mac_window.cpp`: SDL2 Vulkan penceresi (dxvk present hedefi).

ÇALIŞTIRMA (kanıtlanmış):
```
git clone --depth 1 https://github.com/Open-KO/ko-client-assets.git /tmp/ko-assets
scripts/run-mac-client.sh   # VK_ICD_FILENAMES + DXVK_WSI_DRIVER=SDL2 ayarlı
```
- 1024x768 SDL Vulkan penceresi açılıyor; D3D9 cihazı Apple M3'te oluşuyor;
  swapchain + FIFO present; UI yükleniyor; 10sn+ stabil, çökme yok.
- NOT: `screencapture` Terminal'e Ekran Kaydı izni gerektirir (sistem izni); pencere
  kullanıcının ekranında gerçekten görünür.

**Kalan (cila):** asset tam doğrulama, ses, girdi (SDL klavye/fare — şu an stub),
login→server bağlantısı (server zaten Mac'te çalışıyor), .uif tooltip parse'ı.



### 2026-06-15 (4) — 🎉🎉 TÜM CLIENT MAC'TE DERLENDİ + LİNKLENDİ (native arm64 binary)
- **WarFare.Core** (102 .o, 71K satır) + **N3Base** (72 .o) + **JpegFile** (stub) hepsi derlendi.
- **`KnightOnLine`** native arm64 Mach-O binary üretildi (2.6MB), çalışıyor (çökmüyor),
  D3D device stub olduğu için motor init'te düzgünce çıkıyor (exit 255 — beklenen).
- Link yüzeyi şaşırtıcı küçüktü (5 sembol): çoğu D3D çağrısı virtual (vtable) → link
  sembolü gerektirmiyor. Sadece `Direct3DCreate9` + `D3DXCreateTextureFromFileEx` +
  2 JpegFile metodu + `main` gerekti.
- Eklenen Win32 şim yüzeyi (`src/platform/win-compat/`): dinput (DirectInput+54 DIK),
  winsock2 (BSD socket), windowsx, shellapi, io (POSIX dir glob), mmsystem, wincrypt,
  registry/WNDCLASS/mesajlar/GDI/path/string — hepsi #ifndef korumalı.
- `src/platform/mac-entry/mac_platform.cpp`: `main()` (→ WinMain) + geçici
  `Direct3DCreate9` stub (Faz 3'te dxvk-native ile değişecek).
- JpegFile non-Windows'ta stub (`JpegFile_mac.cpp`) — ekran görüntüsü Windows GDI'ya bağlı.

**FAZ 1 TAMAMLANDI: client macOS'ta uçtan uca derleniyor + linkleniyor.**

**Kalan (oynamak için — Faz 2/3, asıl ağır iş):**
1. dxvk-native libdxvk_d3d9'u Mac'te build et (meson + MoltenVK) → gerçek
   `Direct3DCreate9` + D3D9 cihazı. mac_platform.cpp stub'ını kaldır.
2. SDL2 pencere + olay döngüsü (WinMain/Win32 pencere yerine), HWND→SDL_Window.
3. Runtime stub'ları gerçekle: SDL girdi (DirectInput yerine), metin girişi, mesaj pompası.
4. Client asset'lerini çek (ko-client-assets), login→dünya akışını çalışan server'a bağla.



### 2026-06-15 (3) — 🎉 N3BASE MOTORU MAC'TE DERLENDİ
- dxvk-native header'ları (`deps/dxvk-native-src/include/native/{directx,windows}`) +
  `src/platform/` compat şimleri N3Base build'ine bağlandı (`src/N3Base/CMakeLists.txt`,
  `NOT WIN32` dalı). Windows `.lib`'leri + dx9sdk MSVC-only yapıldı.
- `<d3dx9.h>` umbrella (`src/platform/dx-compat/d3dx9.h`) → dxvk d3d9.h + D3DX9 şimi.
- Win32 yüzeyi şimlendi (`src/platform/win-compat/`): win_types, win32_api_compat
  (GDI bitmap, global bellek, GDI font/metin, path/proc/sleep, mesaj-loop),
  win_user32 (pencere/mesaj/IME/VK), mmsystem, wincrypt (STUB), imm.
- **Sonuç:** `libN3Base_client.a` üretildi — 72/72 kaynak derlendi, 0 hata.
- Stub bırakılanlar (Faz 6'da gerçeklenecek): GDI font rasterizer (DFont), Win32 EDIT
  metin girişi (N3UIEdit), WinCrypt RC4/SHA dosya şifre çözme, mesaj pompası.



### 2026-06-15 (2) — SERVER STACK MAC'TE TAM ÇALIŞIYOR + dxvk header doğrulandı
**🎉 Tüm OpenKO server stack'i Apple Silicon'da native + gerçek DB'ye bağlı çalışıyor:**
- SQL Server (Docker, amd64 emülasyon) UP & healthy, `KN_online` DB import edildi.
- DB driver sorunu: msodbcsql18 brew'de CLT-eski hatası verdi → **FreeTDS** ile çözüldü
  (`/opt/homebrew/lib/libtdsodbc.so`). DSN `build-mac/odbc/{odbc.ini,odbcinst.ini}`.
  Server `ODBCSYSINI`/`ODBCINI` env'leriyle DSN'i buluyor.
- **AIServer** (10020): tüm tablolar + 6036 NPC yüklü, "successfully initialized".
- **Ebenezer** (15001 oyun, 2324 telnet): tüm DB tabloları yüklü, AIServer zone 0-9 bağlı,
  "successfully initialized", TCP 15001 bağlantı kabul ediyor.
- **Aujard** (DB agent): shared memory'ye bağlı, "processing requests".
- Başlatıcı: `scripts/run-mac-servers.sh`.
- → Server tarafı için Faz 0 HEDEFİ AŞILDI: Mac'te uçtan uca çalışan server.

**Client/dxvk doğrulaması (kanıtlandı):**
- dxvk (doitsujin/dxvk v2.7.1) klonlandı: `deps/dxvk-native-src`.
- Self-contained header'lar mevcut: `include/native/directx/d3d9.h` + `include/native/windows/`.
- **`#include <d3d9.h>` clang'da DERLENDİ** (probe başarılı) → N3Base header blocker'ı için
  dxvk header'ları çözüm. (Kütüphane build'i ayrı/riskli: meson'da darwin branch yok,
  MoltenVK extension boşlukları olası — bkz. Strateji/Riskler.)
- Gereken brew paketleri (lib build için): meson, glslang, molten-vk, vulkan-headers,
  vulkan-loader, sdl2|sdl3.



### 2026-06-15 — Faz 0 tamamlandı, Faz 1 scope'u çıkarıldı
**KANITLANMIŞ BASELINE — server Mac'te native çalışıyor:**
- Toolchain: cmake 3.31, clang 16, ninja 1.13 (kuruldu), brew ✓
- `cmake --build build-mac` → **5 server da derlendi**, native **arm64 Mach-O**:
  Ebenezer, AIServer, Aujard, ItemManager, VersionManager
- `Ebenezer` çalıştırıldı: logger'lar kalkıyor, config okuyor, MAP yokken düzgün kapanıyor.
  → Server boru hattı (CMake + tüm depler + ODBC/nanodbc) Mac'te SORUNSUZ.

**Faz 1 (client) için somut keşifler:**
- Client CMakeLists'leri zaten var, sadece non-Windows'ta `OPENKO_BUILD_CLIENT OFF` ile kapalı.
- `dx9sdk` paketi = **Microsoft DirectX SDK header'ları** (Open-KO mirror'dan fetch).
  COM/`<windows.h>` bağımlı, Lib'ler Windows `.lib` → **Mac'te derlenmez/linklenmez.**
  → **AKSİYON:** non-Windows'ta `dx9sdk` paketini **dxvk-native** ile değiştir (kendi
  self-contained d3d9.h'ını + windows base shim'ini getirir).
- **D3DX matematiği KULLANILMIYOR** → `src/MathUtils` (Matrix44/Vector3/Vector4/Quaternion)
  zaten var ve taşınabilir. d3dx9'dan sadece ~4 texture fonksiyonu lazım
  (`D3DXCreateTextureFromFileEx`, `D3DXLoadSurfaceFromSurface`, `D3DXGetErrorString`,
  `D3DXIMAGE_INFO`) + birkaç `D3DX_FILTER_*` sabiti → küçük şim.
- Win32 yüzeyi küçük ve şimlenebilir: BOOL/TRUE/FALSE (en sık), RECT, POINT, HRESULT,
  DWORD, ZeroMemory, HWND, MAX_PATH, HDC, QueryPerformance*, LARGE_INTEGER.
- Motor temel tipleri D3D struct'larından TÜRÜYOR (`__ColorValue : public _D3DCOLORVALUE`)
  → d3d9 tip tanımları derleme anında şart (dxvk-native sağlayacak).

**KANITLANAN harvest sonuçları (client configure + N3Base compile denemesi):**
- `OPENKO_EXPERIMENTAL_NONWIN_CLIENT` flag'i eklendi (root CMakeLists). Client Mac'te
  **configure oluyor** (dx9sdk + openal + jpeg fetch edildi, WarFare hedefi üretildi).
- N3Base_client derleme denemesi → ilk blocker: `My_3DStruct.h:6 'd3dx9.h' file not found`.
- **Kesin kanıt:** MS dx9sdk header'ları clang'da derlenmiyor. `d3d9.h` doğrudan
  `objbase.h` (Windows COM) istiyor → macOS'ta yok. Yani Microsoft DirectX SDK header'ları
  kaynak seviyesinde Windows'a bağlı. **dxvk-native zorunlu** (self-contained d3d9.h getirir).

**Hazırlanan compat iskelesi (`src/platform/`):**
- `win-compat/win_types.h` — Win32 tip boşlukları, `#if !defined(_WIN32)` + `#ifndef` korumalı,
  QueryPerformanceCounter/Frequency için `mach_absolute_time` tabanlı inline impl.
- `d3dx9-compat/d3dx9_tex_shim.{h,cpp}` — kullanılan 3 d3dx9 fonksiyonu + sabitleri,
  TODO'lu iskelet (gerçek impl: stb_image/DirectXTex ile yükleme).
- `README.md` — strateji + dxvk windows-base çakışma önleme notları.

**Sıradaki somut iş (Faz 1 devamı — heavy, haftalık):**
1. dxvk-native'i dep olarak vendor'la (meson + glslang build); non-Windows'ta
   `require_config_package(dx9sdk)`'i dxvk-native ile değiştiren CMake dalı yaz.
2. `src/platform/` şimlerini N3Base build'ine bağla, include sırasını dxvk ile uyumla.
3. N3Base'i derle → bir sonraki gerçek hata duvarını topla, fixed-function shader boşluklarını gör.
4. WarFare'i derle, SDL2 pencere/girdi (Faz 2/4) ile birleştir.

---

## 0. Mevcut durum (ölçülmüş gerçekler)

| Şey | Değer | Not |
|---|---|---|
| N3Base motoru | ~45.000 satır | DirectX 9, fixed-function |
| WarFare client | ~71.000 satır | Win32 + DirectInput + Winsock |
| D3D'ye dokunan dosya | 129 | port yüzeyi |
| `SetRenderState`/`SetTextureStageState` | 444 çağrı | **fixed-function pipeline** |
| Global cihaz işaretçisi | `CN3Base::s_lpD3DDev` (65 dosya) | **tek darboğaz = avantaj** |
| Win32 API kullanan dosya | 21 | pencere/mesaj/socket |
| DirectInput kullanan dosya | 6 | girdi |

### Lehimize olan şeyler
- **Build iskeleti hazır:** `src/N3Base/CMakeLists.txt`, `src/Client/WarFare/CMakeLists.txt` vb.
  zaten var; sadece `OPENKO_BUILD_CLIENT OFF` ile Windows-dışında kapatılmışlar.
- **Cross-platform depler hazır:** `openal-soft` (ses), `asio` (ağ), `zlib`, `libjpeg`,
  `mpg123` (mp3), `nanodbc`, `spdlog`. Yani ses/ağ/sıkıştırma katmanı zaten taşınabilir.
- **Tek render darboğazı:** Tüm çizim `CN3Base::s_lpD3DDev` üzerinden geçiyor. Bu, soyutlama
  dikişini (abstraction seam) tek noktada yakalamayı mümkün kılıyor.
- **Apple Silicon little-endian:** Dosya formatları (.n3, .tbl) little-endian; byte-order
  derdi yok. Yalnızca struct packing/alignment'a dikkat.

### Aleyhimize olan şeyler
- **Fixed-function pipeline:** Metal'de yok. 444 render-state kombinasyonunun yaptığı işi
  (ışık, texture blend, fog, alpha) shader'larla yeniden üretmek gerekiyor.
- **MFC araçları (8 editör) portlanmaz** — ama oynamak için gerekmez, kapsam dışı.
- **DX9 konvansiyonları:** sol-el koordinat, row-major matris, clip-space z [0,1] →
  Metal/MoltenVK ile farklar; transform matematiği ayarlanmalı.

---

## Strateji kararı: TEK birleşik kod tabanı (dxvk-native + Vulkan + MoltenVK)

**Karar:** İki ayrı backend maintain etmiyoruz. Hem Windows hem macOS **aynı render yolunu**
kullanır:

```
D3D9 çağrıları (değişmeden)  →  dxvk-native  →  Vulkan
                                                  ├─ Windows: native Vulkan sürücüsü
                                                  └─ macOS:   MoltenVK → Metal
```

Gerekçe: 444 render-state çağrısını ve `s_lpD3DDev` üzerinden konuşan 65 dosyayı **olduğu gibi**
bırakırız; DXVK fixed-function → shader emülasyonunu kendisi yapar. Tek path = tek bakım yükü,
platformlar arası davranış farkı yok. Windows'ta Vulkan her modern GPU'da mevcut.

> Reddedilen alternatif: Windows'u native D3D9'da bırakıp Mac'e ayrı yol açmak (iki backend).
> Daha az risk ama sürekli iki render yolu maintain etmek gerekirdi — bilinçli olarak elenmedi.

### Geçiş güvenliği (regresyon önleme)
Windows hâlâ çalışan tek platform; onu kırmadan geçmek için:
- [ ] Mevcut native D3D9 Windows build'i **referans/fallback** olarak bir CMake flag arkasında
      (`OPENKO_RENDER_NATIVE_D3D9`) geçiş süresince tut.
- [ ] dxvk yolu Windows'ta native ile **görsel parite** sağlayana kadar eski yolu silme.
- [ ] Parite doğrulandıktan sonra native D3D9 yolunu kaldır → tek path kalır.

### Ortak (platform-bağımsız) katmanlar
Pencere + girdi + ağ + ses zaten tek ortak kod olur (SDL2 Windows'ta da çalışır):
- Pencere/olay: SDL2 (her iki platform)
- Girdi: SDL (her iki platform)
- Ağ: asio (her iki platform)
- Ses: OpenAL (zaten her iki platform)

---

## Fazlar

### Faz 1 — Mac'te derlenebilirlik (build getirme)
**Hedef:** N3Base + WarFare, Mac'te (clang) hata vermeden link aşamasına kadar gelsin (çalışması şart değil).
- [ ] Kök `CMakeLists.txt`'te non-Windows için `OPENKO_BUILD_CLIENT` kilidini aç (deneysel flag).
- [ ] Windows-özel header'ları (`<windows.h>`, `<dinput.h>`, `<d3d9.h>`) soyutlama başlıkları
      arkasına al: `platform/` altında shim header'lar.
- [ ] `RECT`, `HWND`, `D3DCOLOR`, `__Vector3` gibi tipler için platform-nötr typedef'ler.
- [ ] Derlemeyi kıran MFC/Win32 sembollerini `#ifdef _WIN32` ile izole et.
- **Çıktı:** "undefined symbol" listesi = gerçek port iş kalemleri.
- **Tahmin:** 3–4 hafta.

### Faz 2 — Pencere + olay döngüsü (Win32 → SDL2)
**Hedef:** Bir pencere açılsın, olay döngüsü dönsün, temiz kapansın.
- [ ] `WarFareMain.cpp`'teki `PeekMessage`/`DispatchMessage` döngüsünü SDL2 olay döngüsüyle değiştir.
- [ ] `CreateMainWindow`/`HWND` → `SDL_Window`.
- [ ] `WM_SOCKETMSG` async socket bildirimini kaldır (Faz 4'te ağ yeniden ele alınacak).
- **Tahmin:** 1–2 hafta.

### Faz 3 — Render (D3D9 → dxvk-native → Vulkan, her iki platform)
**Hedef:** İlk üçgen, sonra ilk sahne — **önce Windows'ta** (Vulkan), sonra Mac'te (MoltenVK).
- [ ] `dxvk-native` + `Vulkan-Loader` (+ macOS'ta `MoltenVK`) dep olarak ekle (CMake FetchContent).
- [ ] `CN3Eng::Init` içindeki `Direct3DCreate9` çağrısını dxvk-native girişine yönlendir.
- [ ] `s_lpD3DDev` cihazını DXVK cihazıyla besle — 65 dosya kod değişmeden çalışmalı (ideal durum).
- [ ] `Present(HWND)` → SDL swapchain sunumu (Vulkan/MoltenVK).
- [ ] **Önce Windows'ta** native D3D9 referansıyla görsel pariteyi doğrula (geçiş güvenliği).
- [ ] DXVK'nın desteklemediği fixed-function yolları tespit et, elle shader ile doldur.
- [ ] Parite tamam → `OPENKO_RENDER_NATIVE_D3D9` fallback'ini ve native D3D9 kodunu kaldır.
- **Risk:** En büyük belirsizlik burada. DXVK'nın D3D9 fixed-function kapsamı kritik.
- **Tahmin:** 3–6 ay (en büyük blok). Windows'ta önce çalıştırmak Mac'ten önce geri bildirim verir.

### Faz 4 — Girdi (DirectInput → SDL)
- [ ] `LocalInput.cpp` klavye (`LPDIRECTINPUTDEVICE8`) → `SDL_GetKeyboardState`.
- [ ] Fare (WM event'leri) → SDL mouse event'leri.
- **Tahmin:** ~1 hafta.

### Faz 5 — Ağ (Winsock → asio/POSIX)
- [ ] `APISocket` Winsock + `WM_SOCKETMSG` modelini, zaten depte olan `asio` ile değiştir.
- [ ] Şifreleme (JvCryption) zaten platform-nötr C++; dokunma.
- **Tahmin:** 1–2 hafta.

### Faz 6 — Ses + font + cila
- [ ] Ses: `N3SndMgr` zaten OpenAL → muhtemelen küçük düzeltmelerle çalışır.
- [ ] Font: `DFont` D3D texture'a bağlı → render katmanı oturunca düzelir.
- [ ] Asset yükleme yollarını (path separator, case-sensitivity) Mac'e göre düzelt.
- **Tahmin:** 2–4 hafta.

### Faz 7 — Entegrasyon: login → dünya
- [ ] Yerel server (Ebenezer/Aujard/AIServer) + Docker SQL Server'ı ayağa kaldır.
- [ ] Login → karakter seç → dünyaya gir akışını uçtan uca çalıştır.
- [ ] Debug, çökme avı, görsel hata düzeltme.
- **Tahmin:** 2–3 ay.

---

## Toplam tahmin
- **Tek deneyimli mühendis (grafik + C++ + Metal/Vulkan):** kaba oynanır hale **~9–15 ay**.
- **Stabil/cilalı:** 1.5–2 yıl.
- **2–3 kişilik ekip:** takvim olarak ~6–9 ay.

## İlk somut adım
Faz 1: kök CMake'te client kilidini deneysel olarak aç, ilk "undefined symbol" listesini çıkar.
Bu liste, gerçek iş kalemlerinin envanteri olur ve sonraki fazları netleştirir.

## Notlar
- Lisans MIT — fork'ta serbestçe değiştir, AI ile geliştir (telif notunu koru).
- Bu port upstream Open-KO'ya gitmez (proje hedefi değil + AI kuralı); fork'ta kalır.
- Hukuki: oyunun orijinal IP/asset'leri 3. tarafın; kişisel/öğrenme kullanımı ≠ halka açık server.

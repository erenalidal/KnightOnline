#!/usr/bin/env bash
# OpenKO — macOS client (KnightOnLine) başlatıcı
#
# Knight Online client'ını macOS'ta dxvk-native + MoltenVK (→Metal) ile çalıştırır.
# Önkoşullar:
#   1. Client derlenmiş: cmake --build build-mac-client --target WarFare
#   2. dxvk d3d9 derlenmiş: deps/dxvk-native-src/build-mac/src/d3d9/libdxvk_d3d9.0.dylib
#   3. Homebrew: molten-vk, vulkan-loader, sdl2, freetds
#   4. Oyun asset'leri (ko-client-assets) bir dizinde (varsayılan: /tmp/ko-assets)
set -e

REPO="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$REPO/build-mac-client/bin/Release/KnightOnLine"
ASSETS="${KO_ASSETS_DIR:-$HOME/ko-assets}"

# dxvk + MoltenVK runtime ortamı
export VK_ICD_FILENAMES="/opt/homebrew/etc/vulkan/icd.d/MoltenVK_icd.json"
export DXVK_WSI_DRIVER=SDL2          # dxvk-native WSI seçimi (zorunlu)
export DXVK_LOG_LEVEL="${DXVK_LOG_LEVEL:-warn}"

if [ ! -f "$BIN" ]; then
  echo "HATA: client binary yok: $BIN"
  echo "Önce derleyin: cmake --build build-mac-client --target WarFare"
  exit 1
fi
if [ ! -d "$ASSETS" ]; then
  echo "HATA: asset dizini yok: $ASSETS"
  echo "Çekin: git clone --depth 1 https://github.com/Open-KO/ko-client-assets.git $ASSETS"
  exit 1
fi

echo "OpenKO client başlatılıyor (assets: $ASSETS)..."
echo "Pencereyi kapatmak için pencereyi kapatın veya Ctrl+C."
cd "$ASSETS"
exec "$BIN"

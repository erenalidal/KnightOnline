// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
// mac_window.h — SDL2 tabanlı pencere + olay pompası (Win32 pencere yerine).
#ifndef KO_PLATFORM_MAC_WINDOW_H_
#define KO_PLATFORM_MAC_WINDOW_H_
#pragma once
#if !defined(_WIN32)
#include <win_types.h>

// SDL2 Vulkan penceresi oluşturur, HWND olarak döndürür (dxvk SDL_Window* bekler).
HWND KO_CreateMacWindow(int width, int height, const char* title);

// SDL olaylarını işler. Pencere kapatıldıysa (SDL_QUIT) true döndürür.
bool KO_PumpMacEvents();

// Pencere ekranda görünür (occluded değil) mi? Çağıran debounce uygulayıp occluded'ken
// render'ı atlar (drawable starvation/siyah/donma önler); frontmost'ta hep görünür döner.
bool KO_MacWindowVisible();
// Render atlanırken döngünün CPU yakmaması için kısa uyku.
void KO_MacIdleSleep();
#endif
#endif

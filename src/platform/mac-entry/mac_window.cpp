// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// mac_window.cpp — SDL2 pencere + olay döngüsü (Win32 pencere/mesaj sistemi yerine).
//
// dxvk-native'in SDL2 WSI'si, D3D9 cihazına verilen "HWND"yi doğrudan SDL_Window*
// olarak yorumlar. Bu yüzden pencereyi SDL_WINDOW_VULKAN ile oluşturup işaretçisini
// HWND olarak döndürüyoruz; motor onu present hedefi olarak dxvk'ya geçiriyor.

#include "mac_window.h"

#include <SDL.h>
#include <cstdio>

static SDL_Window* g_macWindow = nullptr;

HWND KO_CreateMacWindow(int width, int height, const char* title)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
	{
		std::fprintf(stderr, "[mac] SDL_Init failed: %s\n", SDL_GetError());
		return nullptr;
	}

	if (width  <= 0) width  = 1024;
	if (height <= 0) height = 768;

	g_macWindow = SDL_CreateWindow(
		title != nullptr ? title : "Knight OnLine (macOS)",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height,
		SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);

	if (g_macWindow == nullptr)
	{
		std::fprintf(stderr, "[mac] SDL_CreateWindow failed: %s\n", SDL_GetError());
		return nullptr;
	}

	std::fprintf(stderr, "[mac] SDL Vulkan penceresi oluşturuldu: %dx%d\n", width, height);
	return reinterpret_cast<HWND>(g_macWindow);
}

bool KO_PumpMacEvents()
{
	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		if (ev.type == SDL_QUIT)
			return true;
		if (ev.type == SDL_WINDOWEVENT
			&& ev.window.event == SDL_WINDOWEVENT_CLOSE)
			return true;
	}
	return false;
}

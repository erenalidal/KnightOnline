// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// mac_window.cpp — SDL2 pencere + olay döngüsü (Win32 pencere/mesaj sistemi yerine).
//
// dxvk-native'in SDL2 WSI'si, D3D9 cihazına verilen "HWND"yi doğrudan SDL_Window*
// olarak yorumlar. Bu yüzden pencereyi SDL_WINDOW_VULKAN ile oluşturup işaretçisini
// HWND olarak döndürüyoruz; motor onu present hedefi olarak dxvk'ya geçiriyor.

#include "mac_window.h"
#include "mac_input.h"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

static SDL_Window* g_macWindow = nullptr;

// --- Metin girişi (EDIT) durumu ---
static int         g_editSentinel = 0;          // adresi EDIT handle olarak kullanılır
static std::string g_editText;
static bool        g_editActive   = false;
static bool        g_editReturn   = false;
static bool        g_editTab      = false;
static const size_t kEditMaxLen   = 255;

// --- Fare buton durumu (olay-tabanlı latch) ---
// SDL_GetMouseState ile POLL etmek, down+up aynı pump'ta gelirse tıklamayı kaçırır
// (KO kenar-tetiklemeli algılar). Bunun yerine buton olaylarını işleriz; bir pump'ta
// gelen DOWN'u o frame "basılı" raporlarız ki tıklama asla kaçmasın.
static uint32_t    g_mouseButtons       = 0; // o an basılı butonlar (SDL_BUTTON_*MASK)
static uint32_t    g_mousePressedInPump = 0; // bu pump'ta DOWN gelen butonlar

// Oyun-içi Exit/çıkış: Win32'de PostQuitMessage(0) WM_QUIT yollar; macOS'ta o şim boştu, bu yüzden
// Exit çalışmıyordu. Şimdi PostQuitMessage → KO_Mac_RequestQuit() bu bayrağı set eder, ana döngü
// KO_PumpMacEvents'ten true alıp temiz çıkar.
static bool        g_quitRequested      = false;
void KO_Mac_RequestQuit() { g_quitRequested = true; }

static void EditBackspaceUtf8()
{
	if (g_editText.empty())
		return;
	// UTF-8 devam baytlarını (10xxxxxx) da temizle ki çok-baytlı karakter bozulmasın.
	size_t i = g_editText.size();
	do { --i; } while (i > 0 && (static_cast<unsigned char>(g_editText[i]) & 0xC0) == 0x80);
	g_editText.erase(i);
}

// mac_occlusion.mm — pencere occluded değil mi (1=görünür). C++ linkage (.mm tanımıyla aynı).
int KO_Mac_NSWindowVisible(void* sdlWindow);
// mac_occlusion.mm — ekran >65Hz ise 60Hz'e zorlar (ProMotion/120Hz oyunu kilitliyor); çıkışta geri.
int KO_Mac_ForceLowRefreshIfNeeded();

bool KO_MacWindowVisible()
{
	return KO_Mac_NSWindowVisible(g_macWindow) != 0;
}

void KO_MacIdleSleep()
{
	SDL_Delay(15); // occluded'ken render yok → CPU boş-dönmesin
}

HWND KO_CreateMacWindow(int width, int height, const char* title)
{
	// NOT: Eski 60Hz-zorlama workaround'u KALDIRILDI. Eskiden GPU ~9.5ms/frame idi (triangle-fan
	// → MoltenVK render-pass restart/tile-flush yüzünden) ve 120Hz'in 8.3ms bütçesine sığmıyordu;
	// drawable starvation oyunu kilitliyordu. Fan→list düzeltmesinden sonra GPU 0.5-2.5ms'ye düştü,
	// 120Hz/ProMotion'a rahat sığıyor. Artık ekran yenileme hızına dokunmuyoruz (kullanıcının
	// ProMotion ayarı geçerli). Bkz. N3FanFix. (KO_Mac_ForceLowRefreshIfNeeded artık çağrılmıyor.)

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
	{
		std::fprintf(stderr, "[mac] SDL_Init failed: %s\n", SDL_GetError());
		return nullptr;
	}

	// Pencereye odak veren tıklama, app'e de geçsin (yoksa macOS'ta arka plandaki
	// pencerenin İLK tıklaması yutulur, yalnızca odaklar). Tıklama güvenilirliği için.
	SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

	if (width  <= 0) width  = 1024;
	if (height <= 0) height = 768;

	// Tam ekran: KO_FULLSCREEN=1 → BORDERLESS ekranı kaplayan pencere (gerçek fullscreen DEĞİL).
	// macOS gerçek-fullscreen (FULLSCREEN_DESKTOP/Space) yolu, app-switch'te MoltenVK'nın
	// drawable/colorspace durumunu sıfırlayıp renkleri bozuyordu. Kenarlıksız büyük pencere
	// "windowed" sayıldığından o compositing yoluna girmez → windowed gibi stabil (renk/flicker
	// sorunu yok). Boyut = ekran sınırları (SDL_GetDisplayBounds, nokta cinsinden). Çağıran
	// (WarFareMain) gerçek boyutu s_Options.iViewWidth/Height'a yazar → backbuffer=pencere=tıklama.
	const char* fsEnv      = std::getenv("KO_FULLSCREEN");
	bool        fullscreen = (fsEnv != nullptr && fsEnv[0] == '1');

	int winX = SDL_WINDOWPOS_CENTERED, winY = SDL_WINDOWPOS_CENTERED;
	uint32_t flags = SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN;
	if (fullscreen)
	{
		SDL_Rect bounds;
		if (SDL_GetDisplayBounds(0, &bounds) == 0)
		{
			winX = bounds.x; winY = bounds.y;
			width = bounds.w; height = bounds.h;
		}
		flags |= SDL_WINDOW_BORDERLESS;
	}

	g_macWindow = SDL_CreateWindow(
		title != nullptr ? title : "Knight OnLine (macOS)",
		winX, winY,
		width, height,
		flags);

	if (g_macWindow == nullptr)
	{
		std::fprintf(stderr, "[mac] SDL_CreateWindow failed: %s\n", SDL_GetError());
		return nullptr;
	}

	if (fullscreen)
	{
		int fw = 0, fh = 0;
		SDL_GetWindowSize(g_macWindow, &fw, &fh);
		std::fprintf(stderr, "[mac] borderless tam ekran: %dx%d\n", fw, fh);
	}

	int dw = 0, dh = 0;
	SDL_Vulkan_GetDrawableSize(g_macWindow, &dw, &dh);
	std::fprintf(stderr, "[mac] SDL Vulkan penceresi: window=%dx%d drawable(px)=%dx%d (retina ölçek %.1f)\n",
		width, height, dw, dh, height > 0 ? (float) dh / height : 1.0f);
	return reinterpret_cast<HWND>(g_macWindow);
}

bool KO_PumpMacEvents()
{
	if (g_quitRequested) // oyun-içi Exit (PostQuitMessage) istendi
		return true;

	g_mousePressedInPump = 0; // her frame sıfırla; bu pump'taki DOWN'ları topla

	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		if (ev.type == SDL_QUIT)
			return true;
		if (ev.type == SDL_WINDOWEVENT
			&& ev.window.event == SDL_WINDOWEVENT_CLOSE)
			return true;

		if (ev.type == SDL_MOUSEBUTTONDOWN)
		{
			uint32_t m = SDL_BUTTON(ev.button.button);
			g_mouseButtons       |= m;
			g_mousePressedInPump |= m; // down+up tek pump'ta olsa bile bu frame basılı say
		}
		else if (ev.type == SDL_MOUSEBUTTONUP)
		{
			g_mouseButtons &= ~SDL_BUTTON(ev.button.button);
		}

		if (g_editActive)
		{
			if (ev.type == SDL_TEXTINPUT)
			{
				if (g_editText.size() + std::strlen(ev.text.text) <= kEditMaxLen)
					g_editText += ev.text.text;
			}
			else if (ev.type == SDL_KEYDOWN)
			{
				switch (ev.key.keysym.sym)
				{
					case SDLK_BACKSPACE: EditBackspaceUtf8(); break;
					case SDLK_RETURN:
					case SDLK_KP_ENTER:  g_editReturn = true; break;
					case SDLK_TAB:       g_editTab = true; break;
					default: break;
				}
			}
		}
	}
	return false;
}

// --- Metin girişi köprüsü ---
HWND KO_Mac_EditHandle() { return reinterpret_cast<HWND>(&g_editSentinel); }

void KO_Mac_EditSetText(const char* s) { g_editText = (s != nullptr) ? s : ""; }

int KO_Mac_EditGetText(char* buf, int n)
{
	if (buf == nullptr || n <= 0)
		return 0;
	int len = (int) g_editText.size();
	if (len > n - 1)
		len = n - 1;
	std::memcpy(buf, g_editText.data(), len);
	buf[len] = '\0';
	return len;
}

int KO_Mac_EditGetTextLength() { return (int) g_editText.size(); }

void KO_Mac_EditSetActive(int active)
{
	bool a = (active != 0);
	if (a == g_editActive)
		return;
	g_editActive = a;
	if (a)
		SDL_StartTextInput();
	else
		SDL_StopTextInput();
	g_editReturn = false;
	g_editTab    = false;
}

int KO_Mac_EditGetCaret() { return (int) g_editText.size(); } // caret hep sonda

int KO_Mac_EditConsumeReturn() { int r = g_editReturn ? 1 : 0; g_editReturn = false; return r; }
int KO_Mac_EditConsumeTab()    { int r = g_editTab ? 1 : 0; g_editTab = false; return r; }

// ---------------------------------------------------------------------------
// SDL girdi köprüsü (mac_input.h) — Win32/DirectInput şimleri bunları çağırır.
// SDL durumu KO_PumpMacEvents'in SDL_PollEvent'i sayesinde her frame tazelenir.
// ---------------------------------------------------------------------------

int KO_Mac_GetAsyncKeyState(int vk)
{
	switch (vk)
	{
		// Latch'lenmiş durum: o an basılı VEYA bu pump'ta basılmış (kaçan hızlı tık yok).
		case 0x01: return ((g_mouseButtons | g_mousePressedInPump) & SDL_BUTTON_LMASK) ? 0x8000 : 0; // VK_LBUTTON
		case 0x02: return ((g_mouseButtons | g_mousePressedInPump) & SDL_BUTTON_RMASK) ? 0x8000 : 0; // VK_RBUTTON
		case 0x04: return ((g_mouseButtons | g_mousePressedInPump) & SDL_BUTTON_MMASK) ? 0x8000 : 0; // VK_MBUTTON
		default: break;
	}

	const Uint8* ks = SDL_GetKeyboardState(nullptr);
	if (ks == nullptr)
		return 0;

	switch (vk)
	{
		case 0x10: return (ks[SDL_SCANCODE_LSHIFT] || ks[SDL_SCANCODE_RSHIFT]) ? 0x8000 : 0; // VK_SHIFT
		case 0x11: return (ks[SDL_SCANCODE_LCTRL]  || ks[SDL_SCANCODE_RCTRL])  ? 0x8000 : 0; // VK_CONTROL
		case 0x12: return (ks[SDL_SCANCODE_LALT]   || ks[SDL_SCANCODE_RALT])   ? 0x8000 : 0; // VK_MENU
		case 0x0D: return ks[SDL_SCANCODE_RETURN]  ? 0x8000 : 0; // VK_RETURN
		case 0x1B: return ks[SDL_SCANCODE_ESCAPE]  ? 0x8000 : 0; // VK_ESCAPE
		case 0x20: return ks[SDL_SCANCODE_SPACE]   ? 0x8000 : 0; // VK_SPACE
		case 0x25: return ks[SDL_SCANCODE_LEFT]    ? 0x8000 : 0; // VK_LEFT
		case 0x26: return ks[SDL_SCANCODE_UP]      ? 0x8000 : 0; // VK_UP
		case 0x27: return ks[SDL_SCANCODE_RIGHT]   ? 0x8000 : 0; // VK_RIGHT
		case 0x28: return ks[SDL_SCANCODE_DOWN]    ? 0x8000 : 0; // VK_DOWN
		default: break;
	}
	return 0;
}

void KO_Mac_GetCursorPos(POINT* p)
{
	if (p == nullptr)
		return;
	int x = 0, y = 0;
	SDL_GetMouseState(&x, &y); // odaklı pencereye göreli = istemci koordinatları
	p->x = x;
	p->y = y;
}

void KO_Mac_SetCursorPos(int x, int y)
{
	// Çağıran istemci koordinatı verir (mac'te ClientToScreen kimliktir). SDL_WarpMouseInWindow
	// de pencere-istemci uzayını kullanır → KO_Mac_GetCursorPos ile aynı uzay.
	if (g_macWindow != nullptr)
		SDL_WarpMouseInWindow(g_macWindow, x, y);
}

void KO_Mac_GetClientSize(int* w, int* h)
{
	int ww = 0, hh = 0;
	if (g_macWindow != nullptr)
		SDL_GetWindowSize(g_macWindow, &ww, &hh); // nokta cinsinden (SDL fare ile aynı uzay)
	if (w != nullptr)
		*w = ww;
	if (h != nullptr)
		*h = hh;
}

void KO_Mac_FillDIKeyboard(unsigned char* buf, int count)
{
	if (buf == nullptr || count <= 0)
		return;
	std::memset(buf, 0, count);

	const Uint8* ks = SDL_GetKeyboardState(nullptr);
	if (ks == nullptr)
		return;

	// DIK_* (DirectInput tarama kodu) -> SDL_Scancode eşlemesi (dinput.h'deki set).
	struct DikMap { SDL_Scancode sc; int dik; };
	static const DikMap kMap[] = {
		{ SDL_SCANCODE_ESCAPE, 0x01 }, { SDL_SCANCODE_1, 0x02 }, { SDL_SCANCODE_2, 0x03 },
		{ SDL_SCANCODE_3, 0x04 }, { SDL_SCANCODE_4, 0x05 }, { SDL_SCANCODE_5, 0x06 },
		{ SDL_SCANCODE_6, 0x07 }, { SDL_SCANCODE_7, 0x08 }, { SDL_SCANCODE_8, 0x09 },
		{ SDL_SCANCODE_TAB, 0x0F }, { SDL_SCANCODE_Q, 0x10 }, { SDL_SCANCODE_W, 0x11 },
		{ SDL_SCANCODE_E, 0x12 }, { SDL_SCANCODE_R, 0x13 }, { SDL_SCANCODE_T, 0x14 },
		{ SDL_SCANCODE_U, 0x16 }, { SDL_SCANCODE_I, 0x17 }, { SDL_SCANCODE_RETURN, 0x1C },
		{ SDL_SCANCODE_LCTRL, 0x1D }, { SDL_SCANCODE_A, 0x1E }, { SDL_SCANCODE_S, 0x1F },
		{ SDL_SCANCODE_D, 0x20 }, { SDL_SCANCODE_F, 0x21 }, { SDL_SCANCODE_H, 0x23 },
		{ SDL_SCANCODE_K, 0x25 }, { SDL_SCANCODE_Z, 0x2C }, { SDL_SCANCODE_X, 0x2D },
		{ SDL_SCANCODE_C, 0x2E }, { SDL_SCANCODE_V, 0x2F }, { SDL_SCANCODE_B, 0x30 },
		{ SDL_SCANCODE_M, 0x32 }, { SDL_SCANCODE_F1, 0x3B }, { SDL_SCANCODE_F2, 0x3C },
		{ SDL_SCANCODE_F3, 0x3D }, { SDL_SCANCODE_F4, 0x3E }, { SDL_SCANCODE_F5, 0x3F },
		{ SDL_SCANCODE_F6, 0x40 }, { SDL_SCANCODE_F7, 0x41 }, { SDL_SCANCODE_F8, 0x42 },
		{ SDL_SCANCODE_F9, 0x43 }, { SDL_SCANCODE_F10, 0x44 }, { SDL_SCANCODE_KP_MINUS, 0x4A },
		{ SDL_SCANCODE_F11, 0x57 }, { SDL_SCANCODE_F12, 0x58 }, { SDL_SCANCODE_KP_ENTER, 0x9C },
		{ SDL_SCANCODE_RCTRL, 0x9D }, { SDL_SCANCODE_HOME, 0xC7 }, { SDL_SCANCODE_UP, 0xC8 },
		{ SDL_SCANCODE_PAGEUP, 0xC9 }, { SDL_SCANCODE_LEFT, 0xCB }, { SDL_SCANCODE_RIGHT, 0xCD },
		{ SDL_SCANCODE_END, 0xCF }, { SDL_SCANCODE_DOWN, 0xD0 }, { SDL_SCANCODE_PAGEDOWN, 0xD1 },
	};

	for (const DikMap& m : kMap)
	{
		if (m.dik < count && ks[m.sc])
			buf[m.dik] = 0x80;
	}
}

HWND KO_Mac_GetActiveWindow()
{
	// CLocalInput::Tick, GetActiveWindow()!=m_hWnd ise O FRAME TÜM girdiyi atlar
	// (fare+klavye). Vulkan penceresinde SDL'in INPUT_FOCUS bayrağı odak geçişlerinde
	// kararsız olabiliyor → oynarken bile girdi düşüyor (tıklama "ilk seferde almıyor" +
	// gecikme hissi). Fare pencere üstündeyse (MOUSE_FOCUS) de aktif say → girdi düşmez.
	if (g_macWindow == nullptr)
		return nullptr;
	if (SDL_GetWindowFlags(g_macWindow) & (SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS))
		return reinterpret_cast<HWND>(g_macWindow);
	// SDL INPUT_FOCUS, pencere öne getirilse (frontmost) bile bazen geç set ediliyor →
	// klavye/DirectInput Tick'i o frame girdiyi atlıyor (tuşlar düşüyor). Pencere GÖRÜNÜR
	// (occluded/minimize değil) ise aktif say: klavye olayı zaten yalnızca öndeki pencereye
	// gelir, dolayısıyla bu güvenli ve odak-geçişi girdi düşmesini önler.
	if (KO_Mac_NSWindowVisible(g_macWindow) != 0)
		return reinterpret_cast<HWND>(g_macWindow);
	return nullptr;
}

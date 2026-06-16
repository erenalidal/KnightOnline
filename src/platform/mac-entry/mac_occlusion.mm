// mac_occlusion.mm — macOS port: pencere occlusion (görünürlük) durumu (Objective-C++).
//
// macOS occluded (örtülü/arka plan) pencereyi composite ETMEZ → MoltenVK'nın present ettiği
// CAMetalDrawable'lar ekranda gösterilmez → havuza dönmez → nextDrawable tükenir (~1sn timeout,
// Metal System Trace ile doğrulandı) → present kilitlenir/siyaha düşer (flicker/donma) ve öne
// dönünce de kurtulamayabilir. ÇÖZÜM: occluded'ken render/present atla (drawable acquire etme).
//
// Frontmost'ta occlusionState GÜVENİLİR biçimde Visible döner; çağıran taraf yine de
// debounce uygular (kısa anlık glitch'lerde frontmost render'ı kesmesin diye).
//
// See docs/MAC_PORT_ROADMAP.md.

#if defined(__APPLE__)

#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>

#include <SDL.h>
#include <SDL_syswm.h>

#include <csignal>
#include <cstdlib>

// --- Ekran yenileme hızı zorlaması (ProMotion/120Hz throttle workaround) ---
// Bu oyunun GPU süresi ~9.5ms/frame → 120Hz'in 8.3ms bütçesine sığmıyor → 120Hz'de
// CAMetalDrawable starvation → oyun KİLİTLENİYOR (oynanamaz). 60Hz'in 16.7ms bütçesine
// sığıyor → stabil 60 FPS. Bu yüzden başlangıçta ekran >65Hz ise 60Hz'e alır, çıkışta
// orijinaline döndürür. Kullanıcı zaten 60Hz'deyse hiçbir şey yapmaz (no-op).
namespace
{
CGDirectDisplayID g_forcedDisplay = 0;
CGDisplayModeRef  g_savedMode     = nullptr;
bool              g_didForce      = false;

void RestoreRefreshRate()
{
	if (g_didForce && g_savedMode != nullptr)
	{
		CGDisplaySetDisplayMode(g_forcedDisplay, g_savedMode, nullptr);
		CGDisplayModeRelease(g_savedMode);
		g_savedMode = nullptr;
		g_didForce  = false;
	}
}

void SignalRestoreHandler(int sig)
{
	RestoreRefreshRate();
	std::signal(sig, SIG_DFL);
	std::raise(sig);
}
} // namespace

int KO_Mac_ForceLowRefreshIfNeeded()
{
	CGDirectDisplayID did = CGMainDisplayID();
	CGDisplayModeRef  cur = CGDisplayCopyDisplayMode(did);
	if (cur == nullptr)
		return 0;

	double curHz = CGDisplayModeGetRefreshRate(cur);
	size_t w = CGDisplayModeGetWidth(cur);
	size_t h = CGDisplayModeGetHeight(cur);

	// Zaten <=65Hz ise dokunma (kullanıcı 60Hz seçmişse no-op).
	if (curHz > 0.0 && curHz <= 65.0)
	{
		CGDisplayModeRelease(cur);
		return 0;
	}

	// Aynı çözünürlükte 60Hz modu bul.
	NSDictionary* opts = @{ (id) kCGDisplayShowDuplicateLowResolutionModes : @YES };
	CFArrayRef    arr  = CGDisplayCopyAllDisplayModes(did, (CFDictionaryRef) opts);
	if (arr == nullptr)
	{
		CGDisplayModeRelease(cur);
		return 0;
	}

	CGDisplayModeRef pick = nullptr;
	CFIndex          n    = CFArrayGetCount(arr);
	for (CFIndex i = 0; i < n; i++)
	{
		CGDisplayModeRef m = (CGDisplayModeRef) CFArrayGetValueAtIndex(arr, i);
		if (CGDisplayModeGetWidth(m) == w && CGDisplayModeGetHeight(m) == h)
		{
			double hz = CGDisplayModeGetRefreshRate(m);
			if (hz >= 59.0 && hz <= 61.0)
			{
				pick = m;
				break;
			}
		}
	}

	if (pick != nullptr)
	{
		g_forcedDisplay = did;
		g_savedMode     = cur; // sahiplen (Restore'da release)
		g_didForce      = true;
		CGDisplaySetDisplayMode(did, pick, nullptr);
		std::atexit(RestoreRefreshRate);
		std::signal(SIGINT, SignalRestoreHandler);
		std::signal(SIGTERM, SignalRestoreHandler);
		fprintf(stderr, "[mac] ekran %.0fHz → 60Hz zorlandı (ProMotion/120Hz oyunu kilitliyor)\n", curHz);
		CFRelease(arr);
		return 1;
	}

	CGDisplayModeRelease(cur);
	CFRelease(arr);
	return 0;
}

// İmza basit (void*, int) — mac_input.h/windows shim'i ObjC BOOL ile çakıştığından include
// ETMİYORUZ; sadece tanım. Çağıran mac_window.cpp C++ linkage ile extern bildirir.
int KO_Mac_NSWindowVisible(void* sdlWindow)
{
	if (sdlWindow == nullptr)
		return 1;

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (!SDL_GetWindowWMInfo(reinterpret_cast<SDL_Window*>(sdlWindow), &info))
		return 1;
	if (info.subsystem != SDL_SYSWM_COCOA)
		return 1;

	NSWindow* win = info.info.cocoa.window;
	if (win == nil)
		return 1;

	return ([win occlusionState] & NSWindowOcclusionStateVisible) != 0 ? 1 : 0;
}

#endif // __APPLE__

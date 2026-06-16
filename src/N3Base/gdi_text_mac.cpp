// gdi_text_mac.cpp — macOS port: DFont glyph rasterizer'ının kullandığı GDI metin
// API alt kümesinin CoreText/CoreGraphics ile GERÇEK implementasyonu.
//
// DFont.cpp (N3Base) metni şöyle üretir: bir DIB (CreateDIBSection) oluşturur,
// içine glyph'leri ExtTextOut ile çizer, sonra DIB'in yoğunluğunu A4R4G4B4
// texture'a alpha olarak kopyalar. Windows'ta bu GDI'dir; macOS'ta GDI yok →
// stub'lar boş bitmap döndürüyordu → yazı görünmüyordu. Burada aynı GDI yüzeyini
// CoreText (ölçüm + glyph) ve CoreGraphics (CGBitmapContext'e çizim) ile karşılıyoruz.
//
// Tasarım: HDC/HFONT/HBITMAP opak handle'lar küçük struct'lara map'lenir. DC,
// seçili font ve bitmap'i tutar (GDI SelectObject semantiği). Metin beyaz çizilir;
// DFont yoğunluğu (düşük bayt) alpha'ya çevirir, gerçek rengi vertex diffuse verir.
//
// See docs/MAC_PORT_ROADMAP.md.

#if defined(__APPLE__)

#include "StdAfxBase.h" // DFont ile AYNI tip ortamı (SIZE/RECT/BITMAPINFO + GDI bildirimleri)

// dxvk-native windows_base.h `#define interface struct` yapıyor; Apple SDK
// header'larındaki (CoreGraphics→IOKit) `interface` adlı alanlarla çakışıyor.
// CoreText/CoreGraphics'i dahil etmeden önce bu makroyu kaldır.
#ifdef interface
#undef interface
#endif

#include <CoreText/CoreText.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>

#include <cmath>
#include <cstdlib>

namespace
{
enum GdiKind
{
	GK_FONT   = 1,
	GK_BITMAP = 2,
};

struct GdiObj
{
	GdiKind kind;
};

struct GdiFont : GdiObj
{
	CTFontRef font;       // sahiplenilen referans
	double    ascent;     // baseline'ın hücre üstünden uzaklığı (px)
	double    cellHeight; // hücre yüksekliği = ceil(ascent+descent) (px)
};

struct GdiBitmap : GdiObj
{
	void*         bits;   // CreateDIBSection tamponu (DFont sahiplenir/okur)
	int           width;
	int           height;
	CGContextRef  ctx;    // bits üstüne tembel açılan CGBitmapContext
};

struct GdiDC
{
	GdiFont*   font; // seçili font (SelectObject)
	GdiBitmap* bmp;  // seçili bitmap (SelectObject)
};

// CP949(DBCS Korece) → değilse Latin1 → MacRoman ile çöz. DFont tek tek 1/2 baytlık
// karakterleri ya da tüm satırı verir; CP949 ASCII'yi de doğru kapsar.
CFStringRef MakeCFString(const char* b, int len)
{
	if (b == nullptr || len <= 0)
		return CFStringCreateWithCString(nullptr, "", kCFStringEncodingASCII);

	CFStringRef s = CFStringCreateWithBytes(
		nullptr, (const UInt8*) b, len, kCFStringEncodingDOSKorean, false);
	if (s == nullptr)
		s = CFStringCreateWithBytes(
			nullptr, (const UInt8*) b, len, kCFStringEncodingWindowsLatin1, false);
	if (s == nullptr)
		s = CFStringCreateWithBytes(
			nullptr, (const UInt8*) b, len, kCFStringEncodingMacRoman, false);
	if (s == nullptr)
		s = CFStringCreateWithCString(nullptr, "?", kCFStringEncodingASCII);
	return s;
}

CGColorRef WhiteColor()
{
	static CGColorRef s_white = nullptr;
	if (s_white == nullptr)
	{
		CGColorSpaceRef cs   = CGColorSpaceCreateDeviceRGB();
		CGFloat         c[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		s_white              = CGColorCreate(cs, c);
		CGColorSpaceRelease(cs);
	}
	return s_white;
}

// (str,len) için CoreText satırı kur (font + beyaz renk). Çağıran CFRelease eder.
CTLineRef MakeLine(GdiFont* gf, const char* str, int len)
{
	if (gf == nullptr || gf->font == nullptr)
		return nullptr;

	CFStringRef s = MakeCFString(str, len);

	const void* keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
	const void* vals[] = { gf->font, WhiteColor() };
	CFDictionaryRef attrs = CFDictionaryCreate(nullptr, keys, vals, 2,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	CFAttributedStringRef as = CFAttributedStringCreate(nullptr, s, attrs);
	CTLineRef line = CTLineCreateWithAttributedString(as);

	CFRelease(as);
	CFRelease(attrs);
	CFRelease(s);
	return line;
}
} // namespace

// ---------------------------------------------------------------------------
// GDI yüzeyi (win32_api_compat.h'da __APPLE__ altında extern bildirildi)
// ---------------------------------------------------------------------------

HDC CreateCompatibleDC(HDC)
{
	GdiDC* dc = new GdiDC();
	dc->font  = nullptr;
	dc->bmp   = nullptr;
	return reinterpret_cast<HDC>(dc);
}

BOOL DeleteDC(HDC hdc)
{
	delete reinterpret_cast<GdiDC*>(hdc);
	return TRUE;
}

HFONT CreateFontA(int nHeight, int /*nWidth*/, int /*nEsc*/, int /*nOrient*/,
	int fnWeight, DWORD fdwItalic, DWORD /*underline*/, DWORD /*strike*/,
	DWORD /*charset*/, DWORD /*outPrec*/, DWORD /*clipPrec*/, DWORD /*quality*/,
	DWORD /*pitch*/, const char* lpszFace)
{
	// nHeight DFont'ta negatif (karakter hücre yüksekliği). Piksel boyu = |nHeight|.
	double size = std::fabs((double) nHeight);
	if (size < 1.0)
		size = 12.0;

	CFStringRef name = MakeCFString(lpszFace, lpszFace != nullptr ? (int) std::strlen(lpszFace) : 0);
	CTFontRef   font = CTFontCreateWithName(name, size, nullptr);
	CFRelease(name);
	if (font == nullptr)
		font = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, nullptr);

	// Bold/italic sembolik trait'leri uygula.
	CTFontSymbolicTraits want = 0;
	if (fnWeight >= FW_BOLD)
		want |= kCTFontTraitBold;
	if (fdwItalic != 0)
		want |= kCTFontTraitItalic;
	if (want != 0 && font != nullptr)
	{
		CTFontRef f2 = CTFontCreateCopyWithSymbolicTraits(font, size, nullptr, want, want);
		if (f2 != nullptr)
		{
			CFRelease(font);
			font = f2;
		}
	}

	GdiFont* gf   = new GdiFont();
	gf->kind      = GK_FONT;
	gf->font      = font;
	gf->ascent    = (font != nullptr) ? CTFontGetAscent(font) : size;
	double desc   = (font != nullptr) ? CTFontGetDescent(font) : 0.0;
	gf->cellHeight = std::ceil(gf->ascent + desc);
	if (gf->cellHeight < 1.0)
		gf->cellHeight = size;
	return reinterpret_cast<HFONT>(gf);
}

HGDIOBJ SelectObject(HDC hdc, HGDIOBJ obj)
{
	GdiDC* dc = reinterpret_cast<GdiDC*>(hdc);
	if (dc == nullptr || obj == nullptr)
		return nullptr; // GDI: NULL seçme yok; önceki "default" ~ nullptr

	GdiObj* o = reinterpret_cast<GdiObj*>(obj);
	if (o->kind == GK_FONT)
	{
		GdiObj* prev = dc->font;
		dc->font     = static_cast<GdiFont*>(o);
		return reinterpret_cast<HGDIOBJ>(prev);
	}
	if (o->kind == GK_BITMAP)
	{
		GdiObj* prev = dc->bmp;
		dc->bmp      = static_cast<GdiBitmap*>(o);
		return reinterpret_cast<HGDIOBJ>(prev);
	}
	return nullptr;
}

BOOL DeleteObject(HGDIOBJ obj)
{
	if (obj == nullptr)
		return TRUE;
	GdiObj* o = reinterpret_cast<GdiObj*>(obj);
	if (o->kind == GK_FONT)
	{
		GdiFont* gf = static_cast<GdiFont*>(o);
		if (gf->font != nullptr)
			CFRelease(gf->font);
		delete gf;
	}
	else if (o->kind == GK_BITMAP)
	{
		GdiBitmap* gb = static_cast<GdiBitmap*>(o);
		if (gb->ctx != nullptr)
			CGContextRelease(gb->ctx);
		std::free(gb->bits);
		delete gb;
	}
	return TRUE;
}

BOOL GetTextExtentPoint32A(HDC hdc, const char* str, int len, SIZE* sz)
{
	if (sz == nullptr)
		return FALSE;
	sz->cx = 0;
	sz->cy = 0;

	GdiDC* dc = reinterpret_cast<GdiDC*>(hdc);
	if (dc == nullptr || dc->font == nullptr)
		return FALSE;

	CTLineRef line = MakeLine(dc->font, str, len);
	if (line == nullptr)
		return FALSE;

	double w = CTLineGetTypographicBounds(line, nullptr, nullptr, nullptr);
	CFRelease(line);

	sz->cx = (LONG) std::ceil(w);
	sz->cy = (LONG) dc->font->cellHeight;
	return TRUE;
}

BOOL ExtTextOutA(HDC hdc, int x, int y, UINT /*opts*/, const RECT* /*clip*/,
	const char* str, UINT len, const int* /*dx*/)
{
	GdiDC* dc = reinterpret_cast<GdiDC*>(hdc);
	if (dc == nullptr || dc->font == nullptr || dc->bmp == nullptr)
		return FALSE;

	GdiBitmap* gb = dc->bmp;
	if (gb->bits == nullptr)
		return FALSE;

	// CGBitmapContext'i tembel aç. Tampon BGRA32 (DIB ile aynı bellek düzeni),
	// üstten-aşağı: bellek satır 0 = görüntü üst satırı.
	if (gb->ctx == nullptr)
	{
		CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
		gb->ctx = CGBitmapContextCreate(gb->bits, gb->width, gb->height, 8,
			gb->width * 4, cs,
			(CGBitmapInfo) (kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little));
		CGColorSpaceRelease(cs);
		if (gb->ctx == nullptr)
			return FALSE;
		CGContextSetShouldAntialias(gb->ctx, true);
		CGContextSetShouldSmoothFonts(gb->ctx, true);
	}

	CTLineRef line = MakeLine(dc->font, str, (int) len);
	if (line == nullptr)
		return FALSE;

	// GDI TA_TOP: (x,y) hücrenin sol-üstü. CG kökeni sol-alt → baseline'ı
	// CG-y = height - y - ascent konumuna yerleştir.
	double baseY = (double) gb->height - (double) y - dc->font->ascent;
	CGContextSetTextPosition(gb->ctx, (CGFloat) x, (CGFloat) baseY);
	CTLineDraw(line, gb->ctx);
	CFRelease(line);
	return TRUE;
}

HBITMAP CreateDIBSection(HDC, const BITMAPINFO* bmi, UINT, void** ppvBits, HANDLE, DWORD)
{
	if (ppvBits != nullptr)
		*ppvBits = nullptr;
	if (bmi == nullptr)
		return nullptr;

	int w = (int) bmi->bmiHeader.biWidth;
	int h = (int) bmi->bmiHeader.biHeight;
	if (h < 0)
		h = -h; // DFont top-down (negatif biHeight) verir
	if (w <= 0 || h <= 0)
		return nullptr;

	void* bits = std::calloc((size_t) w * (size_t) h, 4); // sıfır = siyah, alpha 0
	if (bits == nullptr)
		return nullptr;

	GdiBitmap* gb = new GdiBitmap();
	gb->kind   = GK_BITMAP;
	gb->bits   = bits;
	gb->width  = w;
	gb->height = h;
	gb->ctx    = nullptr;

	if (ppvBits != nullptr)
		*ppvBits = bits;
	return reinterpret_cast<HBITMAP>(gb);
}

#endif // __APPLE__

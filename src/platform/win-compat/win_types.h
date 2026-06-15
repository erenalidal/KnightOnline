// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// win_types.h — Win32 tip/makro bosluklari icin minimal shim.
//
// Bu bashlik YALNIZCA dxvk-native'in Windows base shim'inin saglamadigi (veya
// garanti etmedigi) Win32 sembolleri doldurur. dxvk-native cogu temel tipi
// (RECT, POINT, BOOL, DWORD, HWND ...) zaten saglar; bu yuzden burada her sembol
// #ifndef ile korunur ve ikili tanim / cakisma olmasi onlenir.
//
// Tum icerik `#if !defined(_WIN32)` altindadir: gercek Windows derlemesinde bu
// dosya hicbir sey yapmaz.

#ifndef KO_PLATFORM_WIN_TYPES_H_
#define KO_PLATFORM_WIN_TYPES_H_

#pragma once

#if !defined(_WIN32)

// dxvk-native'in Windows base shim'i — RECT/POINT/SIZE/LARGE_INTEGER/BOOL/DWORD
// gibi temel tipleri sağlar. Bizim şimlerimiz bu tiplere dayandığı için, include
// sırasından bağımsız çalışmak adına burada GARANTİ ediyoruz.
#include <windows.h>

#include <cstdint>

//
// --- Temel tamsayi / tip takma adlari -------------------------------------
//
// NOT: RECT ve POINT dxvk-native tarafindan saglanir; bu yuzden burada VARSAYIM
//      YAPMADAN sadece #ifndef ile koruyoruz (dxvk tanimlamadiysa fallback).
//

#ifndef _BOOL_DEFINED
#ifndef BOOL
typedef int BOOL;
#endif
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef _BYTE_DEFINED
#ifndef BYTE
typedef uint8_t BYTE;
#endif
#endif

// Windows rpcndr.h `byte` tipi (UITransactionDlg vb. kullanır).
#ifndef KO_BYTE_LOWER_DEFINED
#define KO_BYTE_LOWER_DEFINED
typedef unsigned char byte;
#endif

#ifndef _WORD_DEFINED
#ifndef WORD
typedef uint16_t WORD;
#endif
#endif

#ifndef _DWORD_DEFINED
#ifndef DWORD
typedef uint32_t DWORD;
#endif
#endif

// HRESULT — dxvk genelde saglar; emniyet icin korumali fallback.
#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
#ifndef HRESULT
typedef int32_t HRESULT;
#endif
#endif

//
// --- Opak handle tipleri ---------------------------------------------------
//
// Bu handle'lar macOS'ta gercek bir anlam tasimaz; sadece imzalarin derlenmesi
// icin opak isaretci olarak tanimlanir. dxvk shim'i zaten tanimladiysa atlanir.
//

#ifndef _WINDEF_HANDLES_DEFINED
#ifndef HWND
typedef void* HWND;
#endif
#ifndef HDC
typedef void* HDC;
#endif
#ifndef HINSTANCE
typedef void* HINSTANCE;
#endif
#endif

//
// --- RECT / POINT ----------------------------------------------------------
//
// RECT, POINT (ve LARGE_INTEGER, asagida) dxvk-native'in windows base shim'i
// tarafindan saglanir. Burada YENIDEN TANIMLAMIYORUZ: dxvk struct adlandirmasi
// (tag'siz) bizimkiyle (tagRECT) farkli oldugundan ikili tanim cakismasi olur.
//

//
// --- Cesitli makrolar ------------------------------------------------------
//

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

// SAL annotation makroları (WinMain vb. imzalarda _In_/_In_opt_/...) — no-op.
#ifndef _In_
#define _In_
#define _In_opt_
#define _Out_
#define _Out_opt_
#define _Inout_
#define _Inout_opt_
#define _In_z_
#define _Printf_format_string_
#endif

// String pointer takma adları.
#ifndef LPSTR
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef wchar_t* LPWSTR;
typedef const wchar_t* LPCWSTR;
typedef void* LPVOID;
typedef const void* LPCVOID;
#endif

// WINAPI / cagri kurali makrolari macOS'ta anlamsiz; bos tanimlanir.
#ifndef WINAPI
#define WINAPI
#endif

#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

// ZeroMemory — Win32 makrosunun memset karsiligi.
#ifndef ZeroMemory
#include <cstring>
#define ZeroMemory(dest, len) ::memset((dest), 0, (len))
#endif

//
// --- Yuksek cozunurluklu zamanlama (QPC) -----------------------------------
//
// N3Base, frame zamanlamasi icin QueryPerformanceFrequency/Counter ve
// LARGE_INTEGER kullanir. dxvk bunlari garanti etmedigi icin macOS'a ozgu
// mach_absolute_time() tabanli inline bir implementasyon sagliyoruz.
//

// LARGE_INTEGER de dxvk-native tarafindan saglanir; burada tanimlamiyoruz.
// Asagidaki QPC fonksiyonlari dxvk'nin LARGE_INTEGER'ini kullanir.

#ifndef _KO_QPC_DEFINED
#define _KO_QPC_DEFINED

#include <mach/mach_time.h>

// QueryPerformanceFrequency: sayacin saniyedeki tik sayisi.
// mach_absolute_time() nanosaniye-tabanli olceklenin ce, frekansi 1e9 (1 GHz)
// olarak raporluyoruz; QPC zaten "tutarli birim" oldugu surece dogru calisir.
inline BOOL QueryPerformanceFrequency(LARGE_INTEGER* freq)
{
	if (freq == nullptr)
		return FALSE;

	freq->QuadPart = 1000000000LL; // tik/saniye = nanosaniye
	return TRUE;
}

// QueryPerformanceCounter: mach_absolute_time degerini timebase ile nanosaniyeye
// cevirip dondurur.
inline BOOL QueryPerformanceCounter(LARGE_INTEGER* counter)
{
	if (counter == nullptr)
		return FALSE;

	static mach_timebase_info_data_t s_timebase = {0, 0};
	if (s_timebase.denom == 0)
		mach_timebase_info(&s_timebase);

	uint64_t ticks    = mach_absolute_time();
	// ticks * (numer/denom) => nanosaniye
	uint64_t nanos    = (ticks * s_timebase.numer) / s_timebase.denom;
	counter->QuadPart = static_cast<int64_t>(nanos);
	return TRUE;
}

#endif // _KO_QPC_DEFINED

// Genişletilmiş Win32 API yüzeyi (GDI bitmap, global bellek, user32 stub'ları).
#include "win32_api_compat.h"
// Pencere/mesaj (user32) + IME stub'ları.
#include "win_user32.h"

#endif // !defined(_WIN32)

#endif // KO_PLATFORM_WIN_TYPES_H_

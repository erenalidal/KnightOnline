// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// dinput.h — DirectInput 8'in minimal şimi (klavye).
//
// WarFare LocalInput, klavyeyi DirectInput8 ile okur. macOS'ta gerçek giriş
// Faz 4'te SDL ile gelecek; burada interface'ler no-op stub'tır (GetDeviceState
// tüm tuşları "basılı değil" döndürür) ve DIK_* tarama kodları standart
// DirectInput değerleriyle tanımlıdır (ileride SDL scancode eşlemesi için).

#ifndef KO_PLATFORM_DINPUT_H_
#define KO_PLATFORM_DINPUT_H_

#pragma once

#if !defined(_WIN32)

#include <cstring>
#include "win_types.h" // HWND/HINSTANCE/DWORD/HRESULT + dxvk GUID
#include "mac_input.h" // SDL klavye köprüsü

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#ifndef DI_OK
#define DI_OK 0
#endif

// Cooperative level bayrakları.
#define DISCL_EXCLUSIVE    0x0001
#define DISCL_NONEXCLUSIVE 0x0002
#define DISCL_FOREGROUND   0x0004
#define DISCL_BACKGROUND   0x0008
#define DISCL_NOWINKEY     0x0010

// --- Minimal DirectInput interface stub'ları ---
struct KO_DIDevice8
{
	HRESULT SetDataFormat(const void*) { return DI_OK; }
	HRESULT SetCooperativeLevel(HWND, DWORD) { return DI_OK; }
	HRESULT SetProperty(const GUID&, const void*) { return DI_OK; }
	HRESULT Acquire() { return DI_OK; }
	HRESULT Unacquire() { return DI_OK; }
	HRESULT GetDeviceState(DWORD cbData, void* lpvData) { KO_Mac_FillDIKeyboard((unsigned char*) lpvData, (int) cbData); return DI_OK; }
	HRESULT GetDeviceData(DWORD, void*, DWORD*, DWORD) { return DI_OK; }
	unsigned long Release() { return 0; }
};
typedef KO_DIDevice8* LPDIRECTINPUTDEVICE8;

struct KO_DI8
{
	HRESULT CreateDevice(const GUID&, LPDIRECTINPUTDEVICE8* dev, void*)
	{ static KO_DIDevice8 s_dev; if (dev) *dev = &s_dev; return DI_OK; }
	unsigned long Release() { return 0; }
};
typedef KO_DI8* LPDIRECTINPUT8;

inline HRESULT DirectInput8Create(HINSTANCE, DWORD, const GUID&, void** ppvOut, void*)
{ static KO_DI8 s_di; if (ppvOut) *ppvOut = &s_di; return DI_OK; }

// GUID / dataformat sabitleri (değerleri stub bağlamında önemsiz).
static const GUID IID_IDirectInput8 = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const GUID GUID_SysKeyboard  = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const GUID GUID_SysMouse     = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
static const int  c_dfDIKeyboard    = 0; // SetDataFormat(&c_dfDIKeyboard) için adres
static const int  c_dfDIMouse2      = 0;

// --- DIK_* klavye tarama kodları (standart DirectInput değerleri) ---
#define DIK_ESCAPE      0x01
#define DIK_1           0x02
#define DIK_2           0x03
#define DIK_3           0x04
#define DIK_4           0x05
#define DIK_5           0x06
#define DIK_6           0x07
#define DIK_7           0x08
#define DIK_8           0x09
#define DIK_9           0x0A
#define DIK_0           0x0B
#define DIK_TAB         0x0F
#define DIK_Q           0x10
#define DIK_W           0x11
#define DIK_E           0x12
#define DIK_R           0x13
#define DIK_T           0x14
#define DIK_U           0x16
#define DIK_I           0x17
#define DIK_RETURN      0x1C
#define DIK_LCONTROL    0x1D
#define DIK_A           0x1E
#define DIK_S           0x1F
#define DIK_D           0x20
#define DIK_F           0x21
#define DIK_H           0x23
#define DIK_K           0x25
#define DIK_Z           0x2C
#define DIK_X           0x2D
#define DIK_C           0x2E
#define DIK_V           0x2F
#define DIK_B           0x30
#define DIK_M           0x32
#define DIK_F1          0x3B
#define DIK_F2          0x3C
#define DIK_F3          0x3D
#define DIK_F4          0x3E
#define DIK_F5          0x3F
#define DIK_F6          0x40
#define DIK_F7          0x41
#define DIK_F8          0x42
#define DIK_F9          0x43
#define DIK_F10         0x44
#define DIK_NUMPADMINUS 0x4A
#define DIK_F11         0x57
#define DIK_F12         0x58
#define DIK_NUMPADENTER 0x9C
#define DIK_RCONTROL    0x9D
#define DIK_HOME        0xC7
#define DIK_UP          0xC8
#define DIK_PRIOR       0xC9
#define DIK_LEFT        0xCB
#define DIK_RIGHT       0xCD
#define DIK_END         0xCF
#define DIK_DOWN        0xD0
#define DIK_NEXT        0xD1

#endif // !defined(_WIN32)

#endif // KO_PLATFORM_DINPUT_H_

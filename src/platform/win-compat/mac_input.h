// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// mac_input.h — SDL tabanlı girdi köprüsü bildirimleri.
//
// Win32/DirectInput şimleri (win32_api_compat.h, win_user32.h, dinput.h) bu
// fonksiyonları çağırır; implementasyon SDL'i içeren mac-entry/mac_window.cpp'de.
#ifndef KO_PLATFORM_MAC_INPUT_H_
#define KO_PLATFORM_MAC_INPUT_H_
#pragma once

#if !defined(_WIN32)

#include "win_types.h"

// VK_* sanal tuş için durum: basılıysa 0x8000 döner (fare butonları + Ctrl/Alt/Shift
// ve birkaç yaygın tuş). GetAsyncKeyState/GetKeyState/_IsKeyDown bunu kullanır.
int  KO_Mac_GetAsyncKeyState(int virtualKey);

// Fare imlecini pencere-istemci koordinatlarında döndürür (ScreenToClient kimliktir).
void KO_Mac_GetCursorPos(POINT* p);

// Fare imlecini pencere-istemci koordinatlarına taşır (SDL_WarpMouseInWindow). Kamera
// mouselook (sağ-tık sürükleme) imleci her frame başlangıç noktasına geri sabitler;
// SetCursorPos no-op iken imleç ekran kenarına kayıp kamera dönüşü kesiliyordu.
void KO_Mac_SetCursorPos(int x, int y);

// SDL penceresinin istemci (drawable olmayan, nokta cinsinden) boyutu. GetClientRect
// bunu kullanır; CLocalInput::Tick fare-buton flag'lerini yalnızca imleç bu rect'in
// içindeyse set ettiğinden, doğru boyut tıklamanın çalışması için ŞART.
void KO_Mac_GetClientSize(int* w, int* h);

// DirectInput klavye durumu: buf[DIK_*] = 0x80 basılıysa. count = NUMDIKEYS.
void KO_Mac_FillDIKeyboard(unsigned char* buf, int count);

// Odaktaysa SDL penceresinin HWND'sini, değilse nullptr döndürür
// (CLocalInput::Tick odak kontrolü için).
HWND KO_Mac_GetActiveWindow();

// --- Metin girişi köprüsü (CN3UIEdit gizli "EDIT" kontrolü) ---
// CN3UIEdit, CreateWindow("EDIT"...) ile gizli bir kontrol açıp metni orada tutar.
// macOS'ta bu kontrol, SDL metin girişiyle beslenen bir string tamponuna eşlenir.
HWND KO_Mac_EditHandle();                  // singleton EDIT sentinel handle
void KO_Mac_EditSetText(const char* s);    // tamponu ayarla (SetWindowText)
int  KO_Mac_EditGetText(char* buf, int n); // tamponu kopyala, uzunluk döndür
int  KO_Mac_EditGetTextLength();
void KO_Mac_EditSetActive(int active);     // SDL text input başlat/durdur (odak)
int  KO_Mac_EditGetCaret();                // caret konumu (karakter indeksi)
int  KO_Mac_EditConsumeReturn();           // Return basıldıysa 1 (tüketir)
int  KO_Mac_EditConsumeTab();              // Tab basıldıysa 1 (tüketir)

#endif // !defined(_WIN32)
#endif // KO_PLATFORM_MAC_INPUT_H_

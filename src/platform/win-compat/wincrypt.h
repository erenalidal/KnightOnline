// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// wincrypt.h — Windows CryptoAPI'nin minimal şimi.
//
// CWinCrypt, şifreli oyun dosyalarını okumak için sabit bir cipher string'in
// SHA-1 hash'inden RC4 anahtarı türetip CryptDecrypt ile çözer.
//
// DURUM: Şu an STUB. Fonksiyonlar derlemeyi geçirir ama gerçek şifre çözme
// YAPMAZ. Gerçek implementasyon (macOS CommonCrypto: CC_SHA1 + RC4, Windows
// CryptDeriveKey KDF'ini birebir taklit ederek) Faz 6'da yapılacak.
// TODO(mac-port): CommonCrypto ile gerçek RC4/SHA-1 implementasyonu.

#ifndef KO_PLATFORM_WINCRYPT_H_
#define KO_PLATFORM_WINCRYPT_H_

#pragma once

#if !defined(_WIN32)

#include "win_types.h"

#ifndef TCHAR
typedef char TCHAR;
#endif

typedef uintptr_t HCRYPTPROV;
typedef uintptr_t HCRYPTKEY;
typedef uintptr_t HCRYPTHASH;
typedef unsigned int ALG_ID;

// Provider adı + algoritma/bayrak sabitleri (Windows wincrypt.h ile aynı isim).
#define MS_ENHANCED_PROV    "Microsoft Enhanced Cryptographic Provider v1.0"
#define PROV_RSA_FULL       1
#define CRYPT_VERIFYCONTEXT 0xF0000000
#define CRYPT_NEWKEYSET     0x00000008
#define CALG_SHA            0x00008004
#define CALG_SHA1           0x00008004
#define CALG_RC4            0x00006801

// --- Gerçek implementasyon: wincrypt_compat.cpp (SHA-1 + RC4, CommonCrypto) ---
BOOL CryptAcquireContextA(HCRYPTPROV* p, const char*, const char*, DWORD, DWORD);
#ifndef CryptAcquireContext
#define CryptAcquireContext CryptAcquireContextA
#endif
BOOL CryptCreateHash(HCRYPTPROV, ALG_ID, HCRYPTKEY, DWORD, HCRYPTHASH* h);
BOOL CryptHashData(HCRYPTHASH, const BYTE*, DWORD, DWORD);
BOOL CryptDeriveKey(HCRYPTPROV, ALG_ID, HCRYPTHASH, DWORD, HCRYPTKEY* k);
BOOL CryptDecrypt(HCRYPTKEY, HCRYPTHASH, BOOL, DWORD, BYTE*, DWORD*);
BOOL CryptDestroyHash(HCRYPTHASH);
BOOL CryptDestroyKey(HCRYPTKEY);
BOOL CryptReleaseContext(HCRYPTPROV, DWORD);

#endif // !defined(_WIN32)

#endif // KO_PLATFORM_WINCRYPT_H_

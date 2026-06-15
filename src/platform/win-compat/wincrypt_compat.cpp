// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// wincrypt_compat.cpp — Windows CryptoAPI'nin GERÇEK macOS implementasyonu.
//
// KO .tbl/asset dosyaları RC4 ile şifrelidir. Anahtar, sabit bir cipher
// string'in SHA-1 hash'inden Windows CryptDeriveKey KDF'i ile türetilir:
//   base = SHA1(data)
//   key  = ( SHA1(0x36⊕base ‖ pad) ‖ SHA1(0x5C⊕base ‖ pad) )[0..15]   (RC4-128)
// Sonra veri RC4 ile çözülür. Bu, Windows CALG_SHA + CALG_RC4 davranışını birebir
// taklit eder. SHA-1 için macOS CommonCrypto, RC4 için elle implementasyon.

#if !defined(_WIN32)

#include "wincrypt.h"

#include <cstring>
#include <cstdint>
#include <vector>

#define COMMON_DIGEST_FOR_OPENSSL
#include <CommonCrypto/CommonDigest.h>

namespace {

// --- Hash durumu (CryptCreateHash/HashData) ---
struct KO_HashState
{
	std::vector<uint8_t> data; // CryptHashData ile beslenen ham veri
};

// --- RC4 anahtar durumu (CryptDeriveKey/Decrypt) ---
struct KO_KeyState
{
	uint8_t s[256];
	int     i;
	int     j;
};

void rc4_init(KO_KeyState* st, const uint8_t* key, int keyLen)
{
	for (int n = 0; n < 256; ++n)
		st->s[n] = static_cast<uint8_t>(n);
	int j = 0;
	for (int n = 0; n < 256; ++n)
	{
		j = (j + st->s[n] + key[n % keyLen]) & 0xFF;
		uint8_t t = st->s[n]; st->s[n] = st->s[j]; st->s[j] = t;
	}
	st->i = 0;
	st->j = 0;
}

void rc4_crypt(KO_KeyState* st, uint8_t* buf, size_t len)
{
	int i = st->i, j = st->j;
	for (size_t n = 0; n < len; ++n)
	{
		i = (i + 1) & 0xFF;
		j = (j + st->s[i]) & 0xFF;
		uint8_t t = st->s[i]; st->s[i] = st->s[j]; st->s[j] = t;
		buf[n] ^= st->s[(st->s[i] + st->s[j]) & 0xFF];
	}
	st->i = i;
	st->j = j;
}

} // namespace

BOOL CryptAcquireContextA(HCRYPTPROV* p, const char*, const char*, DWORD, DWORD)
{
	if (p) *p = 1; // provider durumu gerekmez
	return TRUE;
}

BOOL CryptCreateHash(HCRYPTPROV, ALG_ID, HCRYPTKEY, DWORD, HCRYPTHASH* h)
{
	if (!h) return FALSE;
	*h = reinterpret_cast<HCRYPTHASH>(new KO_HashState());
	return TRUE;
}

BOOL CryptHashData(HCRYPTHASH h, const BYTE* data, DWORD len, DWORD)
{
	auto* hs = reinterpret_cast<KO_HashState*>(h);
	if (!hs || !data) return FALSE;
	hs->data.insert(hs->data.end(), data, data + len);
	return TRUE;
}

BOOL CryptDeriveKey(HCRYPTPROV, ALG_ID, HCRYPTHASH h, DWORD, HCRYPTKEY* k)
{
	auto* hs = reinterpret_cast<KO_HashState*>(h);
	if (!hs || !k) return FALSE;

	// base = SHA1(beslenen veri)
	uint8_t base[CC_SHA1_DIGEST_LENGTH];
	CC_SHA1(hs->data.data(), static_cast<CC_LONG>(hs->data.size()), base);

	// Windows CryptDeriveKey KDF (hash-tabanlı, anahtar ≤ hash boyutu):
	uint8_t buf1[64], buf2[64];
	std::memset(buf1, 0x36, sizeof(buf1));
	std::memset(buf2, 0x5C, sizeof(buf2));
	for (int n = 0; n < CC_SHA1_DIGEST_LENGTH; ++n)
	{
		buf1[n] ^= base[n];
		buf2[n] ^= base[n];
	}
	uint8_t keyMaterial[2 * CC_SHA1_DIGEST_LENGTH];
	CC_SHA1(buf1, sizeof(buf1), keyMaterial);
	CC_SHA1(buf2, sizeof(buf2), keyMaterial + CC_SHA1_DIGEST_LENGTH);

	// RC4-128: anahtar materyalinin ilk 16 baytı.
	auto* ks = new KO_KeyState();
	rc4_init(ks, keyMaterial, 16);
	*k = reinterpret_cast<HCRYPTKEY>(ks);
	return TRUE;
}

BOOL CryptDecrypt(HCRYPTKEY k, HCRYPTHASH, BOOL, DWORD, BYTE* data, DWORD* len)
{
	auto* ks = reinterpret_cast<KO_KeyState*>(k);
	if (!ks || !data || !len) return FALSE;
	rc4_crypt(ks, data, *len);
	return TRUE;
}

BOOL CryptDestroyHash(HCRYPTHASH h)
{
	delete reinterpret_cast<KO_HashState*>(h);
	return TRUE;
}

BOOL CryptDestroyKey(HCRYPTKEY k)
{
	delete reinterpret_cast<KO_KeyState*>(k);
	return TRUE;
}

BOOL CryptReleaseContext(HCRYPTPROV, DWORD)
{
	return TRUE;
}

#endif // !defined(_WIN32)

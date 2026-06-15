// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// JpegFile_mac.cpp — CJpegFile'ın macOS/non-Windows stub implementasyonu.
//
// Orijinal JpegFile.cpp, Windows GDI (ekran yakalama, DIB, palette) ile libjpeg'i
// karıştırır ve 64-bit pointer→DWORD truncation gibi Windows'a özgü kabuller içerir.
// WarFare bu sınıfı YALNIZCA ekran görüntüsü almak için kullanır
// (CaptureScreenAndSaveToFile → CopyScreenToDIB + EncryptJPEG) — oynamak için kritik
// değildir. Bu stub, derleme/link'i geçirir; ekran görüntüsü özelliği şimdilik no-op.
//
// TODO(mac-port): Faz 6'da Metal framebuffer okuma + libjpeg encode ile gerçek
// ekran görüntüsü desteği.

// NOT: Stub libjpeg KULLANMAZ, bu yüzden tam Win32 şimlerini (dxvk RECT dahil)
// kullanırız — böylece RECT/HANDLE tipleri WarFare TU'larıyla AYNI olur ve
// link sembolleri eşleşir.
#include <win_types.h>
#include "JpegFile.h"

#include <string>

// Encrypt/Decrypt için statik üyeler (orijinal JpegFile.cpp ile aynı değerler).
WORD CJpegFile::m_r  = 1124;
WORD CJpegFile::m_c1 = 52845;
WORD CJpegFile::m_c2 = 22719;

CJpegFile::CJpegFile() = default;
CJpegFile::~CJpegFile() = default;

// Ekran yakalama — macOS'ta henüz desteklenmiyor; nullptr döndürür.
HANDLE CJpegFile::CopyScreenToDIB(LPRECT /*lpRect*/)
{
	return nullptr;
}

// Şifreli JPEG kaydetme — ekran görüntüsü yolu stub olduğu için no-op.
BOOL CJpegFile::EncryptJPEG(HANDLE /*hDib*/, int /*nQuality*/,
	const std::string& /*csJpeg*/, const char** pcsMsg)
{
	if (pcsMsg != nullptr)
		*pcsMsg = "Screenshot not supported on this platform yet";
	return 0; // FALSE
}

// Şifreli JPEG çözme — ekran görüntüsü/önizleme yolu; macOS'ta stub.
BOOL CJpegFile::DecryptJPEG(const std::string& /*csJpeg*/)
{
	return 0; // FALSE
}

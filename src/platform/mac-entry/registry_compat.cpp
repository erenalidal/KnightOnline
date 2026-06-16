// registry_compat.cpp — macOS: Win32 Registry'yi dosya-tabanlı kalıcı hale getirir.
//
// KO istemcisi hotkey/skill bar, kamera modu, yürü-koş durumu ve pencere konumlarını
// HKEY_CURRENT_USER altında REG_BINARY değerler olarak saklar (CGameProcedure::RegPutSetting/
// RegGetSetting → RegOpenKey/RegCreateKey/RegSetValueEx/RegQueryValueEx/RegCloseKey).
// macOS'ta bu API'ler no-op stub'dı → ayarlar her açılışta sıfırlanıyordu (skill bar boş!,
// kamera modu/pencere konumları unutuluyordu).
//
// Burada her registry "key" yolu (örn. Software\KnightOnline\<hesap>_<server>_<idx>) tek bir
// dosyaya eşlenir: ~/Library/Application Support/KnightOnline/<sanitized-key>.reg
// Dosya, value-name → ham byte (REG_BINARY) eşlemesidir: tekrarlı
// [u32 nameLen][name][u32 dataLen][data] uzunluk-önekli ikili format.
//
// See docs/MAC_PORT_ROADMAP.md.

#if defined(__APPLE__)

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "win_types.h" // HKEY/PHKEY/LONG/BYTE/DWORD/ERROR_SUCCESS/REG_BINARY + Reg* bildirimleri

namespace
{
namespace fs = std::filesystem;

struct RegKey
{
	fs::path                                             file;
	std::unordered_map<std::string, std::vector<uint8_t>> values;
};

std::mutex g_regMutex;

fs::path BaseDir()
{
	const char* home = std::getenv("HOME");
	fs::path    dir  = (home != nullptr && home[0] != '\0')
		    ? fs::path(home) / "Library" / "Application Support" / "KnightOnline"
		    : fs::temp_directory_path() / "KnightOnline";
	std::error_code ec;
	fs::create_directories(dir, ec);
	return dir;
}

// Key yolundaki güvensiz karakterleri ('\\', '/', ':' vb.) '_' yapar → dosya adı.
fs::path KeyToFile(const char* subkey)
{
	std::string name = (subkey != nullptr && subkey[0] != '\0') ? subkey : "default";
	for (char& c : name)
	{
		if (c == '\\' || c == '/' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<'
			|| c == '>' || c == '|')
			c = '_';
	}
	name += ".reg";
	return BaseDir() / name;
}

void LoadFile(RegKey* key)
{
	std::ifstream f(key->file, std::ios::binary);
	if (!f)
		return;
	while (f.peek() != EOF)
	{
		uint32_t nameLen = 0, dataLen = 0;
		if (!f.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen)))
			break;
		std::string vname(nameLen, '\0');
		if (nameLen > 0 && !f.read(vname.data(), nameLen))
			break;
		if (!f.read(reinterpret_cast<char*>(&dataLen), sizeof(dataLen)))
			break;
		std::vector<uint8_t> data(dataLen);
		if (dataLen > 0 && !f.read(reinterpret_cast<char*>(data.data()), dataLen))
			break;
		key->values[vname] = std::move(data);
	}
}

void SaveFile(const RegKey* key)
{
	std::ofstream f(key->file, std::ios::binary | std::ios::trunc);
	if (!f)
		return;
	for (const auto& kv : key->values)
	{
		uint32_t nameLen = static_cast<uint32_t>(kv.first.size());
		uint32_t dataLen = static_cast<uint32_t>(kv.second.size());
		f.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
		f.write(kv.first.data(), nameLen);
		f.write(reinterpret_cast<const char*>(&dataLen), sizeof(dataLen));
		if (dataLen > 0)
			f.write(reinterpret_cast<const char*>(kv.second.data()), dataLen);
	}
}
} // namespace

LONG RegOpenKeyA(HKEY, const char* subkey, PHKEY phkey)
{
	if (phkey == nullptr)
		return 1;
	std::lock_guard<std::mutex> lock(g_regMutex);
	fs::path                    file = KeyToFile(subkey);
	std::error_code             ec;
	if (!fs::exists(file, ec))
		return 1; // anahtar yok: Windows RegOpenKey gibi başarısız → çağıran RegCreateKey dener
	RegKey* key = new RegKey();
	key->file   = file;
	LoadFile(key);
	*phkey = reinterpret_cast<HKEY>(key);
	return ERROR_SUCCESS;
}

LONG RegCreateKeyA(HKEY, const char* subkey, PHKEY phkey)
{
	if (phkey == nullptr)
		return 1;
	std::lock_guard<std::mutex> lock(g_regMutex);
	RegKey*                     key = new RegKey();
	key->file                       = KeyToFile(subkey);
	LoadFile(key); // mevcut değerler varsa koru
	SaveFile(key); // dosyayı oluştur → sonraki RegOpenKey bulur
	*phkey = reinterpret_cast<HKEY>(key);
	return ERROR_SUCCESS;
}

LONG RegCloseKey(HKEY hkey)
{
	std::lock_guard<std::mutex> lock(g_regMutex);
	RegKey*                     key = reinterpret_cast<RegKey*>(hkey);
	if (key == nullptr)
		return ERROR_SUCCESS;
	SaveFile(key);
	delete key;
	return ERROR_SUCCESS;
}

LONG RegQueryValueExA(HKEY hkey, const char* name, DWORD*, DWORD* type, BYTE* data, DWORD* len)
{
	std::lock_guard<std::mutex> lock(g_regMutex);
	RegKey*                     key = reinterpret_cast<RegKey*>(hkey);
	if (key == nullptr || name == nullptr)
		return 1;
	auto it = key->values.find(name);
	if (it == key->values.end())
		return 1; // değer yok → RegGetSetting false döner, client default kullanır
	const std::vector<uint8_t>& v = it->second;
	if (type != nullptr)
		*type = REG_BINARY;
	if (data == nullptr)
	{
		if (len != nullptr) // boyut sorgusu
			*len = static_cast<DWORD>(v.size());
		return ERROR_SUCCESS;
	}
	DWORD avail = (len != nullptr) ? *len : 0;
	DWORD n     = static_cast<DWORD>(v.size());
	if (n > avail)
		n = avail; // tampona sığdığı kadar kopyala
	if (n > 0)
		std::memcpy(data, v.data(), n);
	if (len != nullptr)
		*len = static_cast<DWORD>(v.size());
	return ERROR_SUCCESS;
}

LONG RegSetValueExA(HKEY hkey, const char* name, DWORD, DWORD, const BYTE* data, DWORD len)
{
	std::lock_guard<std::mutex> lock(g_regMutex);
	RegKey*                     key = reinterpret_cast<RegKey*>(hkey);
	if (key == nullptr || name == nullptr)
		return 1;
	std::vector<uint8_t> v(len);
	if (len > 0 && data != nullptr)
		std::memcpy(v.data(), data, len);
	key->values[name] = std::move(v);
	SaveFile(key); // hemen kalıcı yap (RegCloseKey'i beklemeden)
	return ERROR_SUCCESS;
}

#endif // __APPLE__

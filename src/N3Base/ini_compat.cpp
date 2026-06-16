// ini_compat.cpp — macOS port: GetPrivateProfileString/Int için GERÇEK INI parser.
//
// Win32 .ini okuma API'si. Client `Server.Ini`'den server IP listesini okur
// (`[Server] Count`, `IP0`...). Stub varsayılanı döndürdüğünde iServerCount=0
// kalıyordu → "No server list / LogIn Server fail". Burada dosyayı gerçek parse
// ederiz: bölüm/anahtar büyük-küçük harf duyarsız, satır içi `;`/`#` yorumları,
// değer trim. Dosya yolu göreli ise çalışma dizinine göre çözülür.
//
// See docs/MAC_PORT_ROADMAP.md.

#if defined(__APPLE__)

#include "win_types.h" // DWORD/UINT/BOOL + GetPrivateProfile* bildirimleri

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

namespace
{
std::string TrimCopy(const std::string& s)
{
	size_t a = 0, b = s.size();
	while (a < b && std::isspace((unsigned char) s[a]))
		++a;
	while (b > a && std::isspace((unsigned char) s[b - 1]))
		--b;
	return s.substr(a, b - a);
}

bool IEquals(const std::string& a, const std::string& b)
{
	if (a.size() != b.size())
		return false;
	for (size_t i = 0; i < a.size(); ++i)
		if (std::tolower((unsigned char) a[i]) != std::tolower((unsigned char) b[i]))
			return false;
	return true;
}

// [section] altında key'in değerini bul. Bulunursa true + out doldurulur.
bool IniLookup(const char* file, const char* section, const char* key, std::string& out)
{
	if (file == nullptr || section == nullptr || key == nullptr)
		return false;

	// Motor yolları Windows ayracı '\' ile kurar (CN3Base::PathSet). macOS'ta
	// FileReader gibi '\' -> '/' normalize et, yoksa dosya açılmaz. (mac-port)
	std::string path = file;
	for (char& c : path)
		if (c == '\\')
			c = '/';

	std::ifstream in(path);
	if (!in.is_open())
		return false;

	std::string wantSec = section;
	std::string wantKey = key;
	std::string curSec;
	std::string line;
	bool        inSection = false;

	while (std::getline(in, line))
	{
		// Windows satır sonu \r temizle
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		std::string t = TrimCopy(line);
		if (t.empty() || t[0] == ';' || t[0] == '#')
			continue;

		if (t.front() == '[')
		{
			size_t end = t.find(']');
			if (end != std::string::npos)
			{
				curSec    = TrimCopy(t.substr(1, end - 1));
				inSection = IEquals(curSec, wantSec);
			}
			continue;
		}

		if (!inSection)
			continue;

		size_t eq = t.find('=');
		if (eq == std::string::npos)
			continue;

		std::string k = TrimCopy(t.substr(0, eq));
		if (!IEquals(k, wantKey))
			continue;

		std::string v = TrimCopy(t.substr(eq + 1));
		// Satır içi yorum (; veya #) — basit kes (tırnak içi desteklenmez)
		size_t cmt = v.find_first_of(";#");
		if (cmt != std::string::npos)
			v = TrimCopy(v.substr(0, cmt));
		out = v;
		return true;
	}
	return false;
}
} // namespace

DWORD GetPrivateProfileStringA(const char* section, const char* key, const char* def,
	char* ret, DWORD nSize, const char* file)
{
	if (ret == nullptr || nSize == 0)
		return 0;

	std::string value;
	if (!IniLookup(file, section, key, value))
		value = (def != nullptr) ? def : "";

	std::strncpy(ret, value.c_str(), nSize);
	ret[nSize - 1] = '\0';
	return static_cast<DWORD>(std::strlen(ret));
}

UINT GetPrivateProfileIntA(const char* section, const char* key, int def, const char* file)
{
	std::string value;
	if (!IniLookup(file, section, key, value) || value.empty())
		return static_cast<UINT>(def);

	// Win32: baştaki sayıyı parse eder, geçersizse 0/def. atoi yeterli.
	return static_cast<UINT>(std::atoi(value.c_str()));
}

#endif // __APPLE__

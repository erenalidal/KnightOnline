// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// io.h — Windows <io.h>'nin minimal şimi: _findfirst/_findnext/_findclose
// (dosya arama) POSIX opendir/readdir + fnmatch ile GERÇEKLENMİŞTİR.

#ifndef KO_PLATFORM_IO_H_
#define KO_PLATFORM_IO_H_

#pragma once

#if !defined(_WIN32)

#include <cstdint>
#include <cstring>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <libgen.h>
#include <string>

#include "win_types.h"

#ifndef _A_NORMAL
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20
#endif

struct _finddata_t
{
	unsigned    attrib;
	long        time_create;
	long        time_access;
	long        time_write;
	long        size;
	char        name[260];
};

// _findfirst için iç durum (dizin + glob deseni).
struct KO_FindState
{
	DIR*        dir;
	std::string dirPath;
	std::string pattern;
};

inline int KO_FillFind(KO_FindState* st, struct _finddata_t* fd)
{
	struct dirent* ent;
	while ((ent = ::readdir(st->dir)) != nullptr)
	{
		if (::fnmatch(st->pattern.c_str(), ent->d_name, 0) != 0)
			continue;
		::strncpy(fd->name, ent->d_name, sizeof(fd->name) - 1);
		fd->name[sizeof(fd->name) - 1] = '\0';
		std::string full = st->dirPath + "/" + ent->d_name;
		struct stat sb {};
		fd->attrib = 0;
		fd->size   = 0;
		fd->time_write = 0;
		if (::stat(full.c_str(), &sb) == 0)
		{
			fd->size       = static_cast<long>(sb.st_size);
			fd->time_write = static_cast<long>(sb.st_mtime);
			if (S_ISDIR(sb.st_mode)) fd->attrib |= _A_SUBDIR;
		}
		return 0;
	}
	return -1;
}

// _findfirst("dir/*.ext", &fd) → handle (intptr_t) ya da -1.
inline intptr_t _findfirst(const char* spec, struct _finddata_t* fd)
{
	if (spec == nullptr || fd == nullptr)
		return -1;
	std::string s(spec);
	std::string dirPath = ".";
	std::string pattern = s;
	size_t slash = s.find_last_of('/');
	if (slash != std::string::npos)
	{
		dirPath = s.substr(0, slash);
		pattern = s.substr(slash + 1);
	}
	DIR* d = ::opendir(dirPath.c_str());
	if (d == nullptr)
		return -1;
	KO_FindState* st = new KO_FindState{d, dirPath, pattern};
	if (KO_FillFind(st, fd) != 0)
	{
		::closedir(st->dir);
		delete st;
		return -1;
	}
	return reinterpret_cast<intptr_t>(st);
}

inline int _findnext(intptr_t handle, struct _finddata_t* fd)
{
	if (handle == -1 || handle == 0 || fd == nullptr)
		return -1;
	KO_FindState* st = reinterpret_cast<KO_FindState*>(handle);
	return KO_FillFind(st, fd);
}

inline int _findclose(intptr_t handle)
{
	if (handle == -1 || handle == 0)
		return -1;
	KO_FindState* st = reinterpret_cast<KO_FindState*>(handle);
	::closedir(st->dir);
	delete st;
	return 0;
}

#endif // !defined(_WIN32)

#endif // KO_PLATFORM_IO_H_

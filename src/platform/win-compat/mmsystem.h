// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// mmsystem.h — Windows multimedya başlığının minimal şimi.
// N3Base yalnızca timeGetTime() (ve birkaç yerde PlaySound) kullanır.

#ifndef KO_PLATFORM_MMSYSTEM_H_
#define KO_PLATFORM_MMSYSTEM_H_

#pragma once

#if !defined(_WIN32)

#include <cstdint>
#include <mach/mach_time.h>

#include "win_types.h"

// timeGetTime: sistem başlangıcından beri geçen milisaniye (DWORD).
inline DWORD timeGetTime()
{
	static mach_timebase_info_data_t s_tb = {0, 0};
	if (s_tb.denom == 0)
		mach_timebase_info(&s_tb);
	uint64_t nanos = (mach_absolute_time() * s_tb.numer) / s_tb.denom;
	return static_cast<DWORD>(nanos / 1000000ULL);
}

inline unsigned int timeBeginPeriod(unsigned int) { return 0; }
inline unsigned int timeEndPeriod(unsigned int) { return 0; }

// PlaySound stub (ses N3SndMgr/OpenAL üzerinden çalışır; bu yol kullanılmıyor).
#ifndef SND_ASYNC
#define SND_ASYNC    0x0001
#define SND_FILENAME 0x00020000
#endif
inline BOOL PlaySound(const char*, void*, DWORD) { return FALSE; }
inline BOOL PlaySoundA(const char*, void*, DWORD) { return FALSE; }

#endif // !defined(_WIN32)

#endif // KO_PLATFORM_MMSYSTEM_H_

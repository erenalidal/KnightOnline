// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// winsock2.h — Winsock API'sinin BSD socket karşılığı.
//
// WarFare ağ katmanı (APISocket) Winsock kullanır. macOS POSIX socket'leri
// neredeyse aynı API'yi sağlar; bu başlık yalnızca Winsock'a özgü isimleri
// (SOCKET, WSAStartup, closesocket, WSAGetLastError ...) POSIX'e map'ler.
//
// NOT: Bu, gerçek ve çalışan bir karşılıktır (stub değil) — BSD socket'leri
// fonksiyoneldir. Async model (WSAAsyncSelect/WM_SOCKETMSG) Faz 5'te SDL/asio
// ile ele alınacak; senkron socket çağrıları olduğu gibi çalışır.

#ifndef KO_PLATFORM_WINSOCK2_H_
#define KO_PLATFORM_WINSOCK2_H_

#pragma once

#if !defined(_WIN32)

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>

#include "win_types.h"

typedef int SOCKET;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#endif

// WSADATA / başlatma — POSIX'te gereksiz; no-op.
typedef struct WSAData
{
	WORD           wVersion;
	WORD           wHighVersion;
	char           szDescription[257];
	char           szSystemStatus[129];
	unsigned short iMaxSockets;
	unsigned short iMaxUdpDg;
	char*          lpVendorInfo;
} WSADATA, *LPWSADATA;

#ifndef MAKEWORD
#define MAKEWORD(lo, hi) ((WORD) (((BYTE) (lo)) | (((WORD) ((BYTE) (hi))) << 8)))
#endif

inline int WSAStartup(WORD, LPWSADATA lpData) { if (lpData) *lpData = WSADATA{}; return 0; }
inline int WSACleanup() { return 0; }
inline int WSAGetLastError() { return errno; }

inline int closesocket(SOCKET s) { return ::close(s); }

// ioctlsocket: yalnızca FIONBIO (non-blocking) için kullanılır → fcntl.
inline int ioctlsocket(SOCKET s, long cmd, unsigned long* argp)
{
	if (cmd == FIONBIO)
	{
		int flags = ::fcntl(s, F_GETFL, 0);
		if (flags < 0) return SOCKET_ERROR;
		if (argp && *argp) flags |= O_NONBLOCK; else flags &= ~O_NONBLOCK;
		return ::fcntl(s, F_SETFL, flags) < 0 ? SOCKET_ERROR : 0;
	}
	return ::ioctl(s, cmd, argp);
}

// --- Async socket modeli (WSAAsyncSelect) — Faz 5'te asio/poll ile değişecek ---
// WarFare, socket olaylarını pencere mesajına (WM_SOCKETMSG) bağlar. macOS'ta
// bu stub'tır; gerçek non-blocking I/O Faz 5'te ele alınacak.
#ifndef FD_READ
#define FD_READ    0x01
#define FD_WRITE   0x02
#define FD_OOB     0x04
#define FD_ACCEPT  0x08
#define FD_CONNECT 0x10
#define FD_CLOSE   0x20
#endif
#ifndef WSAGETSELECTEVENT
#define WSAGETSELECTEVENT(l) ((l) & 0xFFFF)
#define WSAGETSELECTERROR(l) (((l) >> 16) & 0xFFFF)
#define WSAMAKESELECTREPLY(e, err) ((LPARAM) (((WORD) (e)) | (((DWORD) ((WORD) (err))) << 16)))
#endif
inline int WSAAsyncSelect(SOCKET, HWND, unsigned int, long) { return 0; }

// Winsock hata kodları (sık kullanılanlar) → POSIX errno karşılığı.
#ifndef WSAEWOULDBLOCK
#define WSAEWOULDBLOCK  EWOULDBLOCK
#define WSAEINPROGRESS  EINPROGRESS
#define WSAECONNRESET   ECONNRESET
#define WSAECONNREFUSED ECONNREFUSED
#define WSAETIMEDOUT    ETIMEDOUT
#endif

#endif // !defined(_WIN32)

#endif // KO_PLATFORM_WINSOCK2_H_

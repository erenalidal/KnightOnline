#ifndef SERVER_SHAREDSERVER_INTERPROCESSMUTEX_H
#define SERVER_SHAREDSERVER_INTERPROCESSMUTEX_H

#pragma once

#include <memory>
#include <string>

// GÜVENLIK (#23 M7): Süreçler-arası (Ebenezer <-> Aujard) atomik bölge için named mutex.
// boost::interprocess detaylarını pImpl ile gizliyoruz (SharedMemoryBlock pattern'i).
struct named_mutex_impl;

class InterprocessMutex
{
public:
	// open_or_create semantiği: mutex yoksa oluşturur, varsa açar.
	// removeStaleFirst=true: oluşturmadan önce aynı isimli stale named mutex'i kaldırır. boost
	// named_mutex (macOS'ta POSIX semaphore) temiz çıkışta bile silinmez; bir süreç kilidi
	// TUTARKEN crash ederse semaphore kilitli (value 0) kalır → sonraki açan her lock 3sn timeout'a
	// düşer. İLK başlayan süreç (Ebenezer, restart sırasında Aujard'dan önce) bunu true geçer ve
	// taze, kilitsiz bir mutex garanti eder. Sonraki süreçler (Aujard) false geçip ona bağlanır.
	explicit InterprocessMutex(const std::string& name, bool removeStaleFirst = false);

	// boost date_time'a bağımlı OLMADAN: try_lock() spin + 5ms sleep ile timeoutMs'e kadar dener.
	// Kilit alınırsa true, timeout'a kadar alınamazsa false döner. Exception sızdırmaz.
	bool TimedLock(int timeoutMs);

	// Kilidi serbest bırakır. Exception sızdırmaz.
	void Unlock();

	~InterprocessMutex();

	// Kopyalama yasak (named_mutex sahipliği tekil).
	InterprocessMutex(const InterprocessMutex&)            = delete;
	InterprocessMutex& operator=(const InterprocessMutex&) = delete;

private:
	std::string _name;
	std::unique_ptr<named_mutex_impl> _mutex;
};

// RAII guard: ctor'da TimedLock, dtor'da (kilitliyse) Unlock.
// Locked() ile kilidin gerçekten alınıp alınmadığı kontrol edilebilir (crash senaryosunda
// timeout sonrası çağıran "yine de devam et" diyebilsin diye).
class InterprocessMutexGuard
{
public:
	InterprocessMutexGuard(InterprocessMutex& mutex, int timeoutMs)
		: _mutex(mutex)
	{
		_locked = _mutex.TimedLock(timeoutMs);
	}

	~InterprocessMutexGuard()
	{
		if (_locked)
			_mutex.Unlock();
	}

	bool Locked() const { return _locked; }

	InterprocessMutexGuard(const InterprocessMutexGuard&)            = delete;
	InterprocessMutexGuard& operator=(const InterprocessMutexGuard&) = delete;

private:
	InterprocessMutex& _mutex;
	bool _locked = false;
};

#endif // SERVER_SHAREDSERVER_INTERPROCESSMUTEX_H

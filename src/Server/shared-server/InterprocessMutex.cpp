#include "pch.h"
#include "InterprocessMutex.h"

#include <boost/interprocess/sync/named_mutex.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <thread>

using namespace boost::interprocess;

// GÜVENLIK (#23 M7): boost named_mutex'i pImpl ile izole et (SharedMemoryBlock pattern'i).
struct named_mutex_impl : public named_mutex
{
	using named_mutex::named_mutex;
};

InterprocessMutex::InterprocessMutex(const std::string& name, bool removeStaleFirst)
	: _name(name)
{
	// GÜVENLIK (#23 M7): boost named_mutex macOS'ta POSIX semaphore — temiz çıkışta bile silinmez
	// ve restart script'in /private/tmp/boost_interprocess rm'i onu temizlemez (dosya değil, kernel
	// objesi). Bir süreç kilidi TUTARKEN crash ederse semaphore kilitli (value 0) kalır → sonraki
	// açan her lock 3sn timeout'a düşer. İLK başlayan süreç (Ebenezer) removeStaleFirst=true geçip
	// taze, kilitsiz mutex garanti eder. Başka süreç ona bağlıyken kaldırmak ayrıştırırdı; bu yüzden
	// yalnızca ilk-başlayan true geçer (restart sırası Ebenezer'ı Aujard'dan önce başlatır).
	if (removeStaleFirst)
	{
		try
		{
			named_mutex::remove(name.c_str());
		}
		catch (const interprocess_exception& ex)
		{
			spdlog::warn("InterprocessMutex: stale mutex remove failed (yok sayılır). name='{}' ex={}",
				name, ex.what());
		}
	}

	try
	{
		_mutex = std::make_unique<named_mutex_impl>(open_or_create, name.c_str());
	}
	catch (const interprocess_exception& ex)
	{
		spdlog::error("InterprocessMutex: failed to open_or_create named mutex. name='{}' ex={}",
			name, ex.what());
		_mutex.reset();
	}
}

bool InterprocessMutex::TimedLock(int timeoutMs)
{
	// Mutex oluşturulamadıysa kilit alınamaz; çağıran Locked()==false ile devam kararı verir.
	if (_mutex == nullptr)
		return false;

	using clock     = std::chrono::steady_clock;
	const auto start = clock::now();

	// boost date_time'a bağımlı OLMA: try_lock() spin + 5ms sleep ile timeout'a kadar dene.
	for (;;)
	{
		try
		{
			if (_mutex->try_lock())
				return true;
		}
		catch (const interprocess_exception& ex)
		{
			spdlog::error("InterprocessMutex::TimedLock: try_lock failed. name='{}' ex={}",
				_name, ex.what());
			return false;
		}

		const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
			clock::now() - start).count();
		if (elapsedMs >= timeoutMs)
			return false;

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

void InterprocessMutex::Unlock()
{
	if (_mutex == nullptr)
		return;

	try
	{
		_mutex->unlock();
	}
	catch (const interprocess_exception& ex)
	{
		spdlog::error("InterprocessMutex::Unlock: unlock failed. name='{}' ex={}",
			_name, ex.what());
	}
}

InterprocessMutex::~InterprocessMutex()
{
	// Not: named_mutex'i kasıtlı olarak remove ETMİYORUZ — diğer süreç (Ebenezer/Aujard) hâlâ
	// kullanıyor olabilir. Stale-lock temizliği için ilk-başlayan sürecin removeStaleFirst=true
	// constructor'ı sorumlu (yukarı bkz); deadlock'a karşı ikinci savunma TimedLock timeout'u.
	_mutex.reset();
}

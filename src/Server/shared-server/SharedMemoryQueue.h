#ifndef SERVER_SHAREDSERVER_SHAREDMEMORYQUEUE_H
#define SERVER_SHAREDSERVER_SHAREDMEMORYQUEUE_H

#pragma once

#include <memory>

inline constexpr int SMQ_ERROR_RANGE = 10000;

enum e_SharedMemQueueError : int16_t
{
	SMQ_OK   = 1,
	SMQ_FULL = SMQ_ERROR_RANGE,
	SMQ_EMPTY,
	SMQ_PKTSIZEOVER,
	SMQ_GENERIC_ERROR
};

struct message_queue_impl;
class SharedMemoryQueue
{
public:
	// 512 → 1024: logout'ta güncel envanter (42 slot, ~691B) Aujard'a gönderiliyor (item kaybı
	// düzeltmesi). Eski 512 sınırı paketi reddediyordu (SMQ_PKTSIZEOVER). Bkz. User::LogOut.
	static constexpr uint32_t MAX_MSG_SIZE = 1024;
	static constexpr uint32_t MAX_NUM_MSG  = 4096;

	bool IsOpen() const
	{
		return _queue != nullptr;
	}

	SharedMemoryQueue(int sendRetryCount = 0);
	bool Create(const char* name);
	bool OpenOrCreate(const char* name);
	bool Open(const char* name);
	int GetData(char* pBuf);
	int PutData(const char* pBuf, int size);
	~SharedMemoryQueue();

private:
	void FlushQueue();

private:
	std::unique_ptr<message_queue_impl> _queue;
	int _sendRetryCount;
};

#endif // SERVER_SHAREDSERVER_SHAREDMEMORYQUEUE_H

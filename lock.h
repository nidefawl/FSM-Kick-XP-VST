#ifndef __LOCK_H__
#define __LOCK_H__

#ifdef WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


class WIN32_CriticalSection {
	CRITICAL_SECTION g_critSec;
public:
	~WIN32_CriticalSection() {
		DeleteCriticalSection(&g_critSec);

	}
	WIN32_CriticalSection() {
		InitializeCriticalSection(&g_critSec);
	}
	void lock() {
		EnterCriticalSection(&g_critSec);
	}
	void unlock() {
		LeaveCriticalSection(&g_critSec);
	}
};

typedef WIN32_CriticalSection Lock;

#else

#include <mutex>


class STDMutex {
	std::mutex g_mutex;
public:
	void lock() {
		g_mutex.lock();
	}
	void unlock() {
		g_mutex.unlock();
	}

};

typedef STDMutex Lock;

#endif

#endif // __LOCK_H__
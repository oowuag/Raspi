#ifndef ZSYNCOBJ_H
#   include "ZSyncObj.h"
#endif


#if defined(WIN32)
	ZSyncObj::ZSyncObj()
	{
		lock_count = 0;
		::InitializeCriticalSection(&cs);
	}

	ZSyncObj::~ZSyncObj()
	{
		::DeleteCriticalSection(&cs);
	}

	void ZSyncObj::SyncStart()
	{
		::EnterCriticalSection(&cs);
	#ifdef ZDEBUG
		if(lock_count >= 1){
			printf("ZSyncObj::SyncStart Error\n");
		}
		lock_count ++;
	#endif
	}

	bool ZSyncObj::TrySyncStart()
	{
		return ::TryEnterCriticalSection(&cs);
	}
	void ZSyncObj::SyncEnd()
	{
	#ifdef ZDEBUG
		if(lock_count < 1){
			printf("ZSyncObj::SyncStart SyncEnd\n");
		}
		lock_count --;
	#endif
		::LeaveCriticalSection(&cs);
	}

#else

	ZSyncObj::ZSyncObj()
	{
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&mutex, &attr);
	}

	ZSyncObj::~ZSyncObj()
	{
		pthread_mutexattr_destroy(&attr);
		pthread_mutex_destroy(&mutex);
	}

	VOID ZSyncObj::SyncStart()
	{
		pthread_mutex_lock(&mutex);
	}

	bool ZSyncObj::TrySyncStart()
	{
		int ret = pthread_mutex_trylock(&mutex);
		return (ret==0)? true:false;
	}

	VOID ZSyncObj::SyncEnd()
	{
		pthread_mutex_unlock(&mutex);
	}
#endif

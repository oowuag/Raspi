#ifndef ZSYNCOBJ_H
#define ZSYNCOBJ_H

#ifndef ZOBJECT_H
	#include "ZObject.h"
#endif

#if defined(WIN32)
#else
	#include <pthread.h>
#endif


class ZSyncObj : public ZObject
{
protected:
#if defined(WIN32)
	CRITICAL_SECTION cs;
	long lock_count;
#else
	pthread_mutex_t mutex;
	pthread_mutexattr_t attr;
#endif

private:
	ZSyncObj(const ZSyncObj& src);
	ZSyncObj& operator = (const ZSyncObj& src);

public:
	/**
	* Construction.
	*/
	ZSyncObj();

	/**
	* Destruction.
	*/
	virtual ~ZSyncObj();

	/**
	* Synchronize start.
	*/
	VOID SyncStart();

	/**
	* Try synchronize start
	*
	* @return bool : true means synchronize succeed, and false failed.
	*/
	bool TrySyncStart();

	/**
	* Synchronize end.
	*/
	VOID SyncEnd();
};

#endif //ZSYNCOBJ_H

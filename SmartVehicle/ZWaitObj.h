#ifndef ZWAITOBJ_H
#define ZWAITOBJ_H

#ifndef ZOBJECT_H
	#include "ZObject.h"
#endif

#if defined(WIN32)
#else
	#include <pthread.h>
#endif

class ZWaitObj : public ZObject
{
public:
	/**
	* Construction.
	*
	* @param manual
	* - true : Creates a manual-reset event object
	* - false : Creates an auto-reset event object
	*/
	explicit ZWaitObj(bool manual = false);

	/**
	* Destruction.
	*/
	virtual ~ZWaitObj();

	/**
	* wait for single objects
	*
	* @param msec : Time-out interval, in milliseconds.
	*
	* @return bool : true means succeed, and false failed.
	*/
	bool Wait(DWORD msec = INFINITE);

	/**
	* Set the specified event object to the signaled state.
	*/
	VOID Notify();

	/**
	* Set the specified event object to the nonsignaled state
	*/
	void Reset();

protected:
#if defined(WIN32)
	HANDLE	hEvent;
#else
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_condattr_t condattr;
	bool signal_flag;
	bool manual_flag;
#endif

private:
	ZWaitObj(const ZWaitObj& src);
	ZWaitObj& operator = (const ZWaitObj& src);
};

#endif // ZWAITOBJ_H

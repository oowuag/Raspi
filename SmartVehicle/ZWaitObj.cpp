#ifndef ZWAITOBJ_H
#   include "ZWaitObj.h"
#endif

#include <asm/errno.h>

void gettimespec(struct timespec *time,  int msec)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	time->tv_sec = now.tv_sec + msec/1000;
	time->tv_nsec = now.tv_nsec + (msec%1000)*1000000;

	//bigger than one second
	if (time->tv_nsec >= 1000000000) {
		time->tv_sec ++;
		time->tv_nsec -= 1000000000;
	}
}

#if defined(WIN32)
	ZWaitObj::ZWaitObj(bool manual)
	{
		hEvent = ::CreateEvent(NULL, manual, false, NULL);
	}

	ZWaitObj::~ZWaitObj()
	{
		if(hEvent) ::CloseHandle(hEvent);
	}

	bool ZWaitObj::Wait(DWORD msec)
	{
		if(::WaitForSingleObject(hEvent, msec) == WAIT_OBJECT_0) return true;
		else return false;
	}

	void ZWaitObj::Notify()
	{
		::SetEvent(hEvent);
	}

	void ZWaitObj::Reset()
	{
		::ResetEvent(hEvent);
	}

#else

    ZWaitObj::ZWaitObj(bool manual)
    {
        pthread_mutex_init(&mutex, NULL);

        pthread_condattr_init(&condattr);
        //pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);

        pthread_cond_init(&cond, &condattr);

        signal_flag = false;
        manual_flag = manual;
    }

    ZWaitObj::~ZWaitObj()
    {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
        pthread_condattr_destroy(&condattr);
    }

    bool ZWaitObj::Wait(DWORD msec)
    {
        pthread_mutex_lock(&mutex);
        int ret = 0;
        if(signal_flag == false) {
            if(INFINITE != msec) {
                struct timespec nptime;
                gettimespec(&nptime, msec);
                ret = pthread_cond_timedwait(&cond, &mutex, &nptime);
                if (ret && ret != ETIMEDOUT) {
                    printf("wait failed, [%s]\n", strerror(ret));
                }
            }
            else {
                ret = pthread_cond_wait(&cond, &mutex);
            }
        }
        if (!manual_flag) signal_flag = false;
        pthread_mutex_unlock(&mutex);

        return (ret==0)? true:false;
    }

    VOID ZWaitObj::Notify()
    {
        pthread_mutex_lock(&mutex);
        signal_flag = true;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    VOID ZWaitObj::Reset()
    {
        pthread_mutex_lock(&mutex);
        signal_flag = false;
        pthread_mutex_unlock(&mutex);
    }

#endif

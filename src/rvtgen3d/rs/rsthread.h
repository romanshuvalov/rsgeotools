#ifndef RSTHREAD_H_INCLUDED
#define RSTHREAD_H_INCLUDED


#ifdef RS_WIN

    // WIN32 THREADS

    #include "rsplatform.h"

    #include <windows.h>
    #include <process.h>

    #define RS_MUTEX_T CRITICAL_SECTION

    #define RS_MUTEX_LOCK(x) EnterCriticalSection(x)
    #define RS_MUTEX_UNLOCK(x) LeaveCriticalSection(x)
    #define RS_MUTEX_INIT(x) InitializeCriticalSection(x)

    #define RS_THREAD_CREATE(thread,func,arg) thread = _beginthread(func,0,arg)
    #define RS_THREAD_RETURN(retval) return
    #define RS_THREAD_JOIN(thread) WaitForSingleObject(thread,INFINITE)

    #define RS_THREAD_T uintptr_t

    #define RS_THREAD_FUNC_T void

    #define RS_SET_THREAD_PRIORITY(thread,priority) SetThreadPriority(thread, priority)
    #define RS_GET_THREAD_PRIORITY(thread) GetThreadPriority(HANDLE hThread)
    #define RS_THREAD_PRIORITY_NORMAL THREAD_PRIORITY_NORMAL
    #define RS_THREAD_PRIORITY_HIGHER THREAD_PRIORITY_ABOVE_NORMAL

#else

    // POSIX THREADS

    #include <pthread.h>

    #define RS_MUTEX_T pthread_mutex_t

    #define RS_MUTEX_LOCK(x) pthread_mutex_lock(x)
    #define RS_MUTEX_UNLOCK(x) pthread_mutex_unlock(x)


    #define RS_MUTEX_INIT(x) pthread_mutex_init(x,NULL)

    #define RS_THREAD_CREATE(thread,func,arg) pthread_create(&thread, NULL, func, (void*)arg)
    #define RS_THREAD_RETURN(retval) pthread_exit(retval)
    #define RS_THREAD_JOIN(thread) pthread_join(thread,NULL)

    #define RS_THREAD_KILL(thread) pthread_kill(thread,0)

    #define RS_THREAD_T pthread_t

    #define RS_THREAD_FUNC_T void*

#endif

#endif // RSTHREAD_H_INCLUDED

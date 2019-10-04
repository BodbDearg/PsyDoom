#ifndef __MDFN_MTHREADING_H
#define __MDFN_MTHREADING_H

namespace Mednafen
{
namespace MThreading
{

// Mostly based off SDL's prototypes for compatibility with existing code.  Semantics are a bit different, however.
//
// Caution: Do not attempt to use the synchronization primitives(mutex, cond variables, etc.) for inter-process synchronization, they'll only work reliably with
// intra-process synchronization(the "mutex" is implemented as a a critical section under Windows, for example).
//
struct Thread;
struct Mutex;
struct Cond;	// mmm condiments
struct Sem;

Thread* CreateThread(int (*fn)(void *), void *data, const char* debug_name = nullptr);
void WaitThread(Thread *thread, int *status);
uintptr_t ThreadID(void);

Mutex *CreateMutex(void) MDFN_COLD;
void DestroyMutex(Mutex *mutex) MDFN_COLD;

int LockMutex(Mutex *mutex);
int UnlockMutex(Mutex *mutex);

Cond* CreateCond(void) MDFN_COLD;
void DestroyCond(Cond* cond) MDFN_COLD;

/* SignalCond() *MUST* be called with a lock on the mutex used with WaitCond() or WaitCondTimeout() */
int SignalCond(Cond* cond);
int WaitCond(Cond* cond, Mutex* mutex);

enum { COND_TIMEDOUT = 1 };
int WaitCondTimeout(Cond* cond, Mutex* mutex, unsigned ms);

Sem* CreateSem(void);
void DestroySem(Sem* sem);

int WaitSem(Sem* sem);
enum { SEM_TIMEDOUT = 1 };
int WaitSemTimeout(Sem* sem, unsigned ms);
int PostSem(Sem* sem);

}
}

#endif

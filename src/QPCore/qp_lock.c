
/**
  * Copyright (C) 2sui.
  */


#include "qp_o_ipc.h"


/*
 * * Copy from nginx. *
 * 
 * All modern pthread mutex implementations try to acquire a lock
 * atomically in userland before going to sleep in kernel.  Some
 * spins before the sleeping.
 *
 * In Solaris since version 8 all mutex types spin before sleeping.
 * The default spin count is 1000.  It can be overridden using
 * _THREAD_ADAPTIVE_SPIN=100 environment variable.
 *
 * In MacOSX all mutex types spin to acquire a lock protecting a mutex's
 * internals.  If the mutex is busy, thread calls Mach semaphore_wait().
 *
 *
 * PTHREAD_MUTEX_NORMAL lacks deadlock detection and is the fastest
 * mutex type.
 *
 *   Linux:    No spinning.  The internal name PTHREAD_MUTEX_TIMED_NP
 *             remains from the times when pthread_mutex_timedlock() was
 *             non-standard extension.  Alias name: PTHREAD_MUTEX_FAST_NP.
 *   FreeBSD:  No spinning.
 *
 *
 * PTHREAD_MUTEX_ERRORCHECK is usually as fast as PTHREAD_MUTEX_NORMAL
 * yet has lightweight deadlock detection.
 *
 *   Linux:    No spinning.  The internal name: PTHREAD_MUTEX_ERRORCHECK_NP.
 *   FreeBSD:  No spinning.
 *
 *
 * PTHREAD_MUTEX_RECURSIVE allows recursive locking.
 *
 *   Linux:    No spinning.  The internal name: PTHREAD_MUTEX_RECURSIVE_NP.
 *   FreeBSD:  No spinning.
 *
 *
 * PTHREAD_MUTEX_ADAPTIVE_NP spins on SMP systems before sleeping.
 *
 *   Linux:    No deadlock detection.  Dynamically changes a spin count
 *             for each mutex from 10 to 100 based on spin count taken
 *             previously.
 *   FreeBSD:  Deadlock detection.  The default spin count is 2000.
 *             It can be overriden using LIBPTHREAD_SPINLOOPS environment
 *             variable or by pthread_mutex_setspinloops_np().  If a lock
 *             is still busy, sched_yield() can be called on both UP and
 *             SMP systems.  The default yield loop count is zero, but
 *             it can be set by LIBPTHREAD_YIELDLOOPS environment
 *             variable or by pthread_mutex_setyieldloops_np().
 *   Solaris:  No PTHREAD_MUTEX_ADAPTIVE_NP.
 *   MacOSX:   No PTHREAD_MUTEX_ADAPTIVE_NP.
 *
 *
 * PTHREAD_MUTEX_ELISION_NP is a Linux extension to elide locks using
 * Intel Restricted Transactional Memory.  It is the most suitable for
 * rwlock pattern access because it allows simultaneous reads without lock.
 * Supported since glibc 2.18.
 *
 *
 * PTHREAD_MUTEX_DEFAULT is default mutex type.
 *
 *   Linux:    PTHREAD_MUTEX_NORMAL.
 *   FreeBSD:  PTHREAD_MUTEX_ERRORCHECK.
 *   Solaris:  PTHREAD_MUTEX_NORMAL.
 *   MacOSX:   PTHREAD_MUTEX_NORMAL.
 */


inline void
qp_lock_set_inited(qp_lock_t* lock)
{ lock->is_inited = true;}

inline void
qp_lock_set_alloced(qp_lock_t* lock)
{ lock->is_alloced = true;}

inline void
qp_lock_set_shared(qp_lock_t* lock)
{ lock->is_shared = true;}

inline void
qp_lock_unset_inited(qp_lock_t* lock)
{ lock->is_inited = false;}

inline void
qp_lock_unset_alloced(qp_lock_t* lock)
{ lock->is_alloced = false;}

inline void
qp_lock_unset_shared(qp_lock_t* lock)
{ lock->is_shared = false;}

inline void
qp_lock_set_spin(qp_lock_t* lock)
{ lock->is_spin = true;}

inline void
qp_lock_unset_spin(qp_lock_t* lock)
{ lock->is_spin = false;}


/**
 *   create lock.
 */
qp_lock_t*
qp_lock_create(qp_lock_t *lock, bool shared, bool spin)
{
    if (NULL == lock) {
        lock = (qp_lock_t*)qp_alloc(sizeof(qp_lock_t));
        
        if (NULL == lock) {
            QP_LOGOUT_ERROR("[qp_lock_t] Lock create fail.");
            return NULL;
        }

        memset(lock, 0, sizeof(qp_lock_t));
        qp_lock_set_alloced(lock);

    } else {
        memset(lock, 0, sizeof(qp_lock_t));
    }
    
    if (spin) {
        shared ? qp_lock_set_shared(lock) : 1;
        qp_lock_set_spin(lock);
        qp_lock_set_inited(lock);
        return lock;
    }
    
    pthread_mutexattr_t    attr;

    if (QP_SUCCESS != pthread_mutexattr_init(&attr)) {
        qp_lock_is_alloced(lock) ? qp_free(lock) : 1;
        QP_LOGOUT_ERROR("[qp_lock_t] Lock create attr fail.");
        return NULL;
    }
    
    if (QP_SUCCESS != pthread_mutexattr_settype(&attr, \
        PTHREAD_MUTEX_ERRORCHECK))
    {
        qp_lock_is_alloced(lock) ? qp_free(lock) : 1;
        QP_LOGOUT_ERROR("[qp_lock_t] Lock set attr fail.");
        return NULL;
    }
    
#ifdef _POSIX_THREAD_PROCESS_SHARED
    if (shared) {
        
        if (QP_SUCCESS != pthread_mutexattr_setpshared(&attr, \
            PTHREAD_PROCESS_SHARED))
        {
            qp_lock_is_alloced(lock) ? qp_free(lock) : 1;
            QP_LOGOUT_ERROR("[qp_lock_t] Lock set attr [shared] fail.");
            return NULL;
        }
        
        qp_lock_set_shared(lock);
    }
#endif
    
    if (QP_SUCCESS != pthread_mutex_init(&(lock->lock.mutex), &attr)) {
        qp_lock_is_alloced(lock) ? qp_free(lock) : 1;
        QP_LOGOUT_ERROR("[qp_lock_t] Lock init fail.");
        return NULL;
    }
    
    pthread_mutexattr_destroy(&attr);
    qp_lock_set_inited(lock);
    return lock;
}

qp_lock_t*
qp_lock_init(qp_lock_t *lock, bool shared, bool spin)
{
    return qp_lock_create(lock, shared, spin);
}

qp_int_t
qp_lock_destroy(qp_lock_t *lock)
{
    if (qp_lock_is_inited(lock)) {
        /* incase the lock still locked */
        if (EBUSY == qp_lock_trylock(lock)) {
            QP_LOGOUT_ERROR("[qp_lock_t] Lock has not unlocked.");
            return QP_ERROR;
        }
        
        qp_lock_unlock(lock);
        
        if (!qp_lock_is_spin(lock)) {
            if (QP_SUCCESS != pthread_mutex_destroy(&(lock->lock.mutex))) {
                QP_LOGOUT_ERROR("[qp_lock_t] Lock destroy error.");
                return QP_ERROR;
            }
        }
        
        qp_lock_unset_spin(lock);
        qp_lock_unset_shared(lock);
        qp_lock_unset_inited(lock);
        
        if (qp_lock_is_alloced(lock)) {
            qp_free(lock);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

qp_int_t
qp_lock_lock(qp_lock_t *lock)
{
    qp_uint_t  n = 1024, i, j;
    
    if (qp_lock_is_spin(lock)) {
        
        for ( ;; ) {
            
            if (lock->lock.spin == 0 && qp_atom_cmp_set(&lock->lock.spin,0,1)) {
                return QP_SUCCESS;
            }
            
            if (qp_cpu_num > 1) {
                
                for (i = 1; i < n; i <<= 1) {
                    
                    for (j = 0; j < i; j++) {
                        qp_cpu_pause();
                    }
                    
                    if (lock->lock.spin == 0 && \
                        qp_atom_cmp_set(&lock->lock.spin, 0, 1)) 
                    {
                        return QP_SUCCESS;
                    }
                }
            }
            
            qp_sched_yield();
        }
    }
    
    return pthread_mutex_lock(&(lock->lock.mutex));
}

qp_int_t
qp_lock_trylock(qp_lock_t *lock)
{
    if (qp_lock_is_spin(lock)) {
        
        if (lock->lock.spin == 0 && qp_atom_cmp_set(&lock->lock.spin, 0, 1)) {
            return QP_SUCCESS;
            
        } else {
            return EBUSY;
        }
    }
    
    return pthread_mutex_trylock(&(lock->lock.mutex));
}


qp_int_t
qp_lock_unlock(qp_lock_t *lock)
{
    if (qp_lock_is_spin(lock)) {
        return (lock->lock.spin == 1 && qp_atom_cmp_set(&lock->lock.spin, 1, 0));
    }
    
    return pthread_mutex_unlock(&(lock->lock.mutex));
}


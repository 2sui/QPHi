
/**
  * Copyright (C) 2sui.
  */


#include "qp_o_ipc.h"


bool
qp_lock_is_inited(qp_lock_t lock)
{ return lock ? lock->is_inited : false; }

bool
qp_rwlock_is_inited(qp_rwlock_t rwlock)
{ return rwlock ? rwlock->is_inited : false; }

bool
qp_cond_is_inited(qp_cond_t cond)
{ return cond ? cond->is_inited : false; }

bool
qp_sem_is_inited(qp_sem_t sem)
{ return sem ? sem->is_inited : false; }

bool
qp_shm_is_inited(qp_shm_t shm)
{ return shm ? shm->is_inited : false; }

bool
qp_lock_is_alloced(qp_lock_t lock)
{ return lock ? lock->is_alloced : false; }

bool
qp_rwlock_is_alloced(qp_rwlock_t rwlock)
{ return rwlock ? rwlock->is_alloced : false; }

bool
qp_cond_is_alloced(qp_cond_t cond)
{ return cond ? cond->is_alloced : false; }

bool
qp_sem_is_alloced(qp_sem_t sem)
{ return sem ? sem->is_alloced : false; }

bool
qp_shm_is_alloced(qp_shm_t shm)
{ return shm ? shm->is_alloced : false; }

bool
qp_lock_is_shared(qp_lock_t lock)
{ return lock ? lock->is_shared : false; }

bool
qp_rwlock_is_shared(qp_rwlock_t rwlock)
{ return rwlock ? rwlock->is_shared : false; }

bool
qp_cond_is_shared(qp_cond_t cond)
{ return cond ? cond->is_shared : false; }

bool
qp_sem_is_shared(qp_sem_t sem)
{ return sem ? sem->is_shared : false; }

bool
qp_lock_is_spin(qp_lock_t lock)
{ return lock ? lock->is_spin : false; }

// MARK: lock

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


void
qp_lock_set_inited(qp_lock_t lock)
{ lock ? lock->is_inited = true : 1;}

void
qp_lock_set_alloced(qp_lock_t lock)
{ lock ? lock->is_alloced = true : 1;}

void
qp_lock_set_shared(qp_lock_t lock)
{ lock ? lock->is_shared = true : 1;}

void
qp_lock_unset_inited(qp_lock_t lock)
{ lock ? lock->is_inited = false : 1;}

void
qp_lock_unset_alloced(qp_lock_t lock)
{ lock ? lock->is_alloced = false : 1;}

void
qp_lock_unset_shared(qp_lock_t lock)
{ lock ? lock->is_shared = false : 1;}

void
qp_lock_set_spin(qp_lock_t lock)
{ lock ? lock->is_spin = true : 1;}

void
qp_lock_unset_spin(qp_lock_t lock)
{ lock ? lock->is_spin = false : 1;}


/**
 *   create lock.
 */
qp_lock_t
qp_lock_create(qp_lock_t lock, bool shared, bool spin)
{
    if (NULL == lock) {
        lock = (qp_lock_t)qp_alloc(sizeof(struct qp_ipc_lock_s));
        
        if (NULL == lock) {
            return NULL;
        }

        memset(lock, 0, sizeof(struct qp_ipc_lock_s));
        qp_lock_set_alloced(lock);

    } else {
        memset(lock, 0, sizeof(struct qp_ipc_lock_s));
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
        return NULL;
    }
    
    if (QP_SUCCESS != pthread_mutexattr_settype(&attr, \
        PTHREAD_MUTEX_ERRORCHECK))
    {
        qp_lock_is_alloced(lock) ? qp_free(lock) : 1;
        return NULL;
    }
    
#ifdef _POSIX_THREAD_PROCESS_SHARED
    if (shared) {
        
        if (QP_SUCCESS != pthread_mutexattr_setpshared(&attr, \
            PTHREAD_PROCESS_SHARED))
        {
            qp_lock_is_alloced(lock) ? qp_free(lock) : 1;
            return NULL;
        }
        
        qp_lock_set_shared(lock);
    }
#endif
    
    if (QP_SUCCESS != pthread_mutex_init(&lock->lock.mutex, &attr)) {
        qp_lock_is_alloced(lock) ? qp_free(lock) : 1;
        return NULL;
    }
    
    pthread_mutexattr_destroy(&attr);
    qp_lock_set_inited(lock);
    return lock;
}

qp_lock_t
qp_lock_init(qp_lock_t lock, bool shared, bool spin)
{
    return qp_lock_create(lock, shared, spin);
}

qp_int_t
qp_lock_destroy(qp_lock_t lock)
{
    if (qp_lock_is_inited(lock)) {
        /* incase the lock still locked */
        if (EBUSY == qp_lock_trylock(lock)) {
            return QP_ERROR;
        }
        
        qp_lock_unlock(lock);
        
        if (!qp_lock_is_spin(lock)) {
            if (QP_SUCCESS != pthread_mutex_destroy(&lock->lock.mutex)) {
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
qp_lock_lock(qp_lock_t lock)
{
    if (!lock) {
        return QP_ERROR;
    }
    
    if (0 != lock->hold_counter) {
        if (lock->hold_thread == pthread_self() && lock->hold_process == getpid()) {
            qp_atom_fetch_add(&lock->hold_counter, 1);
            QP_LOGOUT_LOG("current counter %u [%lu][%lu].", lock->hold_counter, 
                lock->hold_thread, getpid());
            return QP_SUCCESS;
        }
    }
    
    if (qp_lock_is_spin(lock)) {
        qp_uint_t  n = 1024, i, j;
        
        for ( ;; ) {
            
            if (qp_atom_cmp_set(&lock->lock.spin,0,1)) {
                lock->hold_thread = pthread_self();
                lock->hold_process = getpid();
                qp_atom_fetch_add(&lock->hold_counter, 1);
                QP_LOGOUT_LOG("current counter %u [%lu][%lu].", lock->hold_counter, 
                    lock->hold_thread, getpid());
                return QP_SUCCESS;
            }
            
            if (qp_cpu_num > 1) {
                
                for (i = 1; i < n; i <<= 1) {
                    
                    for (j = 0; j < i; j++) {
                        qp_cpu_pause();
                    }
                    
                    if (qp_atom_cmp_set(&lock->lock.spin, 0, 1)) {
                        lock->hold_thread = pthread_self();
                        lock->hold_process = getpid();
                        qp_atom_fetch_add(&lock->hold_counter, 1); 
                        QP_LOGOUT_LOG("current counter %u [%lu][%lu].", lock->hold_counter, 
                            lock->hold_thread, getpid());
                        return QP_SUCCESS;
                    }
                }
            }
            
            qp_sched_yield();
        }
    }
    
    if (QP_SUCCESS == pthread_mutex_lock(&lock->lock.mutex)) {
        lock->hold_thread = pthread_self();
        lock->hold_process = getpid();
        qp_atom_fetch_add(&lock->hold_counter, 1);
        QP_LOGOUT_LOG("current counter %u [%lu][%lu].", lock->hold_counter, 
            lock->hold_thread, getpid());
        return QP_SUCCESS;
    } 
    
    return QP_ERROR;
}

qp_int_t
qp_lock_trylock(qp_lock_t lock)
{
    if (!lock) {
        return QP_ERROR;
    }
    
    if (0 != lock->hold_counter) {
        if (lock->hold_thread == pthread_self() && lock->hold_process == getpid()) {
            qp_atom_fetch_add(&lock->hold_counter, 1);
            QP_LOGOUT_LOG("current counter %u [%lu][%lu].", lock->hold_counter, 
                lock->hold_thread, getpid());
            return QP_SUCCESS;
        }
    }
    
    if (qp_lock_is_spin(lock)) {
        if (qp_atom_cmp_set(&lock->lock.spin, 0, 1)) {
            lock->hold_thread = pthread_self();
            lock->hold_process = getpid();
            qp_atom_fetch_add(&lock->hold_counter, 1);
            QP_LOGOUT_LOG("current counter %u [%lu][%lu].", lock->hold_counter, 
                lock->hold_thread, getpid());
            return QP_SUCCESS;
            
        } else {
            errno = EBUSY;
            return QP_ERROR;
        }
    }
    
    if (QP_SUCCESS == pthread_mutex_trylock(&lock->lock.mutex)) {
        lock->hold_thread = pthread_self();
        lock->hold_process = getpid();
        qp_atom_fetch_add(&lock->hold_counter, 1);
        QP_LOGOUT_LOG("current counter %u [%lu][%lu].", lock->hold_counter, 
                lock->hold_thread, getpid());
        return QP_SUCCESS;
    } 
    
    return QP_ERROR;
}


qp_int_t
qp_lock_unlock(qp_lock_t lock)
{
    if (!lock || 1 > lock->hold_counter) {
        return QP_ERROR;
    }
    
    // may be unlocked by another thread
    if (lock->hold_thread == pthread_self() && lock->hold_process == getpid()) {
        qp_atom_fetch_sub(&lock->hold_counter, 1);
        QP_LOGOUT_LOG("current counter %u [%lu][%lu].", lock->hold_counter, 
                lock->hold_thread, getpid());
        
        if (0 != lock->hold_counter) {
            return QP_SUCCESS;
        }
            
    } else {
        return QP_ERROR;
    }
    
    lock->hold_thread = 0;
    lock->hold_process = 0;
    
    if (qp_lock_is_spin(lock)) {
        if (qp_atom_cmp_set(&lock->lock.spin, 1, 0)) {
            return QP_SUCCESS;
        }
        
        return QP_ERROR;
    }
    
    return pthread_mutex_unlock(&lock->lock.mutex);
}


qp_uint_t
qp_lock_counter(qp_lock_t lock) 
{
    if (!lock) {
        return 0;
    }
    
    return lock->hold_counter;
}

// MARK: rwlock

void
qp_rwlock_set_inited(qp_rwlock_t rwlock)
{ rwlock ? rwlock->is_inited = true : 1;}

void
qp_rwlock_set_alloced(qp_rwlock_t rwlock)
{ rwlock ? rwlock->is_alloced = true : 1;}

void
qp_rwlock_set_shared(qp_rwlock_t rwlock)
{ rwlock ? rwlock->is_shared = true : 1;}

void
qp_rwlock_unset_inited(qp_rwlock_t rwlock)
{ rwlock ? rwlock->is_inited = false : 1;}

void
qp_rwlock_unset_alloced(qp_rwlock_t rwlock)
{ rwlock ? rwlock->is_alloced = false : 1;}

void
qp_rwlock_unset_shared(qp_rwlock_t rwlock)
{ rwlock ? rwlock->is_shared = false : 1;}


qp_rwlock_t
qp_rwlock_create(qp_rwlock_t rwlock, bool shared)
{
    pthread_rwlockattr_t    attr;
    
    if (NULL == rwlock) {
        rwlock = (qp_rwlock_t)qp_alloc(sizeof(struct qp_ipc_rwlock_s));
        
        if (NULL == rwlock) {
            return NULL;
        }
        
        memset(rwlock, 0, sizeof(struct qp_ipc_rwlock_s));
        qp_rwlock_set_alloced(rwlock);
        
    } else {
        memset(rwlock, 0, sizeof(struct qp_ipc_rwlock_s));
    }
    
    if (QP_SUCCESS != pthread_rwlockattr_init(&attr)) {
        qp_rwlock_is_alloced(rwlock) ? qp_free(rwlock) : 1;
        return NULL;
    }
    
#ifdef _POSIX_THREAD_PROCESS_SHARED
    if (shared) {
        
        if (QP_SUCCESS != pthread_rwlockattr_setpshared(&attr, \
            PTHREAD_PROCESS_SHARED)) 
        {
            qp_rwlock_is_alloced(rwlock) ? qp_free(rwlock) : 1;
            return NULL;
        }
        
        qp_rwlock_set_shared(rwlock);
    }
#endif
    
    if (QP_SUCCESS != pthread_rwlock_init(&(rwlock->rwlock), &attr)) {
        qp_rwlock_is_alloced(rwlock) ? qp_free(rwlock) : 1;
        return NULL;
    }
    
    pthread_rwlockattr_destroy(&attr);
    qp_rwlock_set_inited(rwlock);
    return rwlock;
}

qp_rwlock_t        
qp_rwlock_init(qp_rwlock_t rwlock, bool shared)
{
    return qp_rwlock_create(rwlock, shared);
}

qp_int_t
qp_rwlock_destroy(qp_rwlock_t rwlock)
{
    if (qp_rwlock_is_inited(rwlock)) {
        /* incase the lock still locked */
        if (EBUSY == qp_rwlock_tryrdlock(rwlock)) {
            return QP_ERROR;
        }
        
        qp_rwlock_unlock(rwlock);
        
        if (EBUSY == qp_rwlock_trywrlock(rwlock)) {
            return QP_ERROR;
        }
        
        qp_rwlock_unlock(rwlock);
        
        if (QP_SUCCESS != pthread_rwlock_destroy(&(rwlock->rwlock))) {
            return QP_ERROR;
        }
        
        qp_rwlock_unset_shared(rwlock);
        qp_rwlock_unset_inited(rwlock);
        
        if (qp_rwlock_is_alloced(rwlock)) {
            qp_free(rwlock);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

qp_int_t
qp_rwlock_rdlock(qp_rwlock_t rwlock)
{
    return rwlock ? pthread_rwlock_rdlock(&rwlock->rwlock) : QP_ERROR;
}

qp_int_t
qp_rwlock_tryrdlock(qp_rwlock_t rwlock)
{
    return rwlock ? pthread_rwlock_tryrdlock(&rwlock->rwlock) : QP_ERROR;
}

qp_int_t
qp_rwlock_wrlock(qp_rwlock_t rwlock)
{
    return rwlock ? pthread_rwlock_wrlock(&rwlock->rwlock) : QP_ERROR;
}

qp_int_t
qp_rwlock_trywrlock(qp_rwlock_t rwlock)
{
    return rwlock ? pthread_rwlock_trywrlock(&rwlock->rwlock) : QP_ERROR;
}

qp_int_t
qp_rwlock_unlock(qp_rwlock_t rwlock)
{
    return rwlock ? pthread_rwlock_unlock(&rwlock->rwlock) : QP_ERROR;
}


// MARK: cond
void
qp_cond_set_inited(qp_cond_t cond)
{ cond ? cond->is_inited = true : 1;}

void
qp_cond_set_alloced(qp_cond_t cond)
{ cond ? cond->is_alloced = true : 1;}

void
qp_cond_set_shared(qp_cond_t cond)
{ cond ? cond->is_shared = true : 1;}

void
qp_cond_unset_inited(qp_cond_t cond)
{ cond ? cond->is_inited = false : 1;}

void
qp_cond_unset_alloced(qp_cond_t cond)
{ cond ? cond->is_alloced = false : 1;}

void
qp_cond_unset_shared(qp_cond_t cond)
{ cond ? cond->is_shared = false : 1;}


qp_cond_t
qp_cond_create(qp_cond_t cond, bool shared)
{
    pthread_condattr_t    attr;
    
    if (NULL == cond) {
        cond = (qp_cond_t)qp_alloc(sizeof(struct qp_ipc_cond_s));
        
        if (NULL == cond) {
            return NULL;
        }
        
        memset(cond, 0, sizeof(struct qp_ipc_cond_s));
        qp_cond_set_alloced(cond);
        
    } else {
        memset(cond, 0, sizeof(struct qp_ipc_cond_s));
    }
    
    if (QP_SUCCESS != pthread_condattr_init(&attr)) {
        qp_cond_is_alloced(cond) ? qp_free(cond) : 1;
        return NULL;
    }
    
    if (NULL == qp_lock_init(&cond->cond_lock, shared, false)) {
        qp_cond_is_alloced(cond) ? qp_free(cond) : 1;
        return NULL;
    }
    
    
#ifdef _POSIX_THREAD_PROCESS_SHARED
    if (shared) {
        
        if (QP_SUCCESS != pthread_condattr_setpshared(&attr,
            PTHREAD_PROCESS_SHARED))
        {
            pthread_condattr_destroy(&attr);
            qp_lock_destroy(&cond->cond_lock);
            qp_cond_is_alloced(cond) ? qp_free(cond) : 1;
            return NULL;
        }
        
        qp_cond_set_shared(cond);
    }
#endif
    
    if (QP_SUCCESS != pthread_cond_init(&(cond->cond), &attr)) {
        pthread_condattr_destroy(&attr);
        qp_lock_destroy(&cond->cond_lock);
        qp_cond_is_alloced(cond) ? qp_free(cond) : 1;
        return NULL;
    }
    
    qp_cond_set_inited(cond);
    return cond;
}

qp_cond_t
qp_cond_init(qp_cond_t cond, bool shared)
{
    return qp_cond_create(cond, shared);
}

qp_int_t
qp_cond_destroy(qp_cond_t cond)
{
    if (qp_cond_is_inited(cond)) {
        
        if (EBUSY == pthread_cond_destroy(&cond->cond)) {
            return QP_ERROR;
        }
        
        qp_lock_destroy(&cond->cond_lock);
        qp_cond_unset_shared(cond);
        qp_cond_unset_inited(cond);
        
        if (qp_cond_is_alloced(cond)) {
            qp_free(cond);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

qp_int_t
qp_cond_signal(qp_cond_t cond, void (*signal_to)(void*), void* arg)
{
    if (!cond) {
        return QP_ERROR;
    }
    
    bool stat = false;
    qp_lock_lock(&cond->cond_lock);
    stat = (0 == cond->nready);
    cond->nready++;
    
    if (signal_to) {
        signal_to(arg);
    }
    
    qp_lock_unlock(&cond->cond_lock);
    
    if (stat) { 
        return pthread_cond_signal(&cond->cond);
    }
    
    return QP_SUCCESS;
}

qp_int_t
qp_cond_wait(qp_cond_t cond, void (*wait_for)(void*), void* arg)
{
    if (!cond) {
        return QP_ERROR;
    }
    
    qp_lock_lock(&(cond->cond_lock));
    
    while (0 == cond->nready) {
        
        if (QP_SUCCESS != pthread_cond_wait(&cond->cond, \
            &cond->cond_lock.lock.mutex)) 
        {
            qp_lock_unlock(&cond->cond_lock);
            return QP_ERROR;
        }
    }
    
    cond->nready--;
    
    if (wait_for) {
        wait_for(arg);
    }
    
    return qp_lock_unlock(&cond->cond_lock);
}

// MARK: sem

void
qp_sem_set_inited(qp_sem_t sem)
{ sem ? sem->is_inited = true : 1;}

void
qp_sem_set_alloced(qp_sem_t sem)
{ sem ? sem->is_alloced = true : 1;}

void
qp_sem_set_shared(qp_sem_t sem)
{ sem ? sem->is_shared = true : 1;}

void
qp_sem_unset_inited(qp_sem_t sem)
{ sem ? sem->is_inited = false : 1;}

void
qp_sem_unset_alloced(qp_sem_t sem)
{ sem ? sem->is_alloced = false : 1;}

void
qp_sem_unset_shared(qp_sem_t sem)
{ sem ? sem->is_shared = false : 1;}


qp_sem_t
qp_sem_create(qp_sem_t sem, bool shared)
{
    if (NULL == sem) {
        sem = (qp_sem_t)qp_alloc(sizeof(struct qp_ipc_sem_s));
        
        if (NULL == sem) {
            return NULL;
        }
        
        memset(sem, 0, sizeof(struct qp_ipc_sem_s));
        qp_sem_set_alloced(sem);
        
    } else {
        memset(sem, 0, sizeof(struct qp_ipc_sem_s));
    }
    
    shared ? qp_sem_set_shared(sem) : 1;
    
    if (QP_SUCCESS != sem_init(&sem->sem, (qp_int_t)(sem->is_shared), 0)) {
        qp_sem_is_alloced(sem) ? qp_free(sem) : 1;
        return NULL;
    }
    
    qp_sem_set_inited(sem);
    return sem;
}

qp_sem_t
qp_sem_init(qp_sem_t sem, bool shared)
{
    return qp_sem_create(sem, shared);
}

qp_int_t
qp_sem_destroy(qp_sem_t sem)
{
    if (qp_sem_is_inited(sem)) {
        /* incase it still wait for signal */
        qp_sem_post(sem);
        
        if (QP_SUCCESS != sem_destroy(&sem->sem)) {
            return QP_ERROR;
        }
        
        qp_sem_unset_shared(sem);
        qp_sem_unset_inited(sem);
        
        if (qp_sem_is_alloced(sem)) {
            qp_free(sem);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

qp_int_t
qp_sem_post(qp_sem_t sem)
{
    return sem ? sem_post(&sem->sem) : QP_ERROR;
}

qp_int_t
qp_sem_trywait(qp_sem_t sem)
{
    return sem ? sem_trywait(&sem->sem) : QP_ERROR;
}

qp_int_t
qp_sem_wait(qp_sem_t sem)
{
    return sem ? sem_wait(&sem->sem) : QP_ERROR;
}

// MARK: shm

void
qp_shm_set_inited(qp_shm_t shm)
{ shm ? shm->is_inited = true : 1;}

void
qp_shm_set_alloced(qp_shm_t shm)
{ shm ? shm->is_alloced = true : 1;}

void
qp_shm_unset_inited(qp_shm_t shm)
{ shm ? shm->is_inited = false : 1;}

void
qp_shm_unset_alloced(qp_shm_t shm)
{ shm ? shm->is_alloced = false : 1;}


qp_shm_t
qp_shm_create(qp_shm_t shm, size_t size, qp_int_t fd)
{
    if (NULL == shm) {
        shm = (qp_shm_t)qp_alloc(sizeof(struct qp_ipc_shm_s));
        
        if (NULL == shm) {
            return NULL;
        }
        
        memset(shm, 0, sizeof(struct qp_ipc_shm_s));
        qp_shm_set_alloced(shm);
        
    } else {
        memset(shm, 0, sizeof(struct qp_ipc_shm_s));
    }
    
    shm->fd = fd;
    shm->size = size;
    
    if (QP_IPC_INVALID < shm->fd) {
        shm->addr = (qp_uchar_t*) mmap(NULL, shm->size, \
            PROT_READ|PROT_WRITE, MAP_SHARED, shm->fd, 0);
        
    } else {
#ifndef MAP_ANON
        shm->fd = open("/dev/zero", O_RDWR);
        
        if (QP_FD_INVALID == shm->fd) {
            qp_shm_is_alloced(shm) ? qp_free(shm) : 1;
            return NULL;
        }
#endif
        
        shm->addr = (qp_uchar_t*) mmap(NULL, shm->size, \
            PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, QP_IPC_INVALID, 0);
        
#ifndef MAP_ANON
        close(shm->fd);
        shm->fd = QP_FD_INVALID;
#endif
    }
    
    if (MAP_FAILED == shm->addr) {
        qp_shm_is_alloced(shm) ? qp_free(shm) : 1;
        return NULL;
    }
    
    qp_shm_set_inited(shm);
    return shm;
}

qp_shm_t
qp_shm_init(qp_shm_t shm, size_t size, const qp_char_t* name)
{
    qp_int_t fd = -1;
    
    if (name) {
        fd = shm_open(name, O_RDWR | O_CREAT, \
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH); 
        
        if (fd < 0) {
            return NULL;
        }
    }
    
    shm = qp_shm_create(shm, size, fd);
    
    if (NULL == shm) {
        
        (fd > QP_IPC_INVALID) ? shm_unlink(name) : 1;
        return NULL;
    }
    
    shm->name = name;
    return shm;
}

qp_int_t
qp_shm_destroy(qp_shm_t shm)
{
    if (qp_shm_is_inited(shm)) {
        
        if (QP_ERROR == munmap(shm->addr, shm->size)) {
            return QP_ERROR;
        }
        
        if (shm->name) {
            close(shm->fd);
            shm_unlink(shm->name);
            shm->name = NULL;
        }
        
        qp_shm_unset_inited(shm);
        
        if (qp_shm_is_alloced(shm)) {
            qp_free(shm);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

qp_uchar_t* 
qp_shm_start(qp_shm_t shm)
{
    if (qp_shm_is_inited(shm)) {
        return shm->addr;
    }
    
    return NULL;
}
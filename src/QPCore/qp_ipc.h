
/**
 * Copyright (C) 2sui.
 *
 * Basic IPC and control operations.We use POSIX ipc instead of SYSTEM V.
*/

#ifndef QP_IPC_H
#define QP_IPC_H


#include "qp_o_typedef.h"


#ifdef __cplusplus
extern "C" {
#endif
    
#define QP_IPC_INVALID    QP_ERROR
    
typedef struct qp_ipc_lock_s*      qp_lock_t;
typedef struct qp_ipc_rwlock_s*    qp_rwlock_t;
typedef struct qp_ipc_cond_s*      qp_cond_t;
typedef struct qp_ipc_sem_s*       qp_sem_t;
typedef struct qp_ipc_shm_s*       qp_shm_t;

/* lock */

/**
 * Init a lock. If shared is true , the lock is shared between processes.
 */
qp_lock_t
qp_lock_init(qp_lock_t lock, bool shared, bool spin);

/**
 * Destroy a lock. If it still locked, it will return QP_ERROR.
 */
qp_int_t
qp_lock_destroy(qp_lock_t lock);

/**
 * Lock the lock. 
 */
qp_int_t
qp_lock_lock(qp_lock_t lock);

/**
 * Try to lock. If the lock has already locked, return EBUSY.
 */
qp_int_t
qp_lock_trylock(qp_lock_t lock);

/**
 * Unlock the lock.
 */
qp_int_t
qp_lock_unlock(qp_lock_t lock);

/**
 * 
 * @param lock
 * @return 
 */
qp_uint_t
qp_lock_counter(qp_lock_t lock);


/* read/write lock */

/**
 * Init a rwlock. If shared is true the lock is shared between processes.
 */
qp_rwlock_t
qp_rwlock_init(qp_rwlock_t rwlock, bool shared);

/**
 * Destroy a rwlock.
 */
qp_int_t
qp_rwlock_destroy(qp_rwlock_t rwlock);

/**
 * Read lock. You should always check the return value in shared mode.
 */
qp_int_t
qp_rwlock_rdlock(qp_rwlock_t rwlock);

/**
 * Read lock. You should always check the return value in shared mode.
 */
qp_int_t
qp_rwlock_tryrdlock(qp_rwlock_t rwlock);

/**
 * Write lock.
 */
qp_int_t
qp_rwlock_wrlock(qp_rwlock_t rwlock);

/**
 * Write lock.
 */
qp_int_t
qp_rwlock_trywrlock(qp_rwlock_t rwlock);

/**
 * Unlock.
 */
qp_int_t
qp_rwlock_unlock(qp_rwlock_t rwlock);

/* cond */

/**
 * Init a cond. If shared is true, the cond is shared between processes.
 */
qp_cond_t
qp_cond_init(qp_cond_t cond, bool shared);

/**
 * Destroy a cond.
 */
qp_int_t
qp_cond_destroy(qp_cond_t cond);

/**
 * Signal waiting cond. If signal_to is not NULL, it will call it atomic.
 */
qp_int_t
qp_cond_signal(qp_cond_t cond, void (*signal_to)(void*), void* arg);

/**
 * Wait for cond. If wait_for is not NULL, it will call it atomic.
 */
qp_int_t
qp_cond_wait(qp_cond_t cond, void (*wait_for)(void*), void* arg);


/* sem */

/**
 * Init sem.
 */
qp_sem_t
qp_sem_init(qp_sem_t sem, bool shared);

/**
 * Destroy sem.
 */
qp_int_t
qp_sem_destroy(qp_sem_t sem);

/**
 * Post sem.
 */
qp_int_t
qp_sem_post(qp_sem_t sem);

/**
 * Wait sem.
 */
qp_int_t
qp_sem_trywait(qp_sem_t sem);

/**
 * Wait sem.
 */
qp_int_t
qp_sem_wait(qp_sem_t sem);

/* share memory */

/**
 * Init share memory (with file [name]).
 */
qp_shm_t
qp_shm_init(qp_shm_t shm, size_t size, const qp_char_t* name);

/**
 * Destroy shared memory,
 */
qp_int_t
qp_shm_destroy(qp_shm_t shm);

/**
 * Get the shared memory address.
 */
qp_uchar_t*
qp_shm_start(qp_shm_t shm);



#ifdef __cplusplus
}
#endif

#endif /* QP_IPC_H */


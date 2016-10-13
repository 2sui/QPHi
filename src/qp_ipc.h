/*
 * The MIT License
 *
 * Copyright © 2016 2sui.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef QP_IPC_H
#define QP_IPC_H

#ifdef __cplusplus
extern "C" {
#endif


#include "core/qp_defines.h"

    
# define QP_IPC_INVALID    QP_ERROR
    
    
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


qp_int_t
qp_lock_destroy(qp_lock_t lock);


qp_int_t
qp_lock_lock(qp_lock_t lock);


qp_int_t
qp_lock_trylock(qp_lock_t lock);


qp_int_t
qp_lock_unlock(qp_lock_t lock);


/**
 * Get the count of lock operation.
 */
qp_uint_t
qp_lock_counter(qp_lock_t lock);


/* read/write lock */

/**
 * Init a rwlock. If shared is true the lock is shared between processes.
 */
qp_rwlock_t
qp_rwlock_init(qp_rwlock_t rwlock, bool shared);


qp_int_t
qp_rwlock_destroy(qp_rwlock_t rwlock);


qp_int_t
qp_rwlock_rdlock(qp_rwlock_t rwlock);


qp_int_t
qp_rwlock_tryrdlock(qp_rwlock_t rwlock);


qp_int_t
qp_rwlock_wrlock(qp_rwlock_t rwlock);


qp_int_t
qp_rwlock_trywrlock(qp_rwlock_t rwlock);


qp_int_t
qp_rwlock_unlock(qp_rwlock_t rwlock);

/* cond */

/**
 * Init a cond. If shared is true, the cond is shared between processes.
 */
qp_cond_t
qp_cond_init(qp_cond_t cond, bool shared);


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

qp_sem_t
qp_sem_init(qp_sem_t sem, bool shared);


qp_int_t
qp_sem_destroy(qp_sem_t sem);


qp_int_t
qp_sem_post(qp_sem_t sem);


qp_int_t
qp_sem_trywait(qp_sem_t sem);


qp_int_t
qp_sem_wait(qp_sem_t sem);

/* share memory */

/**
 * Init share memory (with file [name]).
 */
qp_shm_t
qp_shm_init(qp_shm_t shm, size_t size, const qp_char_t* name);


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

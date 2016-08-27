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


#ifndef QP_IPC_CORE_H
#define QP_IPC_CORE_H

#ifdef __cplusplus
extern "C" {
#endif


#include "../qp_ipc.h"
#include "qp_atomic.h"
#include "qp_memory_core.h"

    
struct qp_ipc_lock_s {
    union {
        pthread_mutex_t  mutex;
        qp_atom_t        spin;
    }                    lock;
    pthread_t            hold_thread;
    pid_t                hold_process;
    qp_atom_t            hold_counter;
    bool                 is_inited;
    bool                 is_alloced;
    bool                 is_shared;
    bool                 is_spin;
};


struct qp_ipc_rwlock_s {
    pthread_rwlock_t     rwlock;
    bool                 is_inited;
    bool                 is_alloced;
    bool                 is_shared;
};


struct qp_ipc_cond_s {
    pthread_cond_t       cond;
    struct qp_ipc_lock_s cond_lock;
    qp_uint32_t          nready;
    bool                 is_inited;
    bool                 is_alloced;
    bool                 is_shared;
};


struct qp_ipc_sem_s {
    sem_t                sem;
    bool                 is_inited;
    bool                 is_alloced;
    bool                 is_shared;
};


struct qp_ipc_shm_s {
    qp_uchar_t*          addr;
    size_t               size;
    const qp_char_t*     name;
    qp_int_t             fd;
    bool                 is_inited;
    bool                 is_alloced;
    /* share mem always shared in processes */
};


static inline bool
qp_lock_is_inited(qp_lock_t lock)
{ 
    return lock->is_inited; 
}


static inline bool
qp_rwlock_is_inited(qp_rwlock_t rwlock)
{ 
    return rwlock->is_inited; 
}


static inline bool
qp_cond_is_inited(qp_cond_t cond)
{ 
    return cond->is_inited; 
}


static inline bool
qp_sem_is_inited(qp_sem_t sem)
{ 
    return sem->is_inited; 
}


static inline bool
qp_shm_is_inited(qp_shm_t shm)
{ 
    return shm->is_inited; 
}


static inline bool
qp_lock_is_alloced(qp_lock_t lock)
{ 
    return lock->is_alloced; 
}


static inline bool
qp_rwlock_is_alloced(qp_rwlock_t rwlock)
{ 
    return rwlock ? rwlock->is_alloced : false; 
}


static inline bool
qp_cond_is_alloced(qp_cond_t cond)
{ 
    return cond->is_alloced; 
}


static inline bool
qp_sem_is_alloced(qp_sem_t sem)
{ 
    return sem->is_alloced; 
}


static inline bool
qp_shm_is_alloced(qp_shm_t shm)
{ 
    return shm->is_alloced; 
}


static inline bool
qp_lock_is_shared(qp_lock_t lock)
{ 
    return lock->is_shared; 
}


static inline bool
qp_rwlock_is_shared(qp_rwlock_t rwlock)
{ 
    return rwlock->is_shared; 
}


static inline bool
qp_cond_is_shared(qp_cond_t cond)
{ 
    return cond->is_shared; 
}


static inline bool
qp_sem_is_shared(qp_sem_t sem)
{ 
    return sem->is_shared; 
}


static inline bool
qp_lock_is_spin(qp_lock_t lock)
{ 
    return lock->is_spin; 
}

#ifdef __cplusplus
}
#endif

#endif /* QP_IPC_CORE_H */

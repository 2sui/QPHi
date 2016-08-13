
/**
 * Copyright (C) 2sui.
 *
 * Basic IPC and control operations.We use POSIX ipc instead of SYSTEM V.
*/


#ifndef QP_O_IPC_H
#define QP_O_IPC_H


#include "qp_ipc.h"
#include "qp_o_atomic.h"
#include "qp_o_memory.h"


#ifdef __cplusplus
extern "C" {
#endif
    
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
    return lock ? lock->is_inited : false; 
}

static inline bool
qp_rwlock_is_inited(qp_rwlock_t rwlock)
{ 
    return rwlock ? rwlock->is_inited : false; 
}

static inline bool
qp_cond_is_inited(qp_cond_t cond)
{ 
    return cond ? cond->is_inited : false; 
}

static inline bool
qp_sem_is_inited(qp_sem_t sem)
{ 
    return sem ? sem->is_inited : false; 
}

static inline bool
qp_shm_is_inited(qp_shm_t shm)
{ 
    return shm ? shm->is_inited : false; 
}

static inline bool
qp_lock_is_alloced(qp_lock_t lock)
{ 
    return lock ? lock->is_alloced : false; 
}

static inline bool
qp_rwlock_is_alloced(qp_rwlock_t rwlock)
{ 
    return rwlock ? rwlock->is_alloced : false; 
}

static inline bool
qp_cond_is_alloced(qp_cond_t cond)
{ 
    return cond ? cond->is_alloced : false; 
}

static inline bool
qp_sem_is_alloced(qp_sem_t sem)
{ 
    return sem ? sem->is_alloced : false; 
}

static inline bool
qp_shm_is_alloced(qp_shm_t shm)
{ 
    return shm ? shm->is_alloced : false; 
}

static inline bool
qp_lock_is_shared(qp_lock_t lock)
{ 
    return lock ? lock->is_shared : false; 
}

static inline bool
qp_rwlock_is_shared(qp_rwlock_t rwlock)
{ 
    return rwlock ? rwlock->is_shared : false; 
}

static inline bool
qp_cond_is_shared(qp_cond_t cond)
{ 
    return cond ? cond->is_shared : false; 
}

static inline bool
qp_sem_is_shared(qp_sem_t sem)
{ 
    return sem ? sem->is_shared : false; 
}

static inline bool
qp_lock_is_spin(qp_lock_t lock)
{ 
    return lock ? lock->is_spin : false; 
}

static inline void
qp_lock_set_inited(qp_lock_t lock)
{ 
    lock ? lock->is_inited = true : 1;
}

static inline void
qp_lock_set_alloced(qp_lock_t lock)
{ 
    lock ? lock->is_alloced = true : 1;
}

static inline void
qp_lock_set_shared(qp_lock_t lock)
{ 
    lock ? lock->is_shared = true : 1;
}

static inline void
qp_lock_unset_inited(qp_lock_t lock)
{ 
    lock ? lock->is_inited = false : 1;
}

static inline void
qp_lock_unset_alloced(qp_lock_t lock)
{ 
    lock ? lock->is_alloced = false : 1;
}

static inline void
qp_lock_unset_shared(qp_lock_t lock)
{ 
    lock ? lock->is_shared = false : 1;
}

static inline void
qp_lock_set_spin(qp_lock_t lock)
{ 
    lock ? lock->is_spin = true : 1;
}

static inline void
qp_lock_unset_spin(qp_lock_t lock)
{ 
    lock ? lock->is_spin = false : 1;
}

static inline void
qp_rwlock_set_inited(qp_rwlock_t rwlock)
{ 
    rwlock ? rwlock->is_inited = true : 1;
}

static inline void
qp_rwlock_set_alloced(qp_rwlock_t rwlock)
{ 
    rwlock ? rwlock->is_alloced = true : 1;
}

static inline void
qp_rwlock_set_shared(qp_rwlock_t rwlock)
{ 
    rwlock ? rwlock->is_shared = true : 1;
}

static inline void
qp_rwlock_unset_inited(qp_rwlock_t rwlock)
{ 
    rwlock ? rwlock->is_inited = false : 1;
}

static inline void
qp_rwlock_unset_alloced(qp_rwlock_t rwlock)
{ 
    rwlock ? rwlock->is_alloced = false : 1;
}

static inline void
qp_rwlock_unset_shared(qp_rwlock_t rwlock)
{ 
    rwlock ? rwlock->is_shared = false : 1;
}

static inline void
qp_cond_set_inited(qp_cond_t cond)
{ 
    cond ? cond->is_inited = true : 1;
}

static inline void
qp_cond_set_alloced(qp_cond_t cond)
{ 
    cond ? cond->is_alloced = true : 1;
}

static inline void
qp_cond_set_shared(qp_cond_t cond)
{ 
    cond ? cond->is_shared = true : 1;
}

static inline void
qp_cond_unset_inited(qp_cond_t cond)
{ 
    cond ? cond->is_inited = false : 1;
}

static inline void
qp_cond_unset_alloced(qp_cond_t cond)
{ 
    cond ? cond->is_alloced = false : 1;
}

static inline void
qp_cond_unset_shared(qp_cond_t cond)
{ 
    cond ? cond->is_shared = false : 1;
}

static inline void
qp_sem_set_inited(qp_sem_t sem)
{ 
    sem ? sem->is_inited = true : 1;
}

static inline void
qp_sem_set_alloced(qp_sem_t sem)
{ 
    sem ? sem->is_alloced = true : 1;
}

static inline void
qp_sem_set_shared(qp_sem_t sem)
{ 
    sem ? sem->is_shared = true : 1;
}

static inline void
qp_sem_unset_inited(qp_sem_t sem)
{ 
    sem ? sem->is_inited = false : 1;
}

static inline void
qp_sem_unset_alloced(qp_sem_t sem)
{ 
    sem ? sem->is_alloced = false : 1;
}

static inline void
qp_sem_unset_shared(qp_sem_t sem)
{ 
    sem ? sem->is_shared = false : 1;
}

static inline void
qp_shm_set_inited(qp_shm_t shm)
{ 
    shm ? shm->is_inited = true : 1;
}

static inline void
qp_shm_set_alloced(qp_shm_t shm)
{ 
    shm ? shm->is_alloced = true : 1;
}

static inline void
qp_shm_unset_inited(qp_shm_t shm)
{ 
    shm ? shm->is_inited = false : 1;
}

static inline void
qp_shm_unset_alloced(qp_shm_t shm)
{ 
    shm ? shm->is_alloced = false : 1;
}

#ifdef __cplusplus
}
#endif

#endif 

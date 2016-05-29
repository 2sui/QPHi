
/**
  * Copyright (C) 2sui.
  */


#include "qp_o_ipc.h"
#include "qp_o_memory.h"


struct qp_ipc_lock_s {
    union {
        pthread_mutex_t  mutex;
        qp_atom_t        spin;
    }                    lock;
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
    qp_lock_t            cond_lock;
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


inline bool
qp_lock_is_inited(qp_lock_t lock)
{ return lock ? lock->is_inited : false; }

inline bool
qp_rwlock_is_inited(qp_rwlock_t rwlock)
{ return rwlock ? rwlock->is_inited : false; }

inline bool
qp_cond_is_inited(qp_cond_t cond)
{ return cond ? cond->is_inited : false; }

inline bool
qp_sem_is_inited(qp_sem_t sem)
{ return sem ? sem->is_inited : false; }

inline bool
qp_shm_is_inited(qp_shm_t shm)
{ return shm ? shm->is_inited : false; }

inline bool
qp_lock_is_alloced(qp_lock_t lock)
{ return lock ? lock->is_alloced : false; }

inline bool
qp_rwlock_is_alloced(qp_rwlock_t rwlock)
{ return rwlock ? rwlock->is_alloced : false; }

inline bool
qp_cond_is_alloced(qp_cond_t cond)
{ return cond ? cond->is_alloced : false; }

inline bool
qp_sem_is_alloced(qp_sem_t sem)
{ return sem ? sem->is_alloced : false; }

inline bool
qp_shm_is_alloced(qp_shm_t shm)
{ return shm ? shm->is_alloced : false; }

inline bool
qp_lock_is_shared(qp_lock_t lock)
{ return lock ? lock->is_shared : false; }

inline bool
qp_rwlock_is_shared(qp_rwlock_t rwlock)
{ return rwlock ? rwlock->is_shared : false; }

inline bool
qp_cond_is_shared(qp_cond_t cond)
{ return cond ? cond->is_shared : false; }

inline bool
qp_sem_is_shared(qp_sem_t sem)
{ return sem ? sem->is_shared : false; }

inline bool
qp_lock_is_spin(qp_lock_t lock)
{ return lock ? lock->is_spin : false; }


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

#ifdef __cplusplus
}
#endif

#endif 

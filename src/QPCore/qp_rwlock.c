
/**
  * Copyright (C) 2sui.
  */


#include "qp_o_ipc.h"


inline void
qp_rwlock_set_inited(qp_rwlock_t* rwlock)
{ rwlock ? rwlock->is_inited = true : 1;}

inline void
qp_rwlock_set_alloced(qp_rwlock_t* rwlock)
{ rwlock ? rwlock->is_alloced = true : 1;}

inline void
qp_rwlock_set_shared(qp_rwlock_t* rwlock)
{ rwlock ? rwlock->is_shared = true : 1;}

inline void
qp_rwlock_unset_inited(qp_rwlock_t* rwlock)
{ rwlock ? rwlock->is_inited = false : 1;}

inline void
qp_rwlock_unset_alloced(qp_rwlock_t* rwlock)
{ rwlock ? rwlock->is_alloced = false : 1;}

inline void
qp_rwlock_unset_shared(qp_rwlock_t* rwlock)
{ rwlock ? rwlock->is_shared = false : 1;}


qp_rwlock_t*
qp_rwlock_create(qp_rwlock_t* rwlock, bool shared)
{
    pthread_rwlockattr_t    attr;
    
    if (NULL == rwlock) {
        rwlock = (qp_rwlock_t*)qp_alloc(sizeof(qp_rwlock_t));
        
        if (NULL == rwlock) {
            return NULL;
        }
        
        memset(rwlock, 0, sizeof(qp_rwlock_t));
        qp_rwlock_set_alloced(rwlock);
        
    } else {
        memset(rwlock, 0, sizeof(qp_rwlock_t));
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

qp_rwlock_t*        
qp_rwlock_init(qp_rwlock_t* rwlock, bool shared)
{
    return qp_rwlock_create(rwlock, shared);
}

qp_int_t
qp_rwlock_destroy(qp_rwlock_t* rwlock)
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
qp_rwlock_rdlock(qp_rwlock_t* rwlock)
{
    return rwlock ? pthread_rwlock_rdlock(&rwlock->rwlock) : QP_ERROR;
}

qp_int_t
qp_rwlock_tryrdlock(qp_rwlock_t* rwlock)
{
    return rwlock ? pthread_rwlock_tryrdlock(&rwlock->rwlock) : QP_ERROR;
}

qp_int_t
qp_rwlock_wrlock(qp_rwlock_t* rwlock)
{
    return rwlock ? pthread_rwlock_wrlock(&rwlock->rwlock) : QP_ERROR;
}

qp_int_t
qp_rwlock_trywrlock(qp_rwlock_t* rwlock)
{
    return rwlock ? pthread_rwlock_trywrlock(&rwlock->rwlock) : QP_ERROR;
}

qp_int_t
qp_rwlock_unlock(qp_rwlock_t* rwlock)
{
    return rwlock ? pthread_rwlock_unlock(&rwlock->rwlock) : QP_ERROR;
}



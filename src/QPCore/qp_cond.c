
/**
  * Copyright (C) 2sui.
  */


#include "qp_o_ipc.h"


inline void
qp_cond_set_inited(qp_cond_t* cond)
{ cond ? cond->is_inited = true : 1;}

inline void
qp_cond_set_alloced(qp_cond_t* cond)
{ cond ? cond->is_alloced = true : 1;}

inline void
qp_cond_set_shared(qp_cond_t* cond)
{ cond ? cond->is_shared = true : 1;}

inline void
qp_cond_unset_inited(qp_cond_t* cond)
{ cond ? cond->is_inited = false : 1;}

inline void
qp_cond_unset_alloced(qp_cond_t* cond)
{ cond ? cond->is_alloced = false : 1;}

inline void
qp_cond_unset_shared(qp_cond_t* cond)
{ cond ? cond->is_shared = false : 1;}


qp_cond_t*
qp_cond_create(qp_cond_t* cond, bool shared)
{
    pthread_condattr_t    attr;
    
    if (NULL == cond) {
        cond = (qp_cond_t*)qp_alloc(sizeof(qp_cond_t));
        
        if (NULL == cond) {
            return NULL;
        }
        
        memset(cond, 0, sizeof(qp_cond_t));
        qp_cond_set_alloced(cond);
        
    } else {
        memset(cond, 0, sizeof(qp_cond_t));
    }
    
    if (QP_SUCCESS != pthread_condattr_init(&attr)) {
        qp_cond_is_alloced(cond) ? qp_free(cond) : 1;
        return NULL;
    }
    
    if (NULL == qp_lock_init(&(cond->cond_lock), shared, false)) {
        qp_cond_is_alloced(cond) ? qp_free(cond) : 1;
        return NULL;
    }
    
    
#ifdef _POSIX_THREAD_PROCESS_SHARED
    if (shared) {
        
        if (QP_SUCCESS != pthread_condattr_setpshared(&attr,
            PTHREAD_PROCESS_SHARED))
        {
            pthread_condattr_destroy(&attr);
            qp_lock_destroy(&(cond->cond_lock));
            qp_cond_is_alloced(cond) ? qp_free(cond) : 1;
            return NULL;
        }
        
        qp_cond_set_shared(cond);
    }
#endif
    
    if (QP_SUCCESS != pthread_cond_init(&(cond->cond), &attr)) {
        pthread_condattr_destroy(&attr);
        qp_lock_destroy(&(cond->cond_lock));
        qp_cond_is_alloced(cond) ? qp_free(cond) : 1;
        return NULL;
    }
    
    qp_cond_set_inited(cond);
    return cond;
}

qp_cond_t*
qp_cond_init(qp_cond_t* cond, bool shared)
{
    return qp_cond_create(cond, shared);
}

qp_int_t
qp_cond_destroy(qp_cond_t* cond)
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
qp_cond_signal(qp_cond_t* cond, void (*signal_to)(void*), void* arg)
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
qp_cond_wait(qp_cond_t* cond, void (*wait_for)(void*), void* arg)
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

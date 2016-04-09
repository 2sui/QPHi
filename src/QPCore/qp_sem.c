
/**
  * Copyright (C) 2sui.
  */


#include "qp_o_ipc.h"


inline void
qp_sem_set_inited(qp_sem_t* sem)
{ sem ? sem->is_inited = true : 1;}

inline void
qp_sem_set_alloced(qp_sem_t* sem)
{ sem ? sem->is_alloced = true : 1;}

inline void
qp_sem_set_shared(qp_sem_t* sem)
{ sem ? sem->is_shared = true : 1;}

inline void
qp_sem_unset_inited(qp_sem_t* sem)
{ sem ? sem->is_inited = false : 1;}

inline void
qp_sem_unset_alloced(qp_sem_t* sem)
{ sem ? sem->is_alloced = false : 1;}

inline void
qp_sem_unset_shared(qp_sem_t* sem)
{ sem ? sem->is_shared = false : 1;}


qp_sem_t*
qp_sem_create(qp_sem_t* sem, bool shared)
{
    if (NULL == sem) {
        sem = (qp_sem_t*)qp_alloc(sizeof(qp_sem_t));
        
        if (NULL == sem) {
            QP_LOGOUT_ERROR("[qp_sem_t] Sem create fail.");
            return NULL;
        }
        
        memset(sem, 0, sizeof(qp_sem_t));
        qp_sem_set_alloced(sem);
        
    } else {
        memset(sem, 0, sizeof(qp_sem_t));
    }
    
    shared ? qp_sem_set_shared(sem) : 1;
    
    if (QP_SUCCESS != sem_init(&sem->sem, (qp_int_t)(sem->is_shared), 0)) {
        qp_sem_is_alloced(sem) ? qp_free(sem) : 1;
        QP_LOGOUT_ERROR("[qp_sem_t] Sem init fail.");
        return NULL;
    }
    
    qp_sem_set_inited(sem);
    return sem;
}

qp_sem_t*
qp_sem_init(qp_sem_t* sem, bool shared)
{
    return qp_sem_create(sem, shared);
}

qp_int_t
qp_sem_destroy(qp_sem_t* sem)
{
    if (qp_sem_is_inited(sem)) {
        /* incase it still wait for signal */
        qp_sem_post(sem);
        
        if (QP_SUCCESS != sem_destroy(&sem->sem)) {
            QP_LOGOUT_ERROR("[qp_sem_t] Sem destroy error.");
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
qp_sem_post(qp_sem_t* sem)
{
    return sem ? sem_post(&sem->sem) : QP_ERROR;
}

qp_int_t
qp_sem_trywait(qp_sem_t* sem)
{
    return sem ? sem_trywait(&sem->sem) : QP_ERROR;
}

qp_int_t
qp_sem_wait(qp_sem_t* sem)
{
    return sem ? sem_wait(&sem->sem) : QP_ERROR;
}


/**
  * Copyright (C) 2sui.
  */


#include "qp_o_ipc.h"


inline void
qp_shm_set_inited(qp_shm_t* shm)
{ shm->is_inited = true;}

inline void
qp_shm_set_alloced(qp_shm_t* shm)
{ shm->is_alloced = true;}

inline void
qp_shm_unset_inited(qp_shm_t* shm)
{ shm->is_inited = false;}

inline void
qp_shm_unset_alloced(qp_shm_t* shm)
{ shm->is_alloced = false;}


qp_shm_t*
qp_shm_create(qp_shm_t* shm, size_t size, qp_int_t fd)
{
    if (NULL == shm) {
        shm = (qp_shm_t*)qp_alloc(sizeof(qp_shm_t));
        
        if (NULL == shm) {
            QP_LOGOUT_ERROR("[qp_shm_t] Shm create fail.");
            return NULL;
        }
        
        memset(shm, 0, sizeof(qp_shm_t));
        qp_shm_set_alloced(shm);
        
    } else {
        memset(shm, 0, sizeof(qp_shm_t));
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
        QP_LOGOUT_ERROR("[qp_shm_t] Shm create fail.");
        return NULL;
    }
    
    qp_shm_set_inited(shm);
    return shm;
}

qp_shm_t*
qp_shm_init(qp_shm_t* shm, size_t size, const qp_char_t* name)
{
    qp_int_t fd = -1;
    
    if (name) {
        fd = shm_open(name, O_RDWR | O_CREAT, \
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH); 
        
        if (fd < 0) {
            QP_LOGOUT_ERROR("[qp_shm_t] Shm open fail.");
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
qp_shm_destroy(qp_shm_t* shm)
{
    if (qp_shm_is_inited(shm)) {
        
        if (QP_ERROR == munmap(shm->addr, shm->size)) {
            QP_LOGOUT_ERROR("[qp_shm_t] Shm destroy error.");
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
qp_shm_start(qp_shm_t* shm)
{
    if (qp_shm_is_inited(shm)) {
        return shm->addr;
    }
    
    return NULL;
}
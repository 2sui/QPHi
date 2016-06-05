
/**
  * Copyright (C) 2sui.
  *
  * Memory pool.
  */

#ifndef QP_O_POOL_H
#define QP_O_POOL_H


#include "qp_pool2.h"
#include "qp_o_memory.h"


struct qp_pool_elm_s {
    struct qp_list_s    next;
    qp_pool_t           root;
};

struct qp_pool_s {
    size_t            nsize;     /* max element number */
    size_t            esize;     /* element size */
    qp_uchar_t*       room;      /* data room */
    struct qp_list_s  idle;
    struct qp_list_s  used;
    size_t            nfree;
    bool              is_inited;
    bool              is_alloced;
};

struct qp_pool_manager_elm_s {
    qp_pool_manager_t   manager;
    struct qp_list_s    pool;
    struct qp_queue_s   queue;
};

struct qp_pool_manager_s {
    struct qp_queue_s      pool_queue; /* queue of pools */
    qp_pool_manager_elm_t  current;
    size_t                 pool_count; /* pool count in queue */
    size_t                 used_count; /* totol used element in manager */
    size_t                 esize;      /* size per element in pool */
    size_t                 ecount;     /* element number in pool */
    bool                   is_inited;
    bool                   is_alloced;
};

#ifdef __cplusplus
extern "C" {
#endif




#ifdef __cplusplus
}
#endif

#endif /* QP_O_POOL_H */


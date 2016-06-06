
/**
  * Copyright (C) 2sui.
  *
  * Memory pool.
  */


#ifndef QP_O_POOL_H
#define QP_O_POOL_H


#include "qp_pool.h"
#include "qp_o_memory.h"


#ifdef __cplusplus
extern "C" {
#endif


struct qp_pool_elm_s {
    struct qp_list_s        next;
    struct qp_pool_s*       root;
};

struct qp_pool_s {
    size_t                  nsize;     /* max element number */
    size_t                  esize;     /* element size */
    qp_uchar_t*             room;      /* data room */
    struct qp_list_s        idle;
    struct qp_list_s        used;
    size_t                  nfree;
    bool                    is_inited;
    bool                    is_alloced;
};

struct qp_pool_manager_elm_s {
    struct qp_pool_manager_s*  manager;
    struct qp_pool_s           pool;
    struct qp_queue_s          queue;
};

struct qp_pool_manager_s {
    struct qp_queue_s              pool_queue; /* queue of pools */
    struct qp_pool_manager_elm_s*  current;
    size_t                         pool_count; /* pool count in queue */
    size_t                         used_count; /* totol used element in manager */
    size_t                         esize;      /* size per element in pool */
    size_t                         ecount;     /* element number in pool */
    bool                           is_inited;
    bool                           is_alloced;
};

#ifdef __cplusplus
}
#endif

#endif /* QP_POOL_H */

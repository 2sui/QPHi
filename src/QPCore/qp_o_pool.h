
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


static inline void
qp_pool_set_inited(qp_pool_t pool)
{ 
    pool ? pool->is_inited = true : 1;
}

static inline void
qp_pool_set_alloced(qp_pool_t pool)
{ 
    pool ? pool->is_alloced = true : 1;
}

static inline void
qp_pool_manager_set_inited(qp_pool_manager_t manager)
{ 
    manager ? manager->is_inited = true : 1;
}

static inline void
qp_pool_manager_set_alloced(qp_pool_manager_t manager)
{ 
    manager ? manager->is_alloced = true : 1;
}

static inline void
qp_pool_unset_inited(qp_pool_t pool)
{ 
    pool ? pool->is_inited = false : 1;
}

static inline void
qp_pool_unset_alloced(qp_pool_t pool)
{ 
    pool ? pool->is_alloced = false : 1;
}

static inline void
qp_pool_manager_unset_inited(qp_pool_manager_t manager)
{ 
    manager ? manager->is_inited = false : 1;
}

static inline void
qp_pool_manager_unset_alloced(qp_pool_manager_t manager)
{ 
    manager ? manager->is_alloced = false : 1;
}

static inline bool
qp_pool_is_inited(qp_pool_t pool) 
{ 
    return pool ? pool->is_inited : false;
}

static inline bool
qp_pool_is_alloced(qp_pool_t pool)
{ 
    return pool ? pool->is_alloced : false;
}

static inline bool
qp_pool_manager_is_inited(qp_pool_manager_t manager)
{ 
    return manager ? manager->is_inited : false;
}

static inline bool
qp_pool_manager_is_alloced(qp_pool_manager_t manager)
{ 
    return manager ? manager->is_alloced : false;
}

#ifdef __cplusplus
}
#endif

#endif /* QP_POOL_H */

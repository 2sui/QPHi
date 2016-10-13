/*
 * The MIT License
 *
 * Copyright © 2016 2sui.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef QP_POOL_CORE_H
#define QP_POOL_CORE_H

#ifdef __cplusplus
extern "C" {
#endif


#include "../qp_pool.h"
#include "qp_memory_core.h"


struct qp_pool_elm_s {
    struct qp_list_s        next;
    struct qp_pool_s*       root;
};


struct qp_pool_s {
    struct qp_list_s        free;
    qp_uchar_t*             room;      /* data room */
    void*                   info_node; /* stored info related to this pool */
    size_t                  nsize;     /* max element number */
    size_t                  esize;     /* element size */
    size_t                  nfree;
    bool                    is_inited;
    bool                    is_alloced;
};

//struct qp_pool_manager_elm_s {
//    struct qp_pool_manager_s*  manager;
//    struct qp_pool_s           pool;
//    struct qp_queue_s          queue;
//};
//
//struct qp_pool_manager_s {
//    struct qp_queue_s              pool_queue; /* queue of pools */
//    struct qp_pool_manager_elm_s*  current;
//    size_t                         pool_count; /* pool count in queue */
//    size_t                         used_count; /* totol used element in manager */
//    size_t                         esize;      /* size per element in pool */
//    size_t                         ecount;     /* element number in pool */
//    bool                           is_inited;
//    bool                           is_alloced;
//};


static inline bool
qp_pool_is_inited(qp_pool_t pool) 
{ 
    return pool->is_inited;
}


static inline bool
qp_pool_is_alloced(qp_pool_t pool)
{ 
    return pool->is_alloced;
}


//static inline bool
//qp_pool_manager_is_inited(qp_pool_manager_t manager)
//{ 
//    return manager->is_inited;
//}
//
//
//static inline bool
//qp_pool_manager_is_alloced(qp_pool_manager_t manager)
//{ 
//    return manager->is_alloced;
//}

#ifdef __cplusplus
}
#endif

#endif /* QP_POOL_CORE_H */

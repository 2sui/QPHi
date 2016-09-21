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


#include "qp_pool_core.h"
#include "qp_debug.h"


static inline void
qp_pool_set_inited(qp_pool_t pool)
{ 
    pool->is_inited = true;
}


static inline void
qp_pool_set_alloced(qp_pool_t pool)
{ 
    pool->is_alloced = true;
}


static inline void
qp_pool_manager_set_inited(qp_pool_manager_t manager)
{ 
    manager->is_inited = true;
}


static inline void
qp_pool_manager_set_alloced(qp_pool_manager_t manager)
{ 
    manager->is_alloced = true;
}


static inline void
qp_pool_unset_inited(qp_pool_t pool)
{ 
    pool->is_inited = false;
}


static inline void
qp_pool_unset_alloced(qp_pool_t pool)
{ 
    pool->is_alloced = false;
}


static inline void
qp_pool_manager_unset_inited(qp_pool_manager_t manager)
{ 
    manager->is_inited = false;
}


static inline void
qp_pool_manager_unset_alloced(qp_pool_manager_t manager)
{ 
    manager->is_alloced = false;
}


qp_pool_t
qp_pool_create(qp_pool_t pool)
{
    if (NULL == pool) {
        pool = (qp_pool_t)qp_alloc(sizeof(struct qp_pool_s));
        
        if (NULL == pool) {
            return NULL;
        }
        
        memset(pool, 0, sizeof(struct qp_pool_s));
        qp_pool_set_alloced(pool);
        
    } else {
        memset(pool, 0, sizeof(struct qp_pool_s));
    }
    
    qp_list_init(&pool->free);
    qp_pool_set_inited(pool);
    return pool;
}


qp_pool_t
qp_pool_init(qp_pool_t pool, size_t elmsize, size_t count)
{
    size_t i = 0, offset = 0, bucket = 0;
    qp_pool_elm_t  elements = NULL;
    pool = qp_pool_create(pool);
    
    if (NULL == pool) {
        return NULL;
    }
    
    pool->esize = elmsize;
    pool->nsize = count;
    pool->nfree = pool->nsize;
    
    bucket = pool->esize + sizeof(struct qp_pool_elm_s);
    pool->room = (qp_uchar_t*)qp_alloc(bucket * pool->nsize);
    
    if (NULL == pool->room) {   
        qp_pool_destroy(pool, true);
        return NULL;
    }
    
    for (i = 0, offset = 0; i < pool->nsize; i++, offset += bucket) {
        elements = (qp_pool_elm_t)(pool->room + offset);
        elements->root = pool;
        qp_list_push(&pool->free, &elements->next);
    }
    
    return pool;
}


qp_int_t 
qp_pool_destroy(qp_pool_t pool, bool force)
{
    if (qp_pool_is_inited(pool)) {
        
        if (!force && (pool->nfree != pool->nsize)) {
            return QP_ERROR;
        }
        
        if (pool->room) {
            qp_free(pool->room);
            pool->room = NULL;
        }
        
        qp_pool_unset_inited(pool);
        
        if (qp_pool_is_alloced(pool)) {
            qp_free(pool);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}


inline size_t
qp_pool_available(qp_pool_t pool)
{
    return pool->nfree;
}


inline size_t
qp_pool_used(qp_pool_t pool)
{
    return pool->nsize - pool->nfree;
}


void*
qp_pool_to_array(qp_pool_t pool, size_t index)
{
    if (qp_pool_is_inited(pool)) {
        return (void*)(pool->room + (index * (pool->esize + 
            sizeof(struct qp_pool_elm_s))) + sizeof(struct qp_pool_elm_s));
    }
    
    return NULL;
}


inline qp_pool_elm_t
qp_pool_belong_to(void* ptr)
{
    return (qp_pool_elm_t)((qp_uchar_t*)ptr - sizeof(struct qp_pool_elm_s));
}


void* 
qp_pool_alloc(qp_pool_t pool, size_t size)
{
    qp_pool_elm_t elements = NULL;
    qp_list_t     node = NULL;
    
    if (size <= pool->esize && qp_pool_available(pool) > 0) {
        if (!(node = qp_list_first(&pool->free))) {
            return NULL;
        }
        
        qp_list_pop(&pool->free);
        pool->nfree--;
        node->next = NULL;
        elements = (qp_pool_elm_t)qp_list_data(node, struct qp_pool_elm_s, next);
        return (void*)((qp_uchar_t*)elements + sizeof(struct qp_pool_elm_s));
    }
    
    return NULL;
}


qp_int_t
qp_pool_free(qp_pool_t pool, void* ptr)
{
    qp_pool_elm_t elements = qp_pool_belong_to(ptr);
    if (elements->root != pool) {
        return QP_ERROR;
    }
        
    qp_list_push(&pool->free, &elements->next);
    pool->nfree++;
    return QP_SUCCESS;
}


qp_pool_manager_t
qp_pool_manager_create(qp_pool_manager_t manager)
{
    if (NULL == manager) {
        manager = (qp_pool_manager_t)qp_alloc(sizeof(struct qp_pool_manager_s));
        
        if (NULL == manager) {
            return NULL;
        }
        
        memset(manager, 0, sizeof(struct qp_pool_manager_s));
        qp_pool_manager_set_alloced(manager);
        
    } else {
        memset(manager, 0, sizeof(struct qp_pool_manager_s));
    }
    
    qp_queue_init(&manager->pool_queue);
    qp_pool_manager_set_inited(manager);
    return manager;
}


qp_pool_manager_t
qp_pool_manager_init(qp_pool_manager_t manager, size_t elmsize, size_t count)
{
    manager = qp_pool_manager_create(manager);
    
    if (!manager) {
        return NULL;
    }
    
    manager->esize = elmsize;
    manager->ecount = count;
    return manager;
}


qp_int_t
qp_pool_manager_destroy(qp_pool_manager_t manager, bool force)
{
    if (qp_pool_manager_is_inited(manager)) {
        qp_pool_manager_elm_t pool = NULL;
        
        while (!qp_queue_is_empty(&manager->pool_queue)) {
            pool = (qp_pool_manager_elm_t)qp_queue_data(qp_queue_first(&manager->pool_queue), \
                struct qp_pool_manager_elm_s, queue);
            qp_queue_remove(&pool->queue);
            qp_pool_destroy(&pool->pool, force);
            qp_free(pool);
        }
     
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}


//void*
//qp_pool_manager_alloc(qp_pool_manager_t manager, size_t size, qp_pool_t* npool)
//{
//    void* ptr = NULL;
        
    /* if have not current pool, find or create one */
//    if ((NULL == manager->current) 
//        || !qp_pool_available(&manager->current->pool)) 
//    {
//        qp_queue_t node = qp_queue_first(&manager->pool_queue);
//        manager->current = NULL;
//            
//        /* find available pool */
//        while (node && (node != &manager->pool_queue)) {
//            manager->current = (qp_pool_manager_elm_t)qp_queue_data(node, 
//                struct qp_pool_manager_elm_s, queue);
//        
//            /* have room in this pool, use it */
//            if (qp_pool_available(&manager->current->pool)) {
//                break;
//            } 
//                
//            manager->current = NULL;
//            node = qp_queue_next(node);
//        }
//            
//        /* if no pool available, create one */
//        if (NULL == manager->current) {
//            manager->current = (qp_pool_manager_elm_t) 
//                qp_alloc(sizeof(struct qp_pool_manager_elm_s));
//                
//            if (NULL == manager->current) {
//                return NULL;
//            }
//                
//            if (NULL == qp_pool_init(&(manager->current->pool),
//                manager->esize, manager->ecount)) 
//            {
//                qp_free(manager->current);
//                manager->current = NULL;
//                return NULL;
//            }
//                
//            qp_queue_insert_after_head(&(manager->pool_queue), 
//                &(manager->current->queue));
//            manager->current->manager = manager;
//            manager->pool_count++;
//        }
//    }
//        
//    if (NULL == (ptr = qp_pool_alloc(&manager->current->pool, size))) {
//        return NULL;
//    }
//        
//    manager->used_count++;
//        
//    if (npool) {
//        *npool = &(manager->current->pool);
//    }
//        
//    return ptr;
//}


//qp_int_t
//qp_pool_manager_free(qp_pool_manager_t manager, void* ptr, qp_pool_t npool)
//{
//    if (manager->used_count)  {
//        
//        if (!npool) {
//           /* find the pool that ptr belongs to */
//            npool = qp_pool_belong_to(ptr)->root;
//        }
//        
//        if (QP_ERROR == qp_pool_free(npool, ptr)) {
//            return QP_ERROR;
//        }
//        
//        manager->used_count--;
//        
//        /* if pool is empty , free it */
//        if (qp_pool_available(npool) == npool->nsize) {
//            qp_pool_manager_elm_t element = qp_pool_manager_belong_to(npool);
//            
//            if ((element->manager == manager)
//                && (manager->pool_count > 1)) 
//            {
//                qp_queue_remove(&(element->queue));
//                qp_pool_destroy(&(element->pool), true);
//                qp_free(element);
//                manager->pool_count--;
//                
//            } else {
//                return QP_ERROR;
//            }  
//        }
//        
//        return QP_SUCCESS;
//    }
//    
//    return QP_ERROR;
//}


inline qp_pool_manager_elm_t
qp_pool_manager_belong_to(qp_pool_t pool)
{
    return (qp_pool_manager_elm_t)((qp_uchar_t*)pool - \
        offsetof(struct qp_pool_manager_elm_s, pool));
}


inline size_t
qp_pool_manager_used(qp_pool_manager_t manager)
{
    return manager->used_count;
}

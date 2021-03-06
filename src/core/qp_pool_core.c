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
    if (pool) {
        pool->is_inited = true;
    }
}


static inline void
qp_pool_set_alloced(qp_pool_t pool)
{ 
    if (pool) {
        pool->is_alloced = true;
    }
}


static inline void
qp_pool_unset_inited(qp_pool_t pool)
{ 
    if (pool) {
        pool->is_inited = false;
    }
}


static inline void
qp_manager_set_inited(qp_manager_t manager)
{ 
    if (manager) {
        manager->is_inited = true;
    }
}


static inline void
qp_manager_set_alloced(qp_manager_t manager)
{ 
    if (manager) {
        manager->is_alloced = true;
    }
}



static inline void
qp_manager_unset_inited(qp_manager_t manager)
{ 
    if (manager) {
        manager->is_inited = false;
    }
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


size_t
qp_pool_available(qp_pool_t pool)
{
    return pool->nfree;
}


size_t
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


qp_pool_elm_t
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


qp_manager_t
qp_manager_create(qp_manager_t manager)
{
    if (NULL == manager) {
        manager = (qp_manager_t)qp_alloc(sizeof(struct qp_manager_s));
        
        if (NULL == manager) {
            return NULL;
        }
        
        memset(manager, 0, sizeof(struct qp_manager_s));
        qp_manager_set_alloced(manager);
        
    } else {
        memset(manager, 0, sizeof(struct qp_manager_s));
    }
    
    qp_queue_init(&manager->pools_queue);
    qp_list_init(&manager->available_pools);
    qp_manager_set_inited(manager);
    return manager;
}


qp_manager_t
qp_manager_init(qp_manager_t manager, size_t elmsize, size_t count)
{
    manager = qp_manager_create(manager);
    
    if (!manager) {
        return NULL;
    }
    
    manager->esize = elmsize;
    manager->ecount = count;
    return manager;
}


qp_int_t
qp_manager_destroy(qp_manager_t manager, bool force)
{
    if (qp_manager_is_inited(manager)) {
        if (!force && (manager->ecount > 0/*manager->ecount > 2*/)) {
            return QP_ERROR;
        }
        
        qp_manager_elm_t elm = NULL;
        
        while (!qp_queue_is_empty(&manager->pools_queue)) {
            elm = (qp_manager_elm_t)qp_queue_data(
                qp_queue_first(&manager->pools_queue), 
                struct qp_manager_elm_s, queue);
            qp_pool_destroy(&elm->pool, force);
            qp_queue_remove(&elm->queue);
            qp_free(elm);
        }
     
        qp_manager_unset_inited(manager);
        
        if (qp_manager_is_alloced(manager)) {
            qp_free(manager);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}


size_t 
qp_manager_available(qp_manager_t manager) 
{
    return manager->nfree;
}


size_t
qp_manager_used(qp_manager_t manager)
{
    return manager->pool_count * manager->ecount - manager->nfree;
}


void* 
qp_manager_alloc(qp_manager_t manager, size_t size)
{
    if (!manager || size > manager->esize) {
        return NULL;
    }
    
    // if no available pool, allocate one 
    qp_list_t node = qp_list_first(&manager->available_pools);
    if (!node) {
        qp_manager_elm_t elm = (qp_manager_elm_t)
            qp_alloc(sizeof(struct qp_manager_elm_s));
        if (!elm || !qp_pool_init(&elm->pool, manager->esize, manager->ecount)) {
            if (elm) {
                qp_free(elm);
            }
            return NULL;
        }
        
        elm->manager = manager;
        node = &elm->link;
        qp_queue_insert_after_head(&manager->pools_queue, &elm->queue);
        qp_list_push(&manager->available_pools, node);
        manager->nfree += manager->ecount;
        manager->pool_count++;
    }
    
    // qp_manager_elm_t
    qp_manager_elm_t elm = (qp_manager_elm_t)qp_list_data(node, 
        struct qp_manager_elm_s, link);
    // allocate block from elm
    void* ptr = qp_pool_alloc(&elm->pool, size);
    if (!ptr) {
        return NULL;
    }
    
    manager->nfree--;
    if (1 > qp_pool_available(&elm->pool)) {
        qp_list_pop(&manager->available_pools);
        qp_list_init(&elm->link);
    }
    
    return ptr;
}


qp_int_t
qp_manager_free(qp_manager_t manager, void* ptr)
{
    if (!manager || !ptr) {
        return QP_ERROR;
    }
    
    qp_pool_elm_t pool_elm = (qp_pool_elm_t)qp_pool_belong_to(ptr);
    qp_pool_t pool = pool_elm->root;
    qp_manager_elm_t elm = (qp_manager_elm_t)qp_list_data(pool, 
        struct qp_manager_elm_s, pool);
    if (elm->manager != manager) {
        return QP_ERROR;
    }
    
    qp_pool_free(pool, ptr);
    manager->nfree++;
    // if not added to available list, add
    if (NULL == qp_list_first(&elm->link)) {
        qp_list_push(&manager->available_pools, &elm->link);
        
    } else {
        // if the head of available pool list is empty, free it
        if (/*1 < manager->pool_count &&*/ 1 > qp_pool_used(pool) 
            && &elm->link == qp_list_first(&manager->available_pools)) 
        {
            qp_list_pop(&manager->available_pools);
            qp_queue_remove(&elm->queue);
            qp_pool_destroy(pool, true);
            qp_free(elm);
            manager->pool_count--;
            manager->nfree -= manager->ecount;   
        }
    }
    
    return QP_SUCCESS;
}

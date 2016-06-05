
/**
  * Copyright (C) 2sui.
  */


#include "qp_o_pool.h"


inline void
qp_pool_set_inited(qp_pool_t* pool)
{ pool ? pool->is_inited = true : 1;}

inline void
qp_pool_set_alloced(qp_pool_t* pool)
{ pool ? pool->is_alloced = true : 1;}

inline void
qp_pool_manager_set_inited(qp_pool_manager_t* manager)
{ manager ? manager->is_inited = true : 1;}

inline void
qp_pool_manager_set_alloced(qp_pool_manager_t* manager)
{ manager ? manager->is_alloced = true : 1;}

inline void
qp_pool_unset_inited(qp_pool_t* pool)
{ pool ? pool->is_inited = false : 1;}

inline void
qp_pool_unset_alloced(qp_pool_t* pool)
{ pool ? pool->is_alloced = false : 1;}

inline void
qp_pool_manager_unset_inited(qp_pool_manager_t* manager)
{ manager ? manager->is_inited = false : 1;}

inline void
qp_pool_manager_unset_alloced(qp_pool_manager_t* manager)
{ manager ? manager->is_alloced = false : 1;}

inline bool
qp_pool_is_inited(qp_pool_t* pool) 
{ return pool ? pool->is_inited : false;}

inline bool
qp_pool_is_alloced(qp_pool_t* pool)
{ return pool ? pool->is_alloced : false;}

inline bool
qp_pool_manager_is_inited(qp_pool_manager_t* manager)
{ return manager ? manager->is_inited : false;}

inline bool
qp_pool_manager_is_alloced(qp_pool_manager_t* manager)
{ return manager ? manager->is_alloced : false;}


qp_pool_t*
qp_pool_create(qp_pool_t* pool)
{
    if (NULL == pool) {
        pool = (qp_pool_t*)qp_alloc(sizeof(qp_pool_t));
        
        if (NULL == pool) {
            return NULL;
        }
        
        memset(pool, 0, sizeof(qp_pool_t));
        qp_pool_set_alloced(pool);
        
    } else {
        memset(pool, 0, sizeof(qp_pool_t));
    }
    
    qp_list_init(&pool->idle);
    qp_list_init(&pool->used);
    qp_pool_set_inited(pool);
    return pool;
}

qp_pool_t*
qp_pool_init(qp_pool_t* pool, size_t elmsize, size_t count)
{
    size_t i = 0, offset = 0;
    qp_pool_elm_t* elements = NULL;
    pool = qp_pool_create(pool);
    
    if (NULL == pool) {
        return NULL;
    }
    
    pool->esize = elmsize;
    pool->nsize = count;
    pool->nfree = pool->nsize;
    
    pool->room = (qp_uchar_t*)qp_alloc((pool->esize + sizeof(qp_pool_elm_t)) \
        * pool->nsize);
    
    if (NULL == pool->room) {   
        qp_pool_destroy(pool, true);
        return NULL;
    }
    
    for (;i < pool->nsize; i++, offset += pool->esize + sizeof(qp_pool_elm_t)) {
        elements = (qp_pool_elm_t*)(pool->room + offset);
        qp_list_push(&pool->idle, &elements->next);
        elements->root = pool;
    }
    
    return pool;
}

qp_int_t 
qp_pool_destroy(qp_pool_t* pool, bool force)
{
    if (qp_pool_is_inited(pool)) {
        
        if (!force) {
            
            if (pool->nfree != pool->nsize) {
                return QP_ERROR;
            }
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

void* 
qp_pool_alloc(qp_pool_t* pool, size_t size)
{
    qp_pool_elm_t* elements = NULL;
    qp_list_t*     node = NULL;
    
    if (qp_pool_is_inited(pool) && (size <= pool->esize)) {
        node = qp_list_first(&pool->idle);
        
        if (!node) {
            return NULL;
        }

        elements = qp_list_data(node, qp_pool_elm_t, next);
//        qp_list_push(&pool->used, &elements->next);
        qp_list_pop(&pool->idle);
        pool->nfree--;
        return (void*)((qp_uchar_t*)elements + sizeof(qp_pool_elm_t));
    }
    
    return NULL;
}

qp_int_t
qp_pool_free(qp_pool_t* pool, void* ptr)
{
    qp_pool_elm_t* elements = qp_pool_belong_to(ptr);
    
    if (qp_pool_is_inited(pool)) {
        
        if (elements->root != pool) {
            return QP_ERROR;
        }
        
        qp_list_push(&pool->idle, &elements->next);
//        qp_list_pop(&elments->next);
        pool->nfree++;
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

size_t
qp_pool_available(qp_pool_t* pool)
{
    return qp_pool_is_inited(pool) ? pool->nfree : 0;
}

size_t
qp_pool_used(qp_pool_t* pool)
{
    return qp_pool_is_inited(pool) ? (pool->nsize - pool->nfree) : 0;
}

void*
qp_pool_to_array(qp_pool_t* pool, size_t index)
{
    if (qp_pool_is_inited(pool)) {
        return (void*)(pool->room + (index * (pool->esize + 
            sizeof(qp_pool_elm_t))) + sizeof(qp_pool_elm_t));
    }
    
    return NULL;
}

qp_pool_elm_t*
qp_pool_belong_to(void* ptr)
{
    return (qp_pool_elm_t*)((qp_uchar_t*)ptr - sizeof(qp_pool_elm_t));
}



qp_pool_manager_t*
qp_pool_manager_create(qp_pool_manager_t* manager)
{
    if (NULL == manager) {
        manager = (qp_pool_manager_t*)qp_alloc(sizeof(qp_pool_manager_t));
        
        if (NULL == manager) {
            return NULL;
        }
        
        memset(manager, 0, sizeof(qp_pool_manager_t));
        qp_pool_manager_set_alloced(manager);
        
    } else {
        memset(manager, 0, sizeof(qp_pool_manager_t));
    }
    
    qp_queue_init(&manager->pool_queue);
    qp_pool_manager_set_inited(manager);
    return manager;
}

qp_pool_manager_t*
qp_pool_manager_init(qp_pool_manager_t* manager, size_t elmsize, size_t count)
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
qp_pool_manager_destroy(qp_pool_manager_t* manager, bool force)
{
    if (qp_pool_manager_is_inited(manager)) {
        qp_pool_manager_elm_t* pool;
        
#ifdef QP_DEBUG
        size_t counter = 0;
#endif 
        while (!qp_queue_is_empty(&manager->pool_queue)) {
            pool = qp_queue_data(qp_queue_first(&manager->pool_queue), \
                qp_pool_manager_elm_t, queue);
            qp_queue_remove(&pool->queue);
            qp_pool_destroy(&pool->pool, force);
            qp_free(pool);
            
#ifdef QP_DEBUG
            counter++;
#endif 
        }
     
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

void*
qp_pool_manager_alloc(qp_pool_manager_t* manager, size_t size, qp_pool_t** npool)
{
    if (qp_pool_manager_is_inited(manager)) {
        void* ptr = NULL;
        
        /* if not say current pool, find or create one */
        if ((NULL == manager->current) 
            || !qp_pool_available(&manager->current->pool)) 
        {
            qp_queue_t* node = qp_queue_first(&manager->pool_queue);
            manager->current = NULL;
            
            /* find available pool */
            while (node && (node != &manager->pool_queue)) {
                manager->current = qp_queue_data(node, qp_pool_manager_elm_t, \
                    queue);
        
                /* have room in this pool, use it */
                if (qp_pool_available(&manager->current->pool)) {
                    break;
                } 
                
                manager->current = NULL;
                node = qp_queue_next(node);
            }
            
            /* if no pool available, create one */
            if (NULL == manager->current) {
                manager->current = \
                    (qp_pool_manager_elm_t*)qp_alloc(sizeof(qp_pool_manager_elm_t));
                
                if (NULL == manager->current) {
                    return NULL;
                }
                
                if (NULL == qp_pool_init(&(manager->current->pool),\
                    manager->esize, manager->ecount)) 
                {
                    qp_free(manager->current);
                    manager->current = NULL;
                    return NULL;
                }
                
                qp_queue_insert_after_head(&(manager->pool_queue), \
                    &(manager->current->queue));
                manager->current->manager = manager;
                manager->pool_count++;
            }
        }
        
        ptr =  qp_pool_alloc(&manager->current->pool, size);
        
        if (NULL == ptr) {
            return ptr;
        }
        
        manager->used_count++;
        
        if (npool) {
            *npool = &(manager->current->pool);
        }
        
        return ptr;
    }
    
    return NULL;
}

qp_int_t
qp_pool_manager_free(qp_pool_manager_t* manager, void* ptr, qp_pool_t* npool)
{
    if (qp_pool_manager_is_inited(manager) && manager->used_count)  {
        
        if (!npool) {
           /* find the pool that ptr belongs to */
            npool = qp_pool_belong_to(ptr)->root;
        }
        
        if (QP_ERROR == qp_pool_free(npool, ptr)) {
            return QP_ERROR;
        }
        
        manager->used_count--;
        
        /* if pool is empty , free it */
        if (qp_pool_available(npool) == npool->nsize) {
            qp_pool_manager_elm_t* element = qp_pool_manager_belong_to(npool);
            
            if ((element->manager == manager)
                && (manager->pool_count > 1)) 
            {
                qp_queue_remove(&(element->queue));
                qp_pool_destroy(&(element->pool), true);
                qp_free(element);
                manager->pool_count--;
                
            } else {
                // TODO:
            }  
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

qp_pool_manager_elm_t*
qp_pool_manager_belong_to(qp_pool_t* pool)
{
    return (qp_pool_manager_elm_t*)((qp_uchar_t*)pool - \
        offsetof(qp_pool_manager_elm_t, pool));
}


size_t
qp_pool_manager_used(qp_pool_manager_t* manager)
{
    if (qp_pool_manager_is_inited(manager)) {
        return manager->used_count;
    }
    
    return 0;
}

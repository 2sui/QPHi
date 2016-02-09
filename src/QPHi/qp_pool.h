
/**
  * Copyright (C) sui
  *
  * Memory pool.
  */


#ifndef QP_POOL_H
#define QP_POOL_H

#ifdef __cplusplus
extern "C" {
#endif
    

#include "qp_o_memory.h"
    

typedef struct qp_pool_elm_s    qp_pool_elm_t;
typedef struct qp_pool_s        qp_pool_t;


struct qp_pool_elm_s {
    qp_list_t     next;
    qp_pool_t*    root;
};


struct qp_pool_s {
    size_t           nsize;     /* max element number */
    size_t           esize;     /* element size */
    qp_uchar_t*      room;      /* data room */
    qp_list_t        idle;
    qp_list_t        used;
    size_t           nfree;
    bool             is_inited;
    bool             is_alloced;
};


typedef struct qp_pool_manager_s        qp_pool_manager_t;
typedef struct qp_pool_manager_elm_s    qp_pool_manager_elm_t;

struct qp_pool_manager_elm_s {
    qp_pool_manager_t*  manager;
    qp_pool_t           pool;
    qp_queue_t          queue;
};

struct qp_pool_manager_s {
    qp_queue_t      pool_queue; /* queue of pools */
    size_t          pool_count; /* pool count in queue */
    size_t          used_count; /* totol used element in manager */
    size_t          esize;      /* size per element in pool */
    size_t          ecount;     /* element number in pool */
    qp_pool_manager_elm_t* current;
    bool            is_inited;
    bool            is_alloced;
};



inline bool
qp_pool_is_inited(qp_pool_t* pool);

inline bool
qp_pool_is_alloced(qp_pool_t* pool);

inline bool
qp_pool_mamager_is_inited(qp_pool_manager_t* manager);

inline bool
qp_pool_manager_is_shared(qp_pool_manager_t* manager);


/**
 * Create a memory pool.
 */
qp_pool_t*
qp_pool_create(qp_pool_t* pool);

/**
 * Init a memory pool.
 * elmsize is the size of element in pool, and count is the element count in 
 * pool.
 */
qp_pool_t*
qp_pool_init(qp_pool_t* pool, size_t elmsize, size_t count);

/**
 * Destroy a memory pool.
 */
qp_int_t
qp_pool_destroy(qp_pool_t* pool, bool force);

/**
 * Allocate a room from memory pool. The size should not
 * bigger than pool element size.
 * 
 * If success return a memory block, and if some error happen or pool used up 
 * it will return NULL.
 */
void*
qp_pool_alloc(qp_pool_t* pool, size_t size);

/**
 * Free a room of memory pool.
 */
qp_int_t
qp_pool_free(qp_pool_t* pool, void* ptr);

/**
 * Get the element number that not used.
 */
size_t
qp_pool_available(qp_pool_t* pool);

size_t
qp_pool_used(qp_pool_t* pool);

/**
 * Change the pool to array, and return the ptr in index.
 */
void*
qp_pool_to_array(qp_pool_t* pool, size_t index);

/**
 * Which element dose the ptr belongs to.
 */
qp_pool_elm_t*
qp_pool_belong_to(void* ptr);


/**
 * Create a pool manager.
 */
qp_pool_manager_t*
qp_pool_manager_create(qp_pool_manager_t* manager);

/**
 * Init a pool manager.
 */
qp_pool_manager_t*
qp_pool_manager_init(qp_pool_manager_t* manager, size_t elmsize, size_t count);

/** 
 * Destroy a pool manager. If force is true, all pool will destroy no matter 
 * there still have element in used.
 */
qp_int_t
qp_pool_manager_destroy(qp_pool_manager_t* manager, bool force);

/**
 * Allocate memory with size.If pool is NOT NULL , it will point the pool that
 * the allocated memory belong to.
 */
void*
qp_pool_manager_alloc(qp_pool_manager_t* manager, size_t size, qp_pool_t** npool);

/**
 * Free memory. If pool is not NULL that mean the ptr belong to pool and it will
 * be freed from pool.
 */
qp_int_t
qp_pool_manager_free(qp_pool_manager_t* manager, void* ptr, qp_pool_t* npool);


qp_pool_manager_elm_t*
qp_pool_manager_belong_to(qp_pool_t* pool);


size_t
qp_pool_manager_used(qp_pool_manager_t* manager);

#ifdef __cplusplus
}
#endif

#endif /* QP_POOL_H */


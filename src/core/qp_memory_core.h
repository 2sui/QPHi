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


#ifndef QP_MEMORY_CORE_H
#define QP_MEMORY_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

    
#include "qp_defines.h"
 

/*
 * Allocate memory, return 16Byte-alignment memory on 64bit system or
 * return 8Byte-alignment memory on 32bit system.
 */
# define qp_alloc    malloc
# define qp_free     free

/*
 * Linux has memalign() or posix_memalign()
 * Solaris has memalign()
 * FreeBSD 7.0 has posix_memalign(), besides, early version's malloc()
 * aligns allocations bigger than page size at the page boundary
 */
    
/*
 * Allocate {size} byte memory with {alignment}.
 * Return memory addr if success otherwise return NULL.
 */
# if defined(QP_OS_LINUX)  || defined(QP_OS_BSD) || defined(QP_OS_SOLARIS)
void*
qp_alloc_align(size_t alignment, size_t size);
# else
# define qp_alloc_align(alignment, size)    malloc(size)
# endif

/* Alignment memory ptr */
//# define  qp_align_ptr(p,a) 
//         (qp_char_t*)(((uintptr_t)(p)+((uintptr_t)(a)-1)) & ~((uintptr_t)(a)-1))
static inline uintptr_t
qp_align_ptr(uintptr_t p,uintptr_t a) 
{
    return (p + (a-1)) & ~(a-1);
}
/* Alignment data */
# define  qp_align(d,a)  (((d)+(a-1)) & ~((a) - 1))


struct qp_list_s {
    struct qp_list_s*           next;
};


struct qp_queue_s {
    struct qp_queue_s*          prev;
    struct qp_queue_s*          next;
};


struct qp_rbtree_node_s {
    struct qp_rbtree_node_s*    left;
    struct qp_rbtree_node_s*    right;
    struct qp_rbtree_node_s*    parent;
    void*                       data;
    qp_uint64_t                 key;
    qp_uint32_t                 color;
};


struct qp_rbtree_s {
    struct qp_rbtree_node_s*    root;
    struct qp_rbtree_node_s     sentinel;
};


typedef struct qp_list_s*           qp_list_t;
typedef struct qp_queue_s*          qp_queue_t;
typedef struct qp_rbtree_node_s*    qp_rbtree_node_t;
typedef struct qp_rbtree_s*         qp_rbtree_t;


/**
 * Operation for list (or stack).
 */ 

static inline void 
qp_list_init(qp_list_t list)
{ 
    if (list) {
        list->next = list;
    }
}


static inline bool
qp_list_is_empty(qp_list_t list)
{ 
    return list ? list == list->next : true;
}


static inline qp_list_t
qp_list_head(qp_list_t list)
{ 
    return list;
}


static inline qp_list_t 
qp_list_first(qp_list_t list)
{ 
    return qp_list_is_empty(list) ? NULL : list->next;
}
 

static inline void
qp_list_push(qp_list_t list, qp_list_t node)
{
    if (list && node) {
        node->next = list->next;
        list->next = node;
    }
}


static inline void
qp_list_pop(qp_list_t list)
{ 
    if (list) {
        list->next = list->next->next;
    }
}


# define  qp_list_next            qp_list_first
# define  qp_list_remove_after    qp_list_pop
# define  qp_list_insert_after    qp_list_push
# define  qp_list_data(l, type, link)  ((qp_uchar_t*)(l) - offsetof(type, link))


/**
 * Operation for queue.
 */ 

static inline void
qp_queue_init(qp_queue_t queue)
{
    if (queue) {
        queue->prev = queue;
        queue->next = queue;
    }
}


static inline bool
qp_queue_is_empty(qp_queue_t queue)
{ 
    return queue ? queue == queue->prev : true;
}


static inline void
qp_queue_insert_after_head(qp_queue_t queue, qp_queue_t node)
{
    if (queue && node) {
        node->next = queue->next;
        node->next->prev = node;
        node->prev = queue;
        queue->next = node;
    }
}


# define qp_queue_insert_after   qp_queue_insert_after_head


static inline void
qp_queue_insert_after_tail(qp_queue_t queue, qp_queue_t node)
{
    if (queue && node) {
        node->prev = queue->prev;
        node->prev->next = node;
        node->next = queue;
        queue->prev = node;
    }
}


static inline qp_queue_t
qp_queue_first(qp_queue_t queue)
{ 
    return qp_queue_is_empty(queue) ? NULL : queue->next;
}


static inline qp_queue_t
qp_queue_last(qp_queue_t queue)
{ 
    return qp_queue_is_empty(queue) ? NULL : queue->prev;
}

  
static inline qp_queue_t
qp_queue_head(qp_queue_t queue)
{ 
    return queue;
}


static inline qp_queue_t
qp_queue_next(qp_queue_t node)
{ 
    return node ? node->next : NULL;
}


static inline qp_queue_t
qp_queue_prev(qp_queue_t node)
{ 
    return node ? node->prev : NULL;
}


static inline void
qp_queue_remove(qp_queue_t node)
{
    if (node) {
        node->next->prev = node->prev;
        node->prev->next = node->next;
    }
}


static inline void
qp_queue_split(qp_queue_t queue, qp_queue_t node, qp_queue_t newq)
{
    if (queue && node && newq) {
        newq->prev = queue->prev;
        newq->prev->next = newq;
        newq->next = node;
        queue->prev = node->prev;
        queue->prev->next = queue;
        node->prev = newq;
    }
}


static inline void
qp_queue_merge(qp_queue_t queue, qp_queue_t newq)
{
    if (queue && newq) {
        queue->prev->next = newq->next;
        newq->next->prev = queue->prev;
        queue->prev = newq->prev;
        queue->prev->next = queue;
    }
}


# define qp_queue_data(q, type, link)  ((qp_uchar_t*)(q) - offsetof(type, link))


/**
 * Operation for rbtree.
 */

# define  QP_RBTREE_RED    1
# define  QP_RBTREE_BLACK  0

# define qp_rbtree_data(t, type, link) ({\
    (type*)((qp_uchar_t*) (t) - offsetof((type), (link)));})


/* init rbtree */
void
qp_rbtree_init(qp_rbtree_t rbtree);


/* insert [node] into [rbtree], return inserted node pointer */
qp_rbtree_node_t 
qp_rbtree_insert(qp_rbtree_t rbtree, qp_rbtree_node_t node);


/* delete [node] from [rbtree], return deleted node pointer */
qp_rbtree_node_t
qp_rbtree_delete(qp_rbtree_t rbtree, qp_rbtree_node_t node);


/* find node with [key] in [rbtree] */
qp_rbtree_node_t
qp_rbtree_find(qp_rbtree_t rbtree, qp_uint32_t key);


/* min node in [rbtree] (from [node]) */
qp_rbtree_node_t
qp_rbtree_min(qp_rbtree_t rbtree, qp_rbtree_node_t node);


/* max node in [rbtree] (from [node]) */
qp_rbtree_node_t
qp_rbtree_max(qp_rbtree_t rbtree, qp_rbtree_node_t node);

#ifdef __cplusplus
}
#endif

#endif /* QP_MEMORY_CORE_H */

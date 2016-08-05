
/**
  * Copyright (C) 2sui.
  *
  * Basic memory operations.
  */


#ifndef QP_O_MEMORY
#define QP_O_MEMORY


#include "qp_o_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/* PAGE size */
#ifndef QP_PAGE_SIZE
#define QP_PAGE_SIZE   getpagesize()
#endif

/*
 * Allocate memory, return 16Byte-alignment memory on 64bit system or
 * return 8Byte-alignment memory on 32bit system.
 */
#define qp_alloc    malloc
#define qp_free     free

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
#if defined(QP_OS_LINUX)  || defined(QP_OS_BSD4) || defined(QP_OS_SOLARIS)
void*
qp_alloc_align(size_t alignment, size_t size);
#else
#define qp_alloc_align(alignment, size)    malloc(size)
#endif

/* Alignment memory ptr */
#define  qp_align_ptr(p,a) \
         (qp_char_t*)( ((uintptr_t)(p)+((uintptr_t)a-1)) & ~((uintptr_t)a-1) )
/* Alignment data */
#define  qp_align(d,a)  (((d)+(a-1)) & ~((a) - 1))


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

/**
 * Init list.
 */
inline void 
qp_list_init(qp_list_t list);

/**
 * Return true if list is empty.
 */
inline bool
qp_list_is_empty(qp_list_t list);

/**
 * Push x to l.
 */
inline void
qp_list_push(qp_list_t list, qp_list_t node);

/**
 * Pop element from l.
 */
inline void
qp_list_pop(qp_list_t list);

/**
 * Get the list head.
 */
inline qp_list_t
qp_list_head(qp_list_t list);

/**
 * Get top element of list.
 */
inline qp_list_t 
qp_list_first(qp_list_t list);

#define  qp_list_next            qp_list_first

#define  qp_list_remove_after    qp_list_pop

#define  qp_list_insert_after    qp_list_push
    

/**
 * Get structure which element l in.
 */
#define qp_list_data(l, type, link) ({ \
    (type *) ((qp_uchar_t *) l - offsetof(type, link)); })

/*
 * Init queue.
*/
inline void
qp_queue_init(qp_queue_t queue);

/*
 * Return true if queue is empty.
*/
inline bool
qp_queue_is_empty(qp_queue_t queue);

/*
 * Insert element before first element of queue.
*/
inline void
qp_queue_insert_after_head(qp_queue_t queue, qp_queue_t node);

/*
 * Insert element after some element.
*/
#define qp_queue_insert_after   qp_queue_insert_after_head

/*
 * Insert element after last element of queue.
*/
inline void
qp_queue_insert_after_tail(qp_queue_t queue, qp_queue_t node);

/*
 * Get first element of queue.
*/
inline qp_queue_t
qp_queue_first(qp_queue_t queue);

/*
 * Ge last element of queue.
*/
inline qp_queue_t
qp_queue_last(qp_queue_t queue);
        

/*
 * Get queue head.
*/
inline qp_queue_t
qp_queue_head(qp_queue_t queue);

/*
 * Get next element of q.
*/
inline qp_queue_t
qp_queue_next(qp_queue_t node);

/*
 * Get preview element of q.
*/
inline qp_queue_t
qp_queue_prev(qp_queue_t node);


/*
 * Remove element from queue.
*/
inline void
qp_queue_remove(qp_queue_t node);


/*
 * Split queue h from element q to new queue n.
*/
inline void
qp_queue_split(qp_queue_t queue, qp_queue_t node, qp_queue_t newq);

/*
 * Merge queue n to queue h (and after h).
*/
inline void
qp_queue_merge(qp_queue_t queue, qp_queue_t newq);

/*
 * Get structure which element q in.
*/
#define qp_queue_data(q, type, link) ({ \
    (type *) ((qp_uchar_t *) q - offsetof(type, link)); })


/**
 * Operation for rbtree.
 */

/**
 * Init [rbtree].
 */

#define  QP_RBTREE_RED    1
#define  QP_RBTREE_BLACK  0

inline void
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

inline bool
qp_rbtree_is_red(qp_rbtree_node_t node);

inline bool
qp_rbtree_is_black(qp_rbtree_node_t node);

inline bool
qp_rbtree_is_left(qp_rbtree_node_t node);

inline bool
qp_rbtree_is_right(qp_rbtree_node_t node);

inline bool
qp_rbtree_is_empty(qp_rbtree_t rbtree);

#define qp_rbtree_data(t, type, link) ({ \
    (type *) ((qp_uchar_t *) t - offsetof(type, link)); })


#ifdef __cplusplus
}
#endif

#endif 


/**
  * Copyright (C) 2sui.
  */


#include "qp_o_memory.h"


#if defined(QP_OS_LINUX)  || defined(QP_OS_BSD4) || defined(QP_OS_SOLARIS)

void*
qp_alloc_align(size_t alignment, size_t size)
{
#if defined(QP_OS_LINUX)  || defined(QP_OS_BSD4)
    void *memptr = NULL;
    return (0 == posix_memalign(&memptr, alignment, size)) ? memptr : NULL;
#else
    return memalign(alignment, size);
#endif
}

#endif


/**
 * Init list.
 */
inline void 
qp_list_init(qp_list_t* list)
{ list->next = list;}

/**
 * Return true if list is empty.
 */
inline bool
qp_list_is_empty(qp_list_t* list)
{ return list == list->next;}

/**
 * Push x to l.
 */
inline void
qp_list_push(qp_list_t* list, qp_list_t* node)
{
    node->next = list->next;
    list->next = node;
}

/**
 * Pop element from l.
 */
inline void
qp_list_pop(qp_list_t* list)
{ list->next = list->next->next;}

/**
 * Get the list head.
 */
inline qp_list_t*
qp_list_head(qp_list_t* list)
{ return list;}

/**
 * Get top element of list.
 */
inline qp_list_t* 
qp_list_first(qp_list_t* list)
{ return qp_list_is_empty(list) ? NULL : list->next;}
    

/*
 * Init queue.
*/
inline void
qp_queue_init(qp_queue_t* queue)
{
    queue->prev = queue;
    queue->next = queue;
}

/*
 * Return true if queue is empty.
*/
inline bool
qp_queue_is_empty(qp_queue_t* queue)
{ return queue == queue->prev;}

/*
 * Insert element before first element of queue.
*/
inline void
qp_queue_insert_after_head(qp_queue_t* queue, qp_queue_t* node)
{
    node->next = queue->next;
    node->next->prev = node;
    node->prev = queue;
    queue->next = node;
}

/*
 * Insert element after last element of queue.
*/
inline void
qp_queue_insert_after_tail(qp_queue_t* queue, qp_queue_t* node)
{
    node->prev = queue->prev;
    node->prev->next = node;
    node->next = queue;
    queue->prev = node;
}

/*
 * Get first element of queue.
*/
inline qp_queue_t*
qp_queue_first(qp_queue_t* queue)
{ return qp_queue_is_empty(queue) ? NULL : queue->next;}

/*
 * Ge last element of queue.
*/
inline qp_queue_t*
qp_queue_last(qp_queue_t* queue)
{ return qp_queue_is_empty(queue) ? NULL : queue->prev;}
        

/*
 * Get queue head.
*/
inline qp_queue_t*
qp_queue_head(qp_queue_t* queue)
{ return queue;}

/*
 * Get next element of q.
*/
inline qp_queue_t*
qp_queue_next(qp_queue_t* node)
{ return node->next;}

/*
 * Get preview element of q.
*/
inline qp_queue_t*
qp_queue_prev(qp_queue_t* node)
{ return node->prev;}


/*
 * Remove element from queue.
*/
inline void
qp_queue_remove(qp_queue_t* node)
{
    node->next->prev = node->prev;
    node->prev->next = node->next;
}


/*
 * Split queue h from element q to new queue n.
*/
inline void
qp_queue_split(qp_queue_t* queue, qp_queue_t* node, qp_queue_t* newq)
{
    newq->prev = queue->prev;
    newq->prev->next = newq;
    newq->next = node;
    queue->prev = node->prev;
    queue->prev->next = queue;
    node->prev = newq;
}

/*
 * Merge queue n to queue h (and after h).
*/
inline void
qp_queue_merge(qp_queue_t* queue, qp_queue_t* newq)
{
    queue->prev->next = newq->next;
    newq->next->prev = queue->prev;
    queue->prev = newq->prev;
    queue->prev->next = queue;
}


/**
 * Operation for rbtree.
 * If the node is root , set its parent as NULL;
 * and if the node has no child, set its left and right as sentinal.
 */
inline void
qp_rbtree_set_red(qp_rbtree_node_t* node)
{ node->color = 1;}

inline void
qp_rbtree_set_black(qp_rbtree_node_t* node)
{ node->color = 0;}

inline bool
qp_rbtree_is_red(qp_rbtree_node_t* node)
{ return node->color;}

inline bool
qp_rbtree_is_black(qp_rbtree_node_t* node)
{ return !qp_rbtree_is_red(node);}

inline qp_rbtree_node_t*
qp_rbtree_parent(qp_rbtree_node_t* node)
{ return node->parent;}

inline qp_rbtree_node_t*
qp_rbtree_grandpa(qp_rbtree_node_t* node)
{ return qp_rbtree_parent(node)->parent;}

inline bool
qp_rbtree_is_left(qp_rbtree_node_t* node)
{ return (node == qp_rbtree_parent(node)->left);}

inline bool
qp_rbtree_is_right(qp_rbtree_node_t* node)
{ return (node == qp_rbtree_parent(node)->right);}

inline qp_rbtree_node_t*
qp_rbtree_uncle(qp_rbtree_node_t* node)
{ 
    return qp_rbtree_is_left(qp_rbtree_parent(node)) ? \
        qp_rbtree_grandpa(node)->right : \
        qp_rbtree_grandpa(node)->left;
}

inline qp_rbtree_node_t*
qp_rbtree_nil(qp_rbtree_t* rbtree)
{ return &rbtree->sentinel;}

inline bool
qp_rbtree_is_empty(qp_rbtree_t* rbtree)
{ return (rbtree->root == qp_rbtree_nil(rbtree));}

/* init rbtree */
inline void
qp_rbtree_init(qp_rbtree_t* rbtree)
{
//    rbtree->sentinel.left = qp_rbtree_nil(rbtree);
//    rbtree->sentinel.right = qp_rbtree_nil(rbtree);
//    rbtree->sentinel.parent = qp_rbtree_nil(rbtree);
//    rbtree->sentinel.data = NULL;
//    rbtree->sentinel.key = 0xffffffff;
    qp_rbtree_set_black(qp_rbtree_nil(rbtree));
    rbtree->root = qp_rbtree_nil(rbtree);
}

/* left rotate */
void
qp_rbtree_left_rotate(qp_rbtree_t* rbtree, qp_rbtree_node_t* node)
{
    qp_rbtree_node_t* right = node->right;
    right->parent = qp_rbtree_parent(node);
    
    if (node == rbtree->root) {
        rbtree->root = right;
        
    } else {
        
        if (qp_rbtree_is_left(node)) {
            qp_rbtree_parent(node)->left = right;
        
        } else {
            qp_rbtree_parent(node)->right = right;
        }
    }
    
    node->parent = right;
    node->right = right->left;
    
    if (right->left != qp_rbtree_nil(rbtree)) {
        right->left->parent = node;
    }
    
    right->left = node;
}

/* right rotate */
void
qp_rbtree_right_rotate(qp_rbtree_t* rbtree, qp_rbtree_node_t* node)
{
    /* get left subtree */
    qp_rbtree_node_t* left = node->left;
    
    /* change the left subtree`s parent and the parent`s child */
    left->parent = qp_rbtree_parent(node);
    
    if (node == rbtree->root) {
        rbtree->root = left;
        
    } else {
        
        if (qp_rbtree_is_left(node)) {
            node->parent->left = left;
        
        } else {
            node->parent->right = left;
        }
    }
    
    /* change the node`s parent and left subtree */
    node->parent = left;
    node->left = left->right;
    
    if (left->right != qp_rbtree_nil(rbtree)) {
        left->right->parent = node;
    }
    
    /* change the left subtree`s right subtree */
    left->right = node;
}

/* insert fix */
void
qp_rbtree_insert_fix(qp_rbtree_t* rbtree, qp_rbtree_node_t* node)
{
    while (node != rbtree->root && qp_rbtree_is_red(qp_rbtree_parent(node))) {
        
        /* if uncle node is red, change parent and uncle to black and set
        * grandpa as red.
        */
        if (qp_rbtree_is_red(qp_rbtree_uncle(node))) {
            qp_rbtree_set_black(qp_rbtree_parent(node));
            qp_rbtree_set_black(qp_rbtree_uncle(node));
            qp_rbtree_set_red(qp_rbtree_grandpa(node));
            node = qp_rbtree_grandpa(node);
            continue;
        
        } else {
            
            /* if parent is left */
            if (qp_rbtree_is_left(qp_rbtree_parent(node))) {
                
                /* if node is right, left rotate */
                if (qp_rbtree_is_right(node)) {
                    node = qp_rbtree_parent(node);
                    qp_rbtree_left_rotate(rbtree, node);
                }
                
                qp_rbtree_set_black(qp_rbtree_parent(node));
                qp_rbtree_set_red(qp_rbtree_grandpa(node));
                qp_rbtree_right_rotate(rbtree, qp_rbtree_grandpa(node));
                
            } else {
                /*parent is right*/
                
                /* if node is left, right rotate */
                if (qp_rbtree_is_left(node)) {
                    node = qp_rbtree_parent(node);
                    qp_rbtree_right_rotate(rbtree, node);
                }
                
                qp_rbtree_set_black(qp_rbtree_parent(node));
                qp_rbtree_set_red(qp_rbtree_grandpa(node));
                qp_rbtree_left_rotate(rbtree, qp_rbtree_grandpa(node));
            }
        }
    }
    
    qp_rbtree_set_black(rbtree->root);
}

/* delete fix */
void
qp_rbtree_delete_fix(qp_rbtree_t* rbtree, qp_rbtree_node_t* node)
{
    while (node != rbtree->root && qp_rbtree_is_black(node)) {
        
        /* if node is left subtree */
        if (qp_rbtree_is_left(node)) {
            
            /* if brother node is red, change brother to black and change 
             * parent to red */
            if (qp_rbtree_is_red(qp_rbtree_parent(node)->right)) {
                qp_rbtree_set_black(qp_rbtree_parent(node)->right);
                qp_rbtree_set_red(qp_rbtree_parent(node));
                qp_rbtree_left_rotate(rbtree, qp_rbtree_parent(node));
            }
            
            /* if brother`s both child is black,change brother to red and 
             * change node to parent */
            if (qp_rbtree_is_black(qp_rbtree_parent(node)->right->left)
                || qp_rbtree_is_black(qp_rbtree_parent(node)->right->right)) 
            {
                qp_rbtree_set_red(qp_rbtree_parent(node)->right);
                node = node->parent;
                
            } else {
                
                /* if right child of brother */
                if (qp_rbtree_is_black(qp_rbtree_parent(node)->right->right)) {
                    qp_rbtree_set_black(qp_rbtree_parent(node)->right->left);
                    qp_rbtree_set_red(qp_rbtree_parent(node)->right);
                    qp_rbtree_right_rotate(rbtree, qp_rbtree_parent(node)->right);
                    
                }
                
                qp_rbtree_is_black(qp_rbtree_parent(node)) ? \
                qp_rbtree_set_black(qp_rbtree_parent(node)->right) : 
                qp_rbtree_set_red(qp_rbtree_parent(node)->right);
                qp_rbtree_set_black(qp_rbtree_parent(node));
                qp_rbtree_set_black(qp_rbtree_parent(node)->right->right);
                qp_rbtree_left_rotate(rbtree, qp_rbtree_parent(node));
                node = rbtree->root;
            }
            
        } else {
            
            if (qp_rbtree_is_red(qp_rbtree_parent(node)->left)) {
                qp_rbtree_set_black(qp_rbtree_parent(node)->left);
                qp_rbtree_set_red(qp_rbtree_parent(node));
                qp_rbtree_right_rotate(rbtree, qp_rbtree_parent(node));
            }
            
            /* if brother`s both child is black,change brother to red and 
             * change node to parent */
            if (qp_rbtree_is_black(qp_rbtree_parent(node)->left->left)
                || qp_rbtree_is_black(qp_rbtree_parent(node)->left->right)) 
            {
                qp_rbtree_set_red(qp_rbtree_parent(node)->left);
                node = node->parent;
                
            } else {
                
                if (qp_rbtree_is_black(qp_rbtree_parent(node)->left->left)) {
                    qp_rbtree_set_black(qp_rbtree_parent(node)->left->right);
                    qp_rbtree_set_red(qp_rbtree_parent(node)->left);
                    qp_rbtree_right_rotate(rbtree, qp_rbtree_parent(node)->left);
                }
                
                qp_rbtree_is_black(qp_rbtree_parent(node)) ? \
                qp_rbtree_set_black(qp_rbtree_parent(node)->left) : 
                qp_rbtree_set_red(qp_rbtree_parent(node)->left);
                qp_rbtree_set_black(qp_rbtree_parent(node));
                qp_rbtree_set_black(qp_rbtree_parent(node)->left->left);
                qp_rbtree_left_rotate(rbtree, qp_rbtree_parent(node));
                node = rbtree->root;
            }
        }
    }
    
    qp_rbtree_set_black(node);
}

/* insert node into rbtree */
void 
qp_rbtree_insert(qp_rbtree_t* rbtree, qp_rbtree_node_t* node)
{
    qp_rbtree_node_t* cur = NULL;
    qp_rbtree_node_t* next = NULL;
    
    if (!node) {
        return;
    }
    
    /* if the tree is empty, just insert it */
    if (qp_rbtree_is_empty(rbtree)) {
        qp_rbtree_set_black(node);
        node->left = rbtree->root;
        node->right = rbtree->root;
        node->parent = rbtree->root;
        rbtree->root = node;
        return;
    }
    
    cur = rbtree->root;
    
    while (1) {
        /* save the no sentinel node */
        if (node->key == cur->key) {
            return;
        }
        
        next = node->key < cur->key ? cur->left : cur->right;
        
        if (next == qp_rbtree_nil(rbtree)) {
            break;
        }
        
        cur = next;
    }
    
    /* do insert */
    if (node->key < cur->key) {
        cur->left = node;
        
    } else {
        cur->right = node;
    }
    
    node->parent = cur;
    node->left = qp_rbtree_nil(rbtree);
    node->right = qp_rbtree_nil(rbtree);
    qp_rbtree_set_red(node);
    
    /* if node`s parent is red, fix insert */
    if (qp_rbtree_is_red(qp_rbtree_parent(node))) {
        qp_rbtree_insert_fix(rbtree, node);
    }
}

/* delete node from rbtree */
void
qp_rbtree_delete(qp_rbtree_t* rbtree, qp_rbtree_node_t* node)
{
    qp_rbtree_node_t* subst = NULL;
    qp_rbtree_node_t* tmp = NULL;
    
    if (!node || !rbtree) {
        return;
    }
    
    /* if node dose not have both childs */
    if ((qp_rbtree_nil(rbtree) == node->left)
       || (qp_rbtree_nil(rbtree) == node->right))
    {
        subst = node;
    
    } else {
        /* has both childs */
        subst = qp_rbtree_min(rbtree, node->right); 
    }
    
    if (subst->left != qp_rbtree_nil(rbtree)) {
        tmp = subst->left;
        
    } else {
        tmp = subst->right;
    }
    
    tmp->parent = qp_rbtree_parent(subst);
    
    /* if node is root,remove it */
    if (qp_rbtree_parent(subst) == qp_rbtree_nil(rbtree)) {
        rbtree->root = tmp;
        qp_rbtree_set_black(tmp);
        
        subst->parent = NULL;
        subst->left = NULL;
        subst->right = NULL;
        return;
        
    } 
    
    /* change child of subst`s parent to tmp */
    if (qp_rbtree_is_left(subst)) {
        qp_rbtree_parent(subst)->left = tmp;
        
    } else {
        qp_rbtree_parent(subst)->right = tmp;
    }
    
    /*
     *           o -> node
     *          / \
     *         o   o
     *        /   /
     *       o   o -> subst
     *            \
     *             o -> tmp
     */
    
    /*  make tmp`parent point to node */
    if (node != subst) {
        node->data = subst->data;
        node->key = subst->key;
        
        subst->parent = NULL;
        subst->left = NULL;
        subst->right = NULL;
    }
    
    if (qp_rbtree_is_black(subst)) {
        qp_rbtree_delete_fix(rbtree, tmp);
    }
}

/* find node with key in rbtree */
qp_rbtree_node_t*
qp_rbtree_find(qp_rbtree_t* rbtree, qp_uint32_t key)
{
    qp_rbtree_node_t* node = rbtree->root;
    
    while ((node != qp_rbtree_nil(rbtree)) && (key != node->key)) {
        
        if (key < node->key) {
            node = node->left;
            
        } else {
            node = node->right;
        }
    }
    
    return (qp_rbtree_nil(rbtree) == node) ? NULL : node;
}

/* min node from [node] */
inline qp_rbtree_node_t*
qp_rbtree_min(qp_rbtree_t* rbtree, qp_rbtree_node_t* node)
{
    node = node ? node : rbtree->root;
    
    while (node->left != qp_rbtree_nil(rbtree)) {
        node = node->left;
    }
    
    return node;
}

/* max node from [node] */
inline qp_rbtree_node_t*
qp_rbtree_max(qp_rbtree_t* rbtree, qp_rbtree_node_t* node) 
{
    node = node ? node : rbtree->root;
    
    while (node->right != qp_rbtree_nil(rbtree)) {
        node = node->right;
    }
    
    return node;
}
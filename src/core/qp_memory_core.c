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


#include "qp_atomic.h"
#include "qp_memory_core.h"


#if defined(QP_OS_LINUX)  || defined(QP_OS_BSD) || defined(QP_OS_SOLARIS)


void*
qp_alloc_align(size_t alignment, size_t size)
{
# if defined(QP_OS_LINUX)  || defined(QP_OS_BSD)
    void *memptr = NULL;
    return (0 == posix_memalign(&memptr, alignment, size)) ? memptr : NULL;
# else
    return memalign(alignment, size);
# endif
}

#endif

void
qp_rbtree_init(qp_rbtree_t rbtree)
{
    rbtree->sentinel.left = qp_rbtree_nil(rbtree);
    rbtree->sentinel.right = qp_rbtree_nil(rbtree);
    rbtree->sentinel.parent = qp_rbtree_nil(rbtree);
    qp_rbtree_set_black(qp_rbtree_nil(rbtree));
    rbtree->root = qp_rbtree_nil(rbtree);
}

void
qp_rbtree_left_rotate(qp_rbtree_t rbtree, qp_rbtree_node_t node)
{
    qp_rbtree_node_t right = node->right;
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
qp_rbtree_right_rotate(qp_rbtree_t rbtree, qp_rbtree_node_t node)
{
    /* get left subtree */
    qp_rbtree_node_t left = node->left;
    
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
qp_rbtree_insert_fix(qp_rbtree_t rbtree, qp_rbtree_node_t node)
{
    while (node != rbtree->root && qp_rbtree_is_red(qp_rbtree_parent(node))) {
        
       /* 
        * if uncle node is red, change parent and uncle to black and set
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
qp_rbtree_delete_fix(qp_rbtree_t rbtree, qp_rbtree_node_t node)
{
    while (node != rbtree->root && qp_rbtree_is_black(node)) {
            
        /* if node is left subtree */
        if (qp_rbtree_is_left(node)) {
            
            /* if brother node is red, change brother to black and change 
             * parent to red */
            if (qp_rbtree_is_red(qp_rbtree_brother(node))) {
                qp_rbtree_set_black(qp_rbtree_brother(node));
                qp_rbtree_set_red(qp_rbtree_parent(node));
                qp_rbtree_left_rotate(rbtree, qp_rbtree_parent(node));
            }
            
            /* if brother`s both child is black,change brother to red and 
             * change node to parent */
            if (qp_rbtree_is_black(qp_rbtree_brother(node)->left) 
                && qp_rbtree_is_black(qp_rbtree_brother(node)->right))
            {
                qp_rbtree_set_red(qp_rbtree_brother(node));
                node = qp_rbtree_parent(node);
                
            } else {
                
                /* if right child of brother */
                if (qp_rbtree_is_black(qp_rbtree_brother(node)->right)) {
                    qp_rbtree_set_black(qp_rbtree_brother(node)->left);
                    qp_rbtree_set_red(qp_rbtree_brother(node));
                    qp_rbtree_right_rotate(rbtree, qp_rbtree_brother(node));
                }
                
                qp_rbtree_is_black(qp_rbtree_parent(node)) ? \
                    qp_rbtree_set_black(qp_rbtree_brother(node)) : \
                    qp_rbtree_set_red(qp_rbtree_brother(node));
                
                qp_rbtree_set_black(qp_rbtree_parent(node));
                qp_rbtree_set_black(qp_rbtree_brother(node)->right);
                qp_rbtree_left_rotate(rbtree, qp_rbtree_parent(node));
                node = rbtree->root;
            }
            
        } else {
            
            if (qp_rbtree_is_red(qp_rbtree_brother(node))) {
                qp_rbtree_set_black(qp_rbtree_brother(node));
                qp_rbtree_set_red(qp_rbtree_parent(node));
                qp_rbtree_right_rotate(rbtree, qp_rbtree_parent(node));
            }
            
            /* if brother`s both child is black,change brother to red and 
             * change node to parent */
            if (qp_rbtree_is_black(qp_rbtree_brother(node)->right) 
                && qp_rbtree_is_black(qp_rbtree_brother(node)->left))
            {
                qp_rbtree_set_red(qp_rbtree_brother(node));
                node = qp_rbtree_parent(node);
                
            } else {
                
                if (qp_rbtree_is_black(qp_rbtree_brother(node)->left)) {
                    qp_rbtree_set_black(qp_rbtree_brother(node)->right);
                    qp_rbtree_set_red(qp_rbtree_brother(node));
                    qp_rbtree_left_rotate(rbtree, qp_rbtree_brother(node));
                }
                
                qp_rbtree_is_black(qp_rbtree_parent(node)) ? \
                    qp_rbtree_set_black(qp_rbtree_brother(node)) : \
                    qp_rbtree_set_red(qp_rbtree_brother(node));
                
                qp_rbtree_set_black(qp_rbtree_parent(node));
                qp_rbtree_set_black(qp_rbtree_brother(node)->left);
                qp_rbtree_right_rotate(rbtree, qp_rbtree_parent(node));
                node = rbtree->root;
            }
        }
    }
    
    qp_rbtree_set_black(node);
}


/* insert node into rbtree */
qp_rbtree_node_t
qp_rbtree_insert(qp_rbtree_t rbtree, qp_rbtree_node_t node)
{
    qp_rbtree_node_t cur = NULL;
    qp_rbtree_node_t next = NULL;
    
    if (!node) {
        return NULL;
    }
    
    /* if the tree is empty, just insert it */
    if (qp_rbtree_is_empty(rbtree)) {
        qp_rbtree_set_black(node);
        node->left = rbtree->root;
        node->right = rbtree->root;
        node->parent = rbtree->root;
        rbtree->root = node;
        return node;
    }
    
    cur = rbtree->root;
    
    while (1) {
        /* save the no sentinel node */
        if (node->key == cur->key) {
            return NULL;
        }
        
        next = node->key < cur->key ? cur->left : cur->right;
        
        if (next == qp_rbtree_nil(rbtree)) {
            break;
        }
        
        cur = next;
    }
    
    /* do insert */
    if (node->key < cur->key) {
        node->left = cur->left;
        node->right = qp_rbtree_nil(rbtree);
        cur->left = node;
        
    } else {
        node->right = cur->right;
        node->left = qp_rbtree_nil(rbtree);
        cur->right = node;
    }
    
    node->parent = cur;
    qp_rbtree_set_red(node);
    
    /* if node`s parent is red, fix insert */
    if (qp_rbtree_is_red(qp_rbtree_parent(node))) {
        qp_rbtree_insert_fix(rbtree, node);
    }
    
    return node;
}


/* delete node from rbtree */
qp_rbtree_node_t
qp_rbtree_delete(qp_rbtree_t rbtree, qp_rbtree_node_t node)
{
    qp_rbtree_node_t subst = NULL;
    qp_rbtree_node_t tmp = NULL;
    
    if (!node || !rbtree || qp_rbtree_is_empty(rbtree)) {
        return NULL;
    }
    
    /* if node dose not has both child */
    if (qp_rbtree_nil(rbtree) == node->left 
        || qp_rbtree_nil(rbtree) == node->right) 
    {
        subst = node;
            
    } else {
        /* has both child */
        subst = qp_rbtree_min(rbtree, node->right);
    }
    
    if (qp_rbtree_nil(rbtree) != subst->left) {
        tmp = subst->left;
                
    } else {
        tmp = subst->right;
    }
    
    tmp->parent = qp_rbtree_parent(subst);
    
    /* if node is root,remove it */
    if (qp_rbtree_nil(rbtree) == qp_rbtree_parent(subst)) {
        rbtree->root = tmp;
        
    } else {
        
        /* change child of subst`s parent to tmp */
        if (qp_rbtree_is_left(subst)) {
            qp_rbtree_parent(subst)->left = tmp;
            
        } else {
            qp_rbtree_parent(subst)->right = tmp;
        }
    }
    
    if (node != subst) {
        node->key = subst->key;
        node->data = subst->data;
    }
    
    if (qp_rbtree_is_black(subst)) {
        qp_rbtree_delete_fix(rbtree, tmp);
    }
 
    return subst;
}


/* find node with key in rbtree */
qp_rbtree_node_t
qp_rbtree_find(qp_rbtree_t rbtree, qp_uint32_t key)
{
    qp_rbtree_node_t node = rbtree->root;
    
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
qp_rbtree_node_t
qp_rbtree_min(qp_rbtree_t rbtree, qp_rbtree_node_t node)
{
    if (qp_rbtree_is_empty(rbtree)) {
        return NULL;
    }
    
    node = node ? node : rbtree->root;
    
    while (node->left != qp_rbtree_nil(rbtree)) {
        node = node->left;
    }
    
    return node;
}


/* max node from [node] */
qp_rbtree_node_t
qp_rbtree_max(qp_rbtree_t rbtree, qp_rbtree_node_t node) 
{
    if (qp_rbtree_is_empty(rbtree)) {
        return NULL;
    }
    
    node = node ? node : rbtree->root;
    
    while (node->right != qp_rbtree_nil(rbtree)) {
        node = node->right;
    }
    
    return node;
}

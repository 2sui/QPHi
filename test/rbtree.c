/**
 * lock.c
 * 
 * Test for qpCore module.
 */

#include <qp_core.h>

void
looping(qp_rbtree_t* rbtree, qp_rbtree_node_t* root)
{
    if (root == &(rbtree->sentinel)) {
        return;
    }
    
    if (root == rbtree->root) {
        fprintf(stderr, "\nRoot is %d with %s.", root->key, 
        qp_rbtree_is_red(root) ? "red" : "black");
        
    } else {
        fprintf(stderr, "\n%d is %d`s %s child , with %s.", root->key, 
        root->parent->key, qp_rbtree_is_left(root) ? "left" : "right",
        qp_rbtree_is_red(root) ? "red" : "black");
        
        if (root->left == &rbtree->sentinel
            && root->right == &rbtree->sentinel) 
        {
            fprintf(stderr, "It is leaf node.");
        }
    } 
    
    looping(rbtree, root->left);
    looping(rbtree, root->right);
}


int
main(int argc, char** argv) 
{
    qp_rbtree_t   rbtree;
    qp_uint32_t   data[20] = {12, 1, 9, 2, 0, 11, 7, 19, 4, 15, 18, 5, 14, 13,
                             10, 16, 6, 3, 8, 17};
    qp_rbtree_node_t  node[20];
    
    int i = 0;
    for (; i < 20; i++) {
        node[i].key = data[i];
    }
    
    qp_rbtree_init(&rbtree);
    
    fprintf(stderr, "\n#### Inserting ####");
    for (i = 0; i < 20; i++) {
        qp_rbtree_insert(&rbtree, &node[i]);
        fprintf(stderr, "\ninsert %d", node[i].key);
    }
    
    fprintf(stderr, "\n Max is %d, Min is %d", qp_rbtree_max(&rbtree, NULL)->key,
        qp_rbtree_min(&rbtree, NULL)->key);
    looping(&rbtree, rbtree.root);
    
    
    fprintf(stderr, "\n#### Deleting ####");
    for (i = 0; i < 20; i++) {
        qp_rbtree_node_t* nod = qp_rbtree_find(&rbtree, data[i]);
        fprintf(stderr, "\n-------》 delete %d", nod->key);
        qp_rbtree_delete(&rbtree, nod);
        looping(&rbtree, rbtree.root);
        fprintf(stderr, "\n");
    }
    
    
    return 0;
}
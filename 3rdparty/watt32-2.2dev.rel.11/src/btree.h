/*!\file btree.h
 *\note <b>Not used</b>
 */
#ifndef _w32_BTREE_H
#define _w32_BTREE_H

typedef struct TreeNode {
        struct TreeNode *left;
        struct TreeNode *right;
        struct TreeNode *parent;
        void            *info;
        size_t           info_size;
      } TreeNode;

typedef enum TreeChild {
        NOT_CHILD  = 1,
        LEFT_CHILD,
        RIGHT_CHILD
      } TreeChild;

int       tree_insert (TreeNode **, const void *, size_t, CmpFunc);
TreeNode *tree_find   (TreeNode *, const void *, CmpFunc);
TreeNode *tree_delete (TreeNode *, TreeNode *);
void      tree_free   (TreeNode *);

#endif

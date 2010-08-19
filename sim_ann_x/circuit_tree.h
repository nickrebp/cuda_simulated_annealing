
#ifndef __CIRCUIT_TREE_H__
#define __CIRCUIT_TREE_H__

#include "red_black_tree.h"

#define TOP_LEVEL 0
#define SECOND_LEVEL 1
#define PRINT_CELLS 0
#define PRINT_NODES 1

#define MIN 0
#define MAX 2

typedef struct circ_tree {
  //main_tree is the main data structure holding all the references
  //y_bound_tree is a quick access tree to determine the boundaries in the secondary axis
  rb_red_blk_tree *main_tree, *y_bound_tree;
} circ_tree;

circ_tree *create_circ_tree();
void print_tree(circ_tree *, int, unsigned int);
int add_block(circ_tree *tree, int x1, int y1, int x2, int y2, int add_amt);
int get_boundary(circ_tree *c_tree, int boundary);


#endif

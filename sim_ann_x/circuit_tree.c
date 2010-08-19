
#include "circuit_tree.h"

#include <stdlib.h>
#include <string.h>



typedef struct circ_tree_node {
  unsigned int type:1; 
  union state_tree { 
    //state (how many pieces overlap) or the other 
    //tree in the other dimension
    int state; // indicates how many pieces overlap
    rb_red_blk_tree *tree;
  } data;
  int pos; //we could also use float for this
} circ_tree_node;


void circ_destroy(void *node) { 
  circ_tree_node *c_node = (circ_tree_node *) node;
  if (c_node->type == TOP_LEVEL) //destroy the contained tree as well if top level
    RBTreeDestroy(c_node->data.tree);
  free(c_node);
}

int circ_compare(const void *node1, const void *node2) {
  if (node1 && node2) {
    int pos1 = ((circ_tree_node *) node1)->pos;
    int pos2 = ((circ_tree_node *) node2)->pos;
    return (pos1 > pos2) - (pos1 < pos2);
  } else {
    if (node1)
      return 1;
    if (node2)
      return -1;
    return 0;
  }
}

//return 1 on equals, else 0
int circ_node_equals(void *n1, void *n2) {
  if (n1 == NULL || n2 == NULL)
    return n1 == n2;
  circ_tree_node *node1 = (circ_tree_node *) n1, 
    *node2 = (circ_tree_node *) n2;

  return node1->pos == node2->pos && node1->data.state == node2->data.state;
}


#ifdef __TREE_TEST

int main(void) {
  int x1, x2, y1, y2, add_amt;
  circ_tree *tree = create_circ_tree();
  for (;;) {
    print_tree(tree, 10, PRINT_CELLS);
    print_tree(tree, 10, PRINT_NODES);
    printf("Boundaries min: (%d, %d), max: (%d, %d)\n", 
	   get_boundary(tree, TOP_LEVEL | MIN),
	   get_boundary(tree, SECOND_LEVEL | MIN),
	   get_boundary(tree, TOP_LEVEL | MAX),
	   get_boundary(tree, SECOND_LEVEL | MAX)); 
    puts("Enter the add amount and the coordinates of the cell");
    scanf("%d %d %d %d %d", &add_amt, &x1, &y1, &x2, &y2);
    if (x1 < 0)
      break;
    printf("Added block, %d cells collided.\n", add_block(tree, x1, y1, x2, y2, add_amt));

  }
}

#endif

//prints x by x view of the tree
//assumes the grid starts from 0, 0; if there are entries less
//than that, it will be ignored, possibly to yield weird results
void print_tree(circ_tree *c_tree, int x, unsigned int print_mode) {
  rb_red_blk_tree *tree = c_tree->main_tree;
  static int static_x = 0;
  static circ_tree_node query_node_low, query_node_high;
  static char *line, *header_line;
  if (static_x != x) {
    if (line != NULL)
      free(line);
    if (header_line != NULL)
      free(header_line);

    static_x = x;
    query_node_low.pos = 0; //start node
    query_node_high.pos = x;
    int size = sizeof(char) * (x + 3);
    line = (char *) malloc(size);
    header_line = (char *) malloc(size);

    header_line[0] = ' ';
    header_line[1] = ' ';
    int i;
    for (i = 0; i < x; i++)
      header_line[2 + i] = '0' + (i % 10);
    header_line[x + 2] = '\0';

  }
  line[1] = ' ';
  memset(line + 2, print_mode == PRINT_CELLS ? '0' : ' ', x);  //initial line is all zeros
  line[x + 2] = '\0';  //null char at the end 
  stk_stack *stack = RBEnumerate(tree, &query_node_low, &query_node_high);
  int line_n = 0;
  puts(header_line);

  int cont_print = 1;

  while (cont_print) {

    int node_pos;
    rb_red_blk_tree *node_tree;    
    if (cont_print = StackNotEmpty(stack)) {
      circ_tree_node *node = ((rb_red_blk_node *) StackPop(stack))->key;
      node_pos = node->pos;
      node_tree = node->data.tree;
    } else {
      node_pos = x;
      node_tree = c_tree->y_bound_tree;
    }

    for (; line_n < node_pos; line_n++) {  
      //print the lines until this point / rest of the lines
      line[0] = '0' + (line_n % 10);
      puts(line);
    }

    //set the new line
    stk_stack *stack2 = RBEnumerate(node_tree, &query_node_low, &query_node_high);
    char c = print_mode == PRINT_CELLS ? '0' : ' ';
    int row_n = 0;
    while (StackNotEmpty(stack2)) {
      circ_tree_node *node2 = ((rb_red_blk_node *) StackPop(stack2))->key;
      for (; row_n < node2->pos; row_n++)  //print the chars until this point
	line[row_n + 2] = c;
      if (print_mode == PRINT_CELLS)
	c = '0' + node2->data.state;
      else
	line[row_n++ + 2] = '0' + node2->data.state;
    }

    if (print_mode == PRINT_NODES && row_n == 0)
      line[1] = '0';


    for (; row_n < x; row_n++)  //print the rest of the chars
      line[row_n + 2] = c;
    free(stack2);

    if (print_mode == PRINT_NODES || !cont_print) {
      if (cont_print)
	line[0] = '0' + (line_n++ % 10);
      else 
	line[0] = '>';  //print a different indicator for the boundary tree
      puts(line);
      memset(line + 1, ' ', x + 1);
    }
  }
  
  free(stack);
}

void *circ_tree_node_copy(void *ptr) {
  if (!ptr)
    return NULL;
  void *cpy_ptr = malloc(sizeof(circ_tree_node));
  memcpy(cpy_ptr, ptr, sizeof(circ_tree_node));
  return cpy_ptr;
}

void dummy_fun(void *a) {

}

rb_red_blk_tree *create_circ_tree_raw() {
  return RBTreeCreate(circ_compare, circ_destroy, 
		      dummy_fun, dummy_fun, dummy_fun);
}

circ_tree *create_circ_tree() {
  circ_tree *c_tree = (circ_tree *) malloc(sizeof(circ_tree));
  c_tree->main_tree = create_circ_tree_raw();
  c_tree->y_bound_tree = create_circ_tree_raw();
  return c_tree;
}

//point 1 must be less than point 2
int add_block(circ_tree *c_tree, int x1, int y1, int x2, int y2, int add_amt) {
  //here we can either add one per block or the length in the other axis
  //I arbitrarily chose to put one
  add_block_1_axis(c_tree->y_bound_tree, y1, 0, y2, 0, SECOND_LEVEL, add_amt);
  return add_block_1_axis(c_tree->main_tree, x1, y1, x2, y2, TOP_LEVEL, add_amt);

}

rb_red_blk_node *get_node(rb_red_blk_tree *tree, int x, unsigned int type);

//returns the total number of collided cells
int add_block_1_axis(rb_red_blk_tree *tree, int x1, int y1, int x2, int y2, unsigned int type, int add_amount) {
  circ_tree_node *node_begin, *node_end;
  node_end = get_node(tree, x2, type)->key;  //the end strictly needs to be called before the beginning
  node_begin = get_node(tree, x1, type)->key;
  stk_stack *axis_range = RBEnumerate(tree, node_begin, node_end);

  rb_red_blk_node *rb_node, *rb_node_prev = NULL;
  int temp_collision = 0, collision = 0, prev_pos;

  for (;;) {
    //rb_node_prev = rb_node;
    rb_node = (rb_red_blk_node *) StackPop(axis_range);

    //if (rb_node_prev == NULL)
      rb_node_prev = TreePredecessor(tree, rb_node);
    
    circ_tree_node *node = (circ_tree_node *) rb_node->key;
    circ_tree_node *node_prev;

    if (rb_node_prev == NULL || rb_node_prev == tree->nil)
      node_prev = NULL;
    else
      node_prev = (circ_tree_node *) rb_node_prev->key;

    unsigned int stack_not_empty = StackNotEmpty(axis_range);

    //collision
    if (temp_collision) 
      //if temp collision is non-zero, by definition, node_prev
      //cannot be NULL
      collision += temp_collision * (node->pos - prev_pos);

    prev_pos = node->pos;

    if (type == TOP_LEVEL) {
      if (stack_not_empty) 
	temp_collision = add_block_1_axis(node->data.tree, y1, 0, y2, 0, SECOND_LEVEL, add_amount);
      if ((node_prev != NULL && 
	   !RBTreeCompare(node->data.tree, node_prev->data.tree, 
			  circ_node_equals)) ||
	  (node_prev == NULL && RBIsTreeEmpty(node->data.tree)))
	RBDelete(tree, rb_node);
      
    } else {
      if (stack_not_empty) {
	if (node->data.state > 0 && node->data.state > -add_amount) 
	  //if there is already a block here, and if there would still 
	  //be a block left, assess collision
	  temp_collision = add_amount;
	else
	  temp_collision = 0;
	node->data.state += add_amount;
      }

      //if both nodes are the same
      if ((node_prev != NULL && node_prev->data.state == node->data.state) ||
	  //or the previous node is null and this is zero
	  (node_prev == NULL && node->data.state == 0)) {
	RBDelete(tree, rb_node);
      }
    }

    if (!stack_not_empty)
      break;
  }
  StackDestroy(axis_range, dummy_fun);

  return collision;
}


//gets the node at that location, creates one if necessary
rb_red_blk_node *get_node(rb_red_blk_tree *tree, int x, unsigned int type) {
  circ_tree_node query_node;
  query_node.pos = x;
  rb_red_blk_node *node = RBExactQuery(tree, &query_node);
  if (!node) { 
    //construct a new tree and copy contents from the one less than this
    circ_tree_node *new_node = (circ_tree_node *) malloc(sizeof(circ_tree_node));
    new_node->type = type;
    new_node->pos = x;
    rb_red_blk_node *node_prev = RBLTEQuery(tree, &query_node);
    circ_tree_node *circ_node_prev;
    if (node_prev)
      circ_node_prev = (circ_tree_node *) node_prev->key;
    if (type == TOP_LEVEL) {
      if (node_prev) { //add the nodes only if the previous is not null
	new_node->data.tree = RBTreeCopy(circ_node_prev->data.tree, circ_tree_node_copy, dummy_fun); 
      } else {
	new_node->data.tree = create_circ_tree_raw();
      }
    } else {
      if (node_prev)
	new_node->data.state = circ_node_prev->data.state;
      else
	new_node->data.state = 0;
    }

    node = RBTreeInsert(tree, new_node, NULL);
  }
  return node;
}

//if there are no blocks in this, it will always return 0
int get_boundary(circ_tree *c_tree, int boundary) {
  rb_red_blk_tree *tree = (boundary & SECOND_LEVEL) ? 
    c_tree->y_bound_tree : c_tree->main_tree; 
  rb_red_blk_node *node = (boundary & MAX) ? 
    TreeFindGreatest(tree) : TreeFindLeast(tree);
  if (node == NULL)
    return 0;
  return ((circ_tree_node *) node->key)->pos;
}


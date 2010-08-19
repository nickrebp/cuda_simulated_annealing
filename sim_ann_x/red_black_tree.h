#ifdef DMALLOC
#include <dmalloc.h>
#endif
#include"misc.h"
#include"stack.h"

#define TREE_SIZE 64
#define TREE_PTR_LEN 6

/*  CONVENTIONS:  All data structures for red-black trees have the prefix */
/*                "rb_" to prevent name conflicts. */
/*                                                                      */
/*                Function names: Each word in a function name begins with */
/*                a capital letter.  An example funcntion name is  */
/*                CreateRedTree(a,b,c). Furthermore, each function name */
/*                should begin with a capital letter to easily distinguish */
/*                them from variables. */
/*                                                                     */
/*                Variable names: Each word in a variable name begins with */
/*                a capital letter EXCEPT the first letter of the variable */
/*                name.  For example, int newLongInt.  Global variables have */
/*                names beginning with "g".  An example of a global */
/*                variable name is gNewtonsConstant. */

/* comment out the line below to remove all the debugging assertion */
/* checks from the compiled code.  */
#define DEBUG_ASSERT 1

typedef struct rb_red_blk_node {
  void* key;
  void* info;
  int red:1; /* if red=0 then the node is black */
  unsigned int left:TREE_PTR_LEN;
  unsigned int right:TREE_PTR_LEN;
  unsigned int parent:TREE_PTR_LEN;
} rb_red_blk_node;

typedef struct rb_red_blk_node_mem {
  rb_red_blk_node node;
  unsigned int next_mem:TREE_PTR_LEN;
} rb_red_blk_node_mem;


/* Compare(a,b) should return 1 if *a > *b, -1 if *a < *b, and 0 otherwise */
/* Destroy(a) takes a pointer to whatever key might be and frees it accordingly */
typedef struct rb_red_blk_tree {
  /*
  int (*Compare)(const void* a, const void* b); 
  void (*DestroyKey)(void* a);
  void (*DestroyInfo)(void* a);
  void (*PrintKey)(const void* a);
  void (*PrintInfo)(void* a);
  */
  /*  A sentinel is used for root and for nil.  These sentinels are */
  /*  created when RBTreeCreate is caled.  root->left should always */
  /*  point to the node which is the root of the tree.  nil points to a */
  /*  node which should always be black but has aribtrary children and */
  /*  parent and no key or info.  The point of using these sentinels is so */
  /*  that the root and nil nodes do not require special cases in the code */
  rb_red_blk_node_mem node_mem[TREE_SIZE];
  rb_red_blk_node root;             
  rb_red_blk_node nil;
  unsigned int next_mem:TREE_PTR_LEN;
  unsigned int next_tree_mem:TREE_PTR_LEN;              
} rb_red_blk_tree;

rb_red_blk_tree tree_arr[TREE_SIZE];
unsigned int next_tree_mem;

rb_red_blk_tree* RBTreeCreate(
			      /*int  (*CompFunc)(const void*, const void*),
			     void (*DestFunc)(void*), 
			     void (*InfoDestFunc)(void*), 
			     void (*PrintFunc)(const void*),
			     void (*PrintInfo)(void*)*/);
rb_red_blk_node * RBTreeInsert(rb_red_blk_tree*, void* key, void* info);
void RBTreePrint(rb_red_blk_tree*);
void RBDelete(rb_red_blk_tree* , rb_red_blk_node* );
void RBTreeDestroy(rb_red_blk_tree*);
rb_red_blk_node* TreePredecessor(rb_red_blk_tree*,rb_red_blk_node*);
rb_red_blk_node* TreeSuccessor(rb_red_blk_tree*,rb_red_blk_node*);
rb_red_blk_node* RBExactQuery(rb_red_blk_tree*, void*);
rb_red_blk_node* RBLTEQuery(rb_red_blk_tree*, void*);
rb_red_blk_node *RBGTEQuery(rb_red_blk_tree*, void*);
stk_stack *RBEnumerate(rb_red_blk_tree*, void* , void*);
void NullFunction(void*);
char RBTreeCompare(rb_red_blk_tree *tree1, rb_red_blk_tree *tree2, int (Equals)(void *, void *));
rb_red_blk_tree *RBTreeCopy(rb_red_blk_tree *tree, void *(*KeyCopy)(void *),
			    void *(*InfoCopy)(void *));
rb_red_blk_node *TreeFindLeast(rb_red_blk_tree *tree);
rb_red_blk_node *TreeFindGreatest(rb_red_blk_tree *tree);
unsigned int RBIsTreeEmpty(rb_red_blk_tree *tree);


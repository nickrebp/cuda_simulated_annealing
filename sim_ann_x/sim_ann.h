

#ifndef SIM_ANN_H
#define SIM_ANN_H


typedef struct wire_node wire_node;
typedef struct block block;

typedef struct coor {
  int x, y;
} coor;

typedef struct wire {
  coor pos1, pos2;
  block *block1, *block2;
} wire;

struct wire_node {
  wire *wire;
  
  //if primary is true, the pos1 in the wire corresponds to this block
  unsigned int primary:1; 
  //terminates on null
  wire_node *next;
};

struct block {
  coor pos1, pos2;
  wire_node *wire_list;
  int id;
};

typedef struct circuit {
  block *blocks;
  int n_block;
  coor min_coor, max_coor; //TODO: there is a problem with this, consider changing this mechanism
} circuit;

long long int register_circuit(circuit *);

#endif

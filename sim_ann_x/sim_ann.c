
#include "sim_ann.h"
#include "circuit_tree.h"

#include <stdlib.h>
#include <math.h>

#define DEF_AREA_CONST 2
#define DEF_WL_CONST 1
#define DEF_OLAP_CONST 30

int add_bl(circ_tree *, block *);
int rem_bl(circ_tree *, block *);
int wire_len(wire *);
int move(block *, int, int);
int replace(block *bl1, block *bl2);
int rotate(block *, int);
int accept(int delta_cost, long long int temp);
int get_overlap_cost(int overlap);
int get_wire_length_cost(int wire_length);
int get_area_cost(int x1, int y1, int x2, int y2);


int area_const = DEF_AREA_CONST, 
  wl_const = DEF_WL_CONST, olap_const = DEF_OLAP_CONST,
  terminate_on_rep = 10, temp_stable_per_block = 200, 
  tot_move_type_mult = 7, move_mult = 5, rot_mult = 6;

double temp_mult = 0.95;

circ_tree *c_tree;
long long int cost;
circuit *circ;

int neg_cost_cnt, pos_cost_acc_cnt, tot_cnt;

//registers the circuit and returns the total cost
long long int register_circuit(circuit *c) {
  cost = 0;
  c_tree = create_circ_tree();
  circ = c;

  int i;
  for (i = 0; i < circ->n_block; i++) {
    fprintf(stderr, "%d %lld\n", i, cost);
    block *bl = &circ->blocks[i];
    cost += get_overlap_cost(add_bl(c_tree, bl));
    wire_node *w_node = bl->wire_list;

    while (w_node != NULL) {
      if (w_node->primary)
	cost += get_wire_length_cost(wire_len(w_node->wire));
      w_node = w_node->next;
    }
  }
  cost += get_area_cost(circ->min_coor.x, circ->min_coor.y, 
			circ->max_coor.x, circ->max_coor.y);

  return cost;
}

int get_overlap_cost(int overlap) {
  return olap_const * overlap;//* overlap;
}

int get_wire_length_cost(int wire_length) {
  return wl_const * wire_length;
}

int get_area_cost(int x1, int y1, int x2, int y2) {
  return area_const * (x2 - x1) * (y2 - y1);
}

//returns the number of cell intersected
int add_bl(circ_tree *tree, block *bl) {
  return add_block(tree, bl->pos1.x, bl->pos1.y, bl->pos1.x + bl->pos2.x, 
		   bl->pos1.y + bl->pos2.y, 1);
}

//returns the negative of the number of cells intersection removed
int rem_bl(circ_tree *tree, block *bl) {
  return add_block(tree, bl->pos1.x, bl->pos1.y, bl->pos1.x + bl->pos2.x, 
		  bl->pos1.y + bl->pos2.y, -1);
}


//returns the wire length (manhattan length)
int wire_len(wire *wire) {
  return abs(wire->pos1.x + wire->block1->pos1.x - 
	     (wire->pos2.x + wire->block2->pos1.x)) + 
    abs(wire->pos1.y + wire->block1->pos1.y - (wire->pos2.y + wire->block2->pos1.y));
}


void anneal() {
  long long int temp = 20000;//cost / circ->n_block / 300; //find the average cost per block
  long long int orig_temp = temp;

  long long int min_cost = cost;
  int cost_stable_cyc = 0;
  //int cyc = 0;
  int par = 1;

  //tot_move_type_mult = 7, move_mult = 5, rot_mult = 6

  while (cost_stable_cyc < terminate_on_rep) {  //how many cost repetitions are allowed before termination
    neg_cost_cnt = 0;
    pos_cost_acc_cnt = 0;
    tot_cnt = 0;
    int i;
    for (i = 0; i < circ->n_block * temp_stable_per_block; i++) {  //how many moves should be done in each temperature
      int block_id = (int) (((double) random() / RAND_MAX) * circ->n_block);
      int move_type = (int) (((double) random() / RAND_MAX) * tot_move_type_mult);
      if (move_type < move_mult) { //move the block somewhere
	double window_const = (double) temp / orig_temp;
	int window_x = (int) (window_const * (circ->max_coor.x - circ->min_coor.x));
	int window_y = (int) (window_const * (circ->max_coor.y - circ->min_coor.y));
	int move_x = (int) (((double) random() / RAND_MAX) * 2 * window_x) - window_x;
	int move_y = (int) (((double) random() / RAND_MAX) * 2 * window_y) - window_y;
	if (!move_x)
	  move_x += (par *= -1);
	if (!move_y)
	  move_y += (par *= -1);

	int delta_cost = move(&circ->blocks[block_id], move_x, move_y);
	if (accept(delta_cost, temp))
	  cost += delta_cost;
	else 
	  move(&circ->blocks[block_id], -move_x, -move_y);


      } else if (move_type < rot_mult) {  //rotate the block
	int rotate_deg = (int) (((double) random() / RAND_MAX) * 3);
	int delta_cost = rotate(&circ->blocks[block_id], rotate_deg);
	if (accept(delta_cost, temp))
	  cost += delta_cost;
	else
	  rotate(&circ->blocks[block_id], 2 - rotate_deg);
      } else {
	int block_id_2 = (int) (((double) random() / RAND_MAX) * circ->n_block);
	int delta_cost = replace(&circ->blocks[block_id], &circ->blocks[block_id_2]);
	  if (accept(delta_cost, temp))
	    cost += delta_cost;
	  else
	    replace(&circ->blocks[block_id], &circ->blocks[block_id_2]);

      }

    }

    fprintf(stderr, "Temp: %lld Cost: %lld T: %d -Cst: %d +CAcc: %d\n", temp, cost, tot_cnt, neg_cost_cnt, pos_cost_acc_cnt);


    temp *= temp_mult;

    if (cost >= min_cost)
      cost_stable_cyc++;
    else {
      cost_stable_cyc = 0;
      min_cost = cost;
    }


  }

}

int accept(int delta_cost, long long int temp) {
  tot_cnt++;
  if (delta_cost < 0) {
    neg_cost_cnt++;
    return 1;
  }
  //return 0;
  double ex = exp((double) -delta_cost/temp);
  int ret = ((double) random() / RAND_MAX) < exp((double) -delta_cost / temp);
  if (ret)
    pos_cost_acc_cnt++;
  return ret;
}

int get_wire_cost_for_block(block *bl) {
  wire_node *w_node = bl->wire_list;
  int delta_cost = 0;
  while (w_node != NULL) { 
    //IDEA: these wire recalculations in rotate operation can be done in parallel
    delta_cost += get_wire_length_cost(wire_len(w_node->wire));
    w_node = w_node->next;
  }
  return delta_cost;
}

int move(block *bl, int x, int y) {
  int delta_cost = -get_wire_cost_for_block(bl);

  delta_cost -= get_area_cost(get_boundary(c_tree, TOP_LEVEL | MIN),
			      get_boundary(c_tree, SECOND_LEVEL | MIN),
			      get_boundary(c_tree, TOP_LEVEL | MAX),
			      get_boundary(c_tree, SECOND_LEVEL | MAX));
  delta_cost -= get_overlap_cost(-rem_bl(c_tree, bl));
  
  bl->pos1.x += x;
  bl->pos1.y += y;
  delta_cost += get_overlap_cost(add_bl(c_tree, bl));
  delta_cost += get_area_cost(get_boundary(c_tree, TOP_LEVEL | MIN),
			      get_boundary(c_tree, SECOND_LEVEL | MIN),
			      get_boundary(c_tree, TOP_LEVEL | MAX),
			      get_boundary(c_tree, SECOND_LEVEL | MAX));
  
  delta_cost += get_wire_cost_for_block(bl);
  return delta_cost;
}

int replace(block *bl1, block *bl2) {
  int delta_cost = -get_wire_cost_for_block(bl1) - get_wire_cost_for_block(bl2);

  delta_cost -= get_area_cost(get_boundary(c_tree, TOP_LEVEL | MIN),
			      get_boundary(c_tree, SECOND_LEVEL | MIN),
			      get_boundary(c_tree, TOP_LEVEL | MAX),
			      get_boundary(c_tree, SECOND_LEVEL | MAX));
  delta_cost -= get_overlap_cost(-rem_bl(c_tree, bl1)) + 
    get_overlap_cost(-rem_bl(c_tree, bl2));
  

  coor temp_pos1 = bl1->pos1;
  bl1->pos1 = bl2->pos1;
  bl2->pos1 = temp_pos1;

  delta_cost += get_overlap_cost(add_bl(c_tree, bl1)) + 
    get_overlap_cost(add_bl(c_tree, bl2));
  delta_cost += get_area_cost(get_boundary(c_tree, TOP_LEVEL | MIN),
			      get_boundary(c_tree, SECOND_LEVEL | MIN),
			      get_boundary(c_tree, TOP_LEVEL | MAX),
			      get_boundary(c_tree, SECOND_LEVEL | MAX));
  
  delta_cost += get_wire_cost_for_block(bl1) + get_wire_cost_for_block(bl2);
  return delta_cost;
}

int rotate(block *bl, int deg) {
  deg = deg % 3;
  int delta_cost = -get_wire_cost_for_block(bl);
  if (deg != 1) { //unless the rotation is 180 degrees, the dimensions of the block neeed to change
    coor new_pos2;
    new_pos2.x = bl->pos2.y;
    new_pos2.y = bl->pos2.x;
    delta_cost -= get_area_cost(get_boundary(c_tree, TOP_LEVEL | MIN),
				get_boundary(c_tree, SECOND_LEVEL | MIN),
				get_boundary(c_tree, TOP_LEVEL | MAX),
				get_boundary(c_tree, SECOND_LEVEL | MAX));
    delta_cost -= get_overlap_cost(-rem_bl(c_tree, bl));
    

    bl->pos2 = new_pos2;
    delta_cost += get_overlap_cost(add_bl(c_tree, bl));
    delta_cost += get_area_cost(get_boundary(c_tree, TOP_LEVEL | MIN),
				get_boundary(c_tree, SECOND_LEVEL | MIN),
				get_boundary(c_tree, TOP_LEVEL | MAX),
				get_boundary(c_tree, SECOND_LEVEL | MAX));
  }

  wire_node *w_node = bl->wire_list;
  while (w_node != NULL) { 
    //IDEA: these wire recalculations in rotate operation can be done in parallel
    coor pos = w_node->primary ? w_node->wire->pos1 : w_node->wire->pos2;
    coor new_pos;
    
    switch (deg) {
    case 0:
      new_pos.x = bl->pos2.x - pos.y;
      new_pos.y = pos.x;
      break;
    case 1:
      new_pos.x = bl->pos2.x - pos.x;
      new_pos.y = bl->pos2.y - pos.y;
      break;
    case 2:
      new_pos.x = pos.y;
      new_pos.y = bl->pos2.y - pos.x;
    }

    if (w_node->primary)
      w_node->wire->pos1 = new_pos;
    else
      w_node->wire->pos2 = new_pos;
    
    delta_cost += get_wire_length_cost(wire_len(w_node->wire));

    w_node = w_node->next;
  }

  return delta_cost;
}


#ifndef __TREE_TEST

int main() {
  circ = load_circuit();
  //print_circuit(circ);

  //print_tree(circ_tree, 600, 0);
  //print_tree(circ_tree, 600, 1);

  fprintf(stderr, "Cost: %lld\n", register_circuit(circ));
  anneal();

  //print_tree(c_tree, 650,0);

  print_circuit(circ);

}

#endif

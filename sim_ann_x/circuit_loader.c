
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sim_ann.h"
#include "circuit_loader.h"

#define READ_BUF_SIZE 256


void *load_error(char *err);



circuit *load_circuit() {
  char read_buf[READ_BUF_SIZE];

  circuit *circ = NULL;
  wire *temp_wire = NULL;
  unsigned int coor_init = 0;

  for (;;) {
    fgets(read_buf, READ_BUF_SIZE, stdin);
    
    char *command = strtok(read_buf, " \t");
    
    if (command == NULL || *command == '\n')
      break;

    //we first expect a tot_module keyword    
    if (circ == NULL) {
      if (strcmp(command, "tot_module") == 0) { //matches
	circ = (circuit *) malloc(sizeof(circuit));
	circ->n_block = atoi(strtok(NULL, " \t"));
	circ->blocks = (block *) malloc(sizeof(block) * circ->n_block);
      } else {
	return load_error("the first keyword needs to be tot_module");
      }
    } else {
      if (strcmp(command, "module") == 0) {
	int block_id = atoi(strtok(NULL, " \t") + 1); //gets the XX in mXX
	block *bl = &circ->blocks[block_id];
	bl->pos1.x = atoi(strtok(NULL, " \t"));
	bl->pos1.y = atoi(strtok(NULL, " \t"));
	bl->pos2.x = atoi(strtok(NULL, " \t"));
	bl->pos2.y = atoi(strtok(NULL, " \t"));
      
	bl->wire_list = NULL;
	bl->id = block_id;

	//establish circuit max/min coordinates
	if (coor_init) {
	  if (circ->min_coor.x > bl->pos1.x)
	    circ->min_coor.x = bl->pos1.x;
	  if (circ->min_coor.y > bl->pos1.y)
	    circ->min_coor.y = bl->pos1.y;
	  if (circ->max_coor.x < bl->pos1.x + bl->pos2.x)
	    circ->max_coor.x = bl->pos1.x + bl->pos2.x;
	  if (circ->max_coor.y < bl->pos1.y + bl->pos2.y) 
	    circ->max_coor.y = bl->pos1.y + bl->pos2.y;
	} else {
	  circ->min_coor = bl->pos1;
	  circ->max_coor.x = bl->pos1.x + bl->pos2.x;
	  circ->max_coor.y = bl->pos1.y + bl->pos2.y;
	  coor_init = 1;
	}
      } else if (strcmp(command, "terminal") == 0) {
	wire_node *w_node = (wire_node *) malloc(sizeof(wire_node));
	strtok(NULL, " \t"); //scrap the nXX
	int block_id = atoi(strtok(NULL, " \t") + 1);
	w_node->next = circ->blocks[block_id].wire_list;
	circ->blocks[block_id].wire_list = w_node;
	int x = atoi(strtok(NULL, " \t")),
	  y = atoi(strtok(NULL, " \t"));

	if (temp_wire == NULL) {
	  w_node->primary = 1;
	  w_node->wire = temp_wire = (wire *) malloc(sizeof(wire));
	  temp_wire->pos1.x = x;
	  temp_wire->pos1.y = y;
	  temp_wire->block1 = &circ->blocks[block_id];
	} else {
	  w_node->primary = 0;
	  w_node->wire = temp_wire;
	  temp_wire->pos2.x = x;
	  temp_wire->pos2.y = y;
	  temp_wire->block2 = &circ->blocks[block_id];
	  temp_wire = NULL;
	}

      } else if (strcmp(command, "net") == 0) {
	//do nothing: we already did everything in terminal part

      } else {
	return load_error("unrecognized command");
      }
    }
  }
  return circ;
}

void *load_error(char *err) {

  puts("load error!");
  puts(err);
  return NULL;
}

void print_circuit(circuit *circ) {
  printf("tot_module %d\n", circ->n_block);
  int i;
  for (i = 0; i < circ->n_block; i++) {
    block *bl = &circ->blocks[i];
    printf("module m%d %d %d %d %d\n", i, bl->pos1.x, bl->pos1.y, 
	   bl->pos2.x, bl->pos2.y);
  }

  int net_id = 0;
  for (i = 0; i < circ->n_block; i++) {
    block *bl = &circ->blocks[i];
    wire_node *w_node = bl->wire_list;
    while (w_node) {
      if (w_node->primary) {
	wire *w = w_node->wire;
	printf("terminal n%d m%d %d %d\n", net_id, w->block1->id, w->pos1.x, w->pos1.y);
	printf("terminal n%d m%d %d %d\n", net_id, w->block2->id, w->pos2.x, w->pos2.y);
	printf("net n%d m%d n%d m%d n%d\n", net_id, w->block1->id, net_id, w->block2->id, net_id);
	net_id++;
      }
      w_node = w_node->next;
    }
  }
  puts("");
}

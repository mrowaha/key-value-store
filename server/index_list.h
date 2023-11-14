#include<stddef.h>
#include<stdbool.h>

/**
 * node struct for the sorted linked list index
 * @offset: index of the node in the linked list
 * @next: next node in the list
 * @key: value of the node
*/
typedef struct index_node {
  int offset;
  struct index_node* next;
  int key;
} index_node;

/**
 * list struct for the index
 * @size: size of the linked list
 * @head: head node of the linked list
*/
typedef struct {
  int size;
  index_node* head;
} index_list;


index_list* new_indexlist(void);

/**
 * loads an index based on the datastore keys
 * @filename: filename of the data file
 * @blocksize: size of one dataitem in the datastore
*/
void load_bin(const char* filename, const size_t blocksize);

/**
 * insert key to the index
 * @return false if the key already exists else returns true
*/
bool insert_key(index_list* indexlist, const int key);

void print_indexlist(const index_list*);

void free_indexlist(index_list*);
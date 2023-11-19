#ifndef INDEX_LIST_0x123
#define INDEX_LIST_0x123

#include <stddef.h>
#include <stdbool.h>

/**
 * element of the index list
 */
struct index_node;

/**
 * list struct for the index
 * @size: size of the linked list
 * @head: head node of the linked list
 */
typedef struct
{
  int size;
  struct index_node *head;
} index_list;

index_list *new_indexlist(void);

/**
 * loads an index based on the datastore keys
 * @filename: filename of the data file
 * @blocksize: size of one dataitem in the datastore
 * the caller must ensure that the dataitem structs must have the key as their first attribute
 * for example, struct dataitem { int key; int value; } is valid
 * but, struct dataitem {int value; int key; } is not valid
 */
bool load_bin(index_list *, const char *filename, const size_t blocksize);

/**
 * insert key to the index
 * @return false if the key already exists else returns true
 */
bool insert_key(index_list *indexlist, const int key, int *offset);

/**
 * get offset of the key
 * @return key offset or -1 if key does not exist
 */
int get_offset(index_list *indexlist, const int key);

void print_indexlist(const index_list *);

void free_indexlist(index_list *);

#endif
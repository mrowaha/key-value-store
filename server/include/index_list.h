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
 * insert key to the index
 * @return false if the key already exists else returns true
 */
bool insert_key(index_list *indexlist, const int key, int *offset);

bool delete_key(index_list *indexlist, const int key);

/**
 * get offset of the key
 * @return key offset or -1 if key does not exist
 */
int get_offset(index_list *indexlist, const int key);

void print_indexlist(const index_list *);

void free_indexlist(index_list *);

#endif
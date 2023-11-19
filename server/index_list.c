#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include "index_list.h"

/**
 * node struct for the sorted linked list index
 * @offset: index of the node in the linked list
 * @next: next node in the list
 * @key: value of the node
 */
typedef struct index_node
{
  int offset;
  struct index_node *next;
  int key;
} index_node;

index_node *new_indexnode(void)
{
  index_node *node = (index_node *)malloc(sizeof(index_node));
  node->next = NULL;
  node->offset = -1;
  node->key = -1;
  return node;
}

index_node *new_indexnode_with_key(const int key)
{
  index_node *node = new_indexnode();
  node->key = key;
  return node;
}

void print_indexnode(const index_node *node)
{
  int *ptr = (int *)node;
  unsigned int node_adr = (unsigned int)((uintptr_t)ptr & 0xFFFF);
  if (node->next != NULL)
  {
    ptr = (int *)node->next;
    unsigned int temp = (unsigned int)((uintptr_t)ptr & 0xFFFF);
    printf("{%x -> offset:%d, key:%d, next:%x}", node_adr, node->offset, node->key, temp);
  }
  else
  {
    printf("{%x -> offset:%d, key:%d, next:%p}", node_adr, node->offset, node->key, node->next);
  }
}

void free_indexnode(index_node *node)
{
  if (node->next)
  {
    free_indexnode(node->next);
  }
  free(node);
}

index_list *new_indexlist(void)
{
  index_list *indexlist = (index_list *)malloc(sizeof(index_list));
  indexlist->size = 0;
  indexlist->head = NULL;
  return indexlist;
}

bool exists_key(index_list *indexlist, const int key)
{
  if (indexlist->head == NULL)
    return false;
  index_node *curr = indexlist->head;
  while (curr != NULL)
  {
    if (curr->key == key)
      return true;
    curr = curr->next;
  }
  return false;
}

bool insert_key(index_list *indexlist, const int key, int *offset)
{
  if (exists_key(indexlist, key))
    return false;
  *offset = 0;
  index_node *new_node = new_indexnode_with_key(key);
  if (indexlist->head == NULL)
  {
    indexlist->head = new_node;
    indexlist->head->offset = *offset;
  }
  else
  {
    /* the index is sorted */
    index_node *curr = indexlist->head, *prev = NULL;
    while (curr != NULL && curr->key < key)
    {
      *offset = (*offset) + 1;
      prev = curr;
      curr = curr->next;
    }
    if (prev == NULL)
    {
      /* make new node the head node */
      new_node->next = curr;
      indexlist->head = new_node;
    }
    else if (curr == NULL)
    {
      /* insert into the end */
      prev->next = new_node;
      new_node->next = NULL;
    }
    else
    {
      /* insert somewhere in mid of the index */
      new_node->next = curr;
      prev->next = new_node;
    }
    new_node->offset = *offset;
  }
  indexlist->size++;
  return true;
}

void print_indexlist(const index_list *indexlist)
{
  printf("[\n");
  index_node *curr = indexlist->head;
  while (curr != NULL)
  {
    print_indexnode(curr);
    printf(",\n");
    curr = curr->next;
  }
  printf("]\n");
}

int get_offset(index_list *indexlist, const int key)
{
  if (indexlist->head == NULL)
    return -1;
  index_node *curr = indexlist->head;
  while (curr != NULL)
  {
    if (curr->key == key)
    {
      return curr->offset;
    }
  }
  return -1;
}

void free_indexlist(index_list *indexlist)
{
  if (indexlist->head)
  {
    free_indexnode(indexlist->head);
  }
  indexlist->size = 0;
  free(indexlist);
}
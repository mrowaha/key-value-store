#ifndef INDEX_BUILDER_H
#define INDEX_BUILDER_H

#include <stdbool.h>

#include "dataset_mng.h"
#include "index_list.h"

typedef struct index_builder
{
  dataset_mng *datasetmng;
  index_list **indexlists;
  int idxlistcount;
} index_builder;

index_builder *new_indexbuilder(dataset_mng *mng);

void free_indexbuilder(index_builder *);

void printb_indexlist(index_builder *, const unsigned int idxlist);

void print_all_index(index_builder *);

/**
 * return value of the index builder for the offset of a key
 * @offset: offset from the mmap
 * @dataset: id of the dataset file
 */
typedef struct
{
  int offset;
  int dataset;
} dataset_offset;

dataset_offset getb_offset(index_builder *, const int key, bool *status);

#endif
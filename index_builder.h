#ifndef INDEX_BUILDER_H
#define INDEX_BUILDER_H

#include"dataset_mng.h"
#include"index_list.h"

typedef struct index_builder {
  dataset_mng* datasetmng;
  index_list** indexlists;
  int idxlistcount;
} index_builder;

index_builder* new_index_builder(dataset_mng* mng);

void free_index_builder(index_builder*);

void print_index_list(index_builder*, const int idxlist);

void print_all_index(index_builder*);

#endif
#include<stdlib.h>
#include<stdio.h>
#include"index_builder.h"

/* build indexes on top of datasets */
void make_indexes(index_builder* idxbuilder);

index_builder* new_index_builder(dataset_mng* mng) {
  // there will be as many index lists as the dcount
  if (mng == NULL) {
    fprintf(stderr, "new_index_builder: dataset manager must be initialized");
  }
  
  index_builder* new_idx_builder = (index_builder*)malloc(sizeof(index_builder));
  new_idx_builder->datasetmng = mng;
  new_idx_builder->idxlistcount = mng->dcount;
  new_idx_builder->indexlists = (index_list**)malloc(sizeof(index_list*) * new_idx_builder->idxlistcount);  
  make_indexes(new_idx_builder);
  return new_idx_builder;
}

void free_index_builder(index_builder* idxbuilder) {
  if (idxbuilder) {
    for (int i = 0; i < idxbuilder->idxlistcount; i++) {
      free_indexlist(idxbuilder->indexlists[i]);
    } 
    free(idxbuilder->indexlists);
    free(idxbuilder);
  }
}

void make_indexes(index_builder* idxbuilder) {
  int* keys = NULL;
  int* offsets = NULL;

  printf("checkpoint 1\n");
  fflush(stdout);
  for (int i = 0; i < idxbuilder->idxlistcount; i++) {
    int mapcount = get_dataitem_offsets(
      idxbuilder->datasetmng,
      i+1 /* dataset number */,
      keys,
      offsets
    );

    printf("checkpoint 1.2\n");

    if (mapcount == -1) {
      fprintf(stderr, "make_index: getting dataitem offsets failed");
      if (keys != NULL) free(keys);
      if (offsets != NULL) free(offsets);
      continue;
    }
    
    printf("checkpoint 1.3\n");

    for (int j = 0; j < mapcount; j++) {
      printf("checpoint 1.3.1\n");
      idxbuilder->indexlists[i] = new_indexlist();
      printf("inserting key: %d, offset: %d\n", keys[j], offsets[j]);
      insert_key_offset(idxbuilder->indexlists[i], keys[j], offsets[j]);
    }

    printf("checkpoint 1.4\n");

    free(keys);
    free(offsets);
  }

  printf("checkpoint 2\n");
}
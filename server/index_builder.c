#include <stdlib.h>
#include <stdio.h>
#include "index_builder.h"

/* build indexes on top of datasets */
void make_indexes(index_builder *idxbuilder);

index_builder *new_indexbuilder(dataset_mng *mng)
{
  // there will be as many index lists as the dcount
  if (mng == NULL)
  {
    fprintf(stderr, "new_index_builder: dataset manager must be initialized");
  }

  index_builder *new_idx_builder = (index_builder *)malloc(sizeof(index_builder));
  new_idx_builder->datasetmng = mng;
  new_idx_builder->idxlistcount = mng->dcount;
  new_idx_builder->indexlists = (index_list **)malloc(sizeof(index_list *) * new_idx_builder->idxlistcount);
  for (int i = 0; i < new_idx_builder->idxlistcount; i++)
  {
    new_idx_builder->indexlists[i] = new_indexlist();
  }
  make_indexes(new_idx_builder);
  return new_idx_builder;
}

void free_indexbuilder(index_builder *idxbuilder)
{
  if (idxbuilder)
  {
    for (int i = 0; i < idxbuilder->idxlistcount; i++)
    {
      free_indexlist(idxbuilder->indexlists[i]);
    }
    free(idxbuilder->indexlists);
    free(idxbuilder);
  }
}

void make_indexes(index_builder *idxbuilder)
{

  dataset_mng *mng = idxbuilder->datasetmng;
  index_list **indexes = idxbuilder->indexlists;

  for (int i = 0; i < idxbuilder->idxlistcount; i++)
  {
    get_dataset_idx(mng, i + 1 /* dataset */, indexes[i]);
  }
}

void print_all_index(index_builder *idxbuilder)
{
  printf("************************\n");
  printf("*****IN-MEM INDEXES*****\n");
  printf("************************\n");

  for (int i = 0; i < idxbuilder->idxlistcount; i++)
  {
    print_indexlist(idxbuilder->indexlists[i]);
    printf("************************\n");
  }
}

void printb_indexlist(index_builder *idxbuilder, const unsigned int idxlist)
{

  if (idxlist >= idxbuilder->idxlistcount)
  {
    fprintf(stderr, "print_index_list: idxlist cannot be greater than index list count\n");
    return;
  }

  print_indexlist(idxbuilder->indexlists[idxlist]);
}

dataset_offset getb_offset(index_builder *idxbuilder, const int key, bool *status)
{

  int indexlist_idx = (key % idxbuilder->idxlistcount);
  int offset = get_offset(idxbuilder->indexlists[indexlist_idx], key);
  *status = offset != -1;
  dataset_offset returnval;
  returnval.offset = offset;
  returnval.dataset = indexlist_idx + 1;
  return returnval;
}
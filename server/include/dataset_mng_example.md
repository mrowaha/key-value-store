## This example demonstrates writing to a dataset and retrieving an index list for that dataset.

1. write data to the dataset
2. build index list for the dataset
3. retrieve values by offset

```
int main(const int argc, const char *argv[])
{
  cmd_args *args = new_cmdargs();
  if (!validate_cmdargs(args, argc, argv, true))
  {
    return EXIT_FAILURE;
  }
#ifdef DEBUG
  print_cmdargs(args);
#endif

  dataset_mng *dbmng = new_datasetmng(
      args->fname,
      args->dcount,
      args->vsize);
  int dataset;
  write_dataitem(dbmng, 20, "THIS23awawra", &dataset);
  write_dataitem(dbmng, 25, "THIS23asfasfsad", &dataset);
  write_dataitem(dbmng, 30, "THIS23swfaf", &dataset);
  print_dataset(dbmng, dataset);

  index_list *idxlist = new_indexlist();
  get_dataset_idx(dbmng, dataset, idxlist);
  print_indexlist(idxlist);

  char value[args->vsize];
  int key = 0;
  get_value_by_offset(dbmng, dataset, 0, value, &key);
  printf(" %d : %s (vsize %ld)\n", key, value, sizeof(value));

  free_indexlist(idxlist);
  free_datasetmng(dbmng);
  free_cmdargs(args);

  return EXIT_SUCCESS;

```
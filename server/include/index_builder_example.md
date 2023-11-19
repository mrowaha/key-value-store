## This example demonstrates how index builder builds around the dataset manager and retrieving values by offsets

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
  write_dataitem(dbmng, 20, "THIS23", &dataset);
  write_dataitem(dbmng, 25, "THIS23asfasfsad", &dataset);
  write_dataitem(dbmng, 30, "THIS23swfaf", &dataset);
  print_dataset(dbmng, dataset);

  write_dataitem(dbmng, 21, "THIS23", &dataset);
  write_dataitem(dbmng, 26, "THIS23asfasfsad", &dataset);
  write_dataitem(dbmng, 31, "THIS23swfaf", &dataset);
  print_dataset(dbmng, dataset);

  index_builder *idxbuilder = new_indexbuilder(dbmng);
  bool status = true;
  dataset_offset offset = getb_offset(idxbuilder, 22, &status);
  if (!status)
  {
    printf("key does not exist\n");
  }
  else
  {
    char value[args->vsize];
    int key = -1;
    get_value_by_offset(dbmng, offset.dataset, offset.offset, value, &key);
    printf("{ %d : %s (vsize : %ld) }\n", key, value, sizeof(value));
  }

  free_indexbuilder(idxbuilder);
  free_datasetmng(dbmng);
  free_cmdargs(args);

  return EXIT_SUCCESS;
}
```
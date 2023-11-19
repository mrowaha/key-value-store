## This example extends the dataset_mng example with inline dataset and index updates to values and insertions

```
int main(const int argc, const char *argv[])
{
  args = new_cmdargs();
  if (!validate_cmdargs(args, argc, argv, true))
  {
    return EXIT_FAILURE;
  }
#ifdef DEBUG
  print_cmdargs(args);
#endif

  // signal(SIGTERM, signal_handler);
  // signal(SIGINT, signal_handler);

  // appserver = new_server(args);
  // open_serverconnections(appserver);
  // make_connectionpool(appserver);
  // begin_serverloop(appserver);

  // free_server(appserver);

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
  print_all_index(idxbuilder);
  dataset_offset offset = getb_offset(idxbuilder, 31, &status);

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
    update_dataitem(dbmng, 31, "AnotherO", offset.offset, &dataset);
    print_dataset(dbmng, dataset);
  }

  offset = getb_offset(idxbuilder, 31, &status);

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

  write_dataitem(dbmng, 36, "new", &dataset);
  offset = insertb_key(idxbuilder, 36);
  printb_indexlist(idxbuilder, offset.dataset - 1);

  print_dataset(dbmng, dataset);
  print_all_index(idxbuilder);

  printb_indexlist(idxbuilder, offset.dataset - 1);
  deleteb_key(idxbuilder, 26);
  printb_indexlist(idxbuilder, offset.dataset - 1);

  free_indexbuilder(idxbuilder);
  free_datasetmng(dbmng);
  free_cmdargs(args);
  return EXIT_SUCCESS;
}
```
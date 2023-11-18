```
int main(const int argc, const char* argv[]) {
  cmd_args* args = new_cmdargs();
  if (!validate_cmdargs(args, argc, argv)) {
    return EXIT_FAILURE;
  }  
  
  dataset_mng* dbmng = new_datasetmng(
    args->fname,
    args->dcount,
    args->vsize
  );
  int dataset;
  write_dataitem(dbmng, 20, "THIS", &dataset);
  write_dataitem(dbmng, 20, "THIS", &dataset);
  write_dataitem(dbmng, 20, "THIS", &dataset);
  print_dataset(dbmng, dataset);

  write_dataitem(dbmng, 21, "THIS2", &dataset);
  write_dataitem(dbmng, 21, "THIS2", &dataset);
  write_dataitem(dbmng, 21, "THIS2", &dataset);
  print_dataset(dbmng, dataset);

  write_dataitem(dbmng, 22, "THIS2", &dataset);
  write_dataitem(dbmng, 22, "THIS2", &dataset);
  write_dataitem(dbmng, 22, "THIS2", &dataset);
  print_dataset(dbmng, dataset);

  write_dataitem(dbmng, 23, "THIS2", &dataset);
  write_dataitem(dbmng, 23, "THIS2", &dataset);
  write_dataitem(dbmng, 23, "THIS2", &dataset);
  print_dataset(dbmng, dataset);

  write_dataitem(dbmng, 24, "THIS2", &dataset);
  write_dataitem(dbmng, 24, "THIS2", &dataset);
  write_dataitem(dbmng, 24, "THIS2", &dataset);
  print_dataset(dbmng, dataset);

  write_dataitem(dbmng, 25, "THIS2", &dataset);
  write_dataitem(dbmng, 25, "THIS2", &dataset);
  write_dataitem(dbmng, 25, "THIS2", &dataset);
  print_dataset(dbmng, dataset);


  free_datasetmng(dbmng);  
  free_cmdargs(args);

  return EXIT_SUCCESS;
}
```
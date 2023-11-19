#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "serverk_args.h"
#include "index_list.h"
#include "dataset_mng.h"
#include "message_parser.h"

#define DEBUG

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

  char value[args->vsize];
  int key = 0;
  get_value_by_offset(dbmng, dataset, 0, value, &key);
  printf(" %d : %s (vsize %ld)\n", key, value, sizeof(value));
  free_datasetmng(dbmng);
  free_cmdargs(args);

  return EXIT_SUCCESS;
}
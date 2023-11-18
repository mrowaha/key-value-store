#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>

#include"serverk_args.h"
#include"index_list.h"
#include"dataset_mng.h"
#include"message_parser.h"

#define DEBUG

int main(const int argc, const char* argv[]) {
  cmd_args* args = new_cmdargs();
  if (!validate_cmdargs(args, argc, argv, true)) {
    return EXIT_FAILURE;
  }  
  #ifdef DEBUG
  print_cmdargs(args);
  #endif

  dataset_mng* dbmng = new_datasetmng(
    args->fname,
    args->dcount,
    args->vsize
  );
  int dataset;
  write_dataitem(dbmng, 20, "THIS23awawra", &dataset);
  write_dataitem(dbmng, 20, "THIS23asfasfsad", &dataset);
  write_dataitem(dbmng, 20, "THIS23swfaf", &dataset);
  print_dataset(dbmng, dataset);

  write_dataitem(dbmng, 21, "1234", &dataset);
  write_dataitem(dbmng, 21, "12345678", &dataset);
  write_dataitem(dbmng, 21, "1234567890", &dataset);
  write_dataitem(dbmng, 21, "1234567890102", &dataset);
  print_dataset(dbmng, dataset);

  free_datasetmng(dbmng);  
  free_cmdargs(args);

  return EXIT_SUCCESS;
}
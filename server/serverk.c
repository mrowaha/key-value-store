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
  if (!validate_cmdargs(args, argc, argv)) {
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
  write_dataitem(dbmng, 20, "THIS", &dataset);
  write_dataitem(dbmng, 20, "THIS", &dataset);
  write_dataitem(dbmng, 20, "THIS", &dataset);
  print_dataset(dbmng, 1);
  free_datasetmng(dbmng);  
  free_cmdargs(args);

  return EXIT_SUCCESS;
}
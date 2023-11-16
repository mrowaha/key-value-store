#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>

#include"serverk_args.h"
#include"index_list.h"
#include"dataset_mng.h"
#include"message.h"

#define DEBUG

int main(const int argc, const char* argv[]) {
  cmd_args* args = new_cmdargs();
  if (!validate_cmdargs(args, argc, argv)) {
    return EXIT_FAILURE;
  }  
  #ifdef DEBUG
  print_cmdargs(args);
  #endif

  // dataset_mng* dbmng = new_datasetmng(
  //   args->fname,
  //   args->dcount,
  //   args->vsize
  // );
  // int dataset;
  // write_dataitem(dbmng, 20, "THIS", &dataset);
  // write_dataitem(dbmng, 20, "THIS", &dataset);
  // write_dataitem(dbmng, 20, "THIS", &dataset);
  // print_dataset(dbmng, 1);
  // free_datasetmng(dbmng);  
  // free_cmdargs(args);

  message_parser* parser = new_message_parser(args->vsize);
  void* request_msg;
  {
    char* value = (char*)malloc((sizeof(char) * args->vsize)); 
    strcpy(value, "HELO");
    int key = 2;
    int method = DEL_int;
    request_msg = new_request_msg(parser, key, method, value);
    free(value);
  }

  int key;
  int method;
  char* value = decode_request_msg(parser, request_msg, &key, &method);
  printf("key: %d\n", key);
  char* method_str = method_int_to_str(method);
  printf("method: %s\n", method_str);
  printf("value: %s\n", value);
  if (method_str) free(method_str);
  if (value) free(value);
  free(request_msg);
  free_message_parser(parser);
  free_cmdargs(args);
  return EXIT_SUCCESS;
}
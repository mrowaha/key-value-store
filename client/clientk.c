#include<stdlib.h>
#include"clientk_args.h"

int main(const int argc, const char* argv[]) {
  cmd_args* process_args = new_cmdargs();
  if (!validate_cmdargs(process_args, argc, argv)) {
    return EXIT_FAILURE;
  }

  print_cmdargs(process_args);
  free_cmdargs(process_args);
  return EXIT_SUCCESS;
}
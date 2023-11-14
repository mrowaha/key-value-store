#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>
#include"clientk_args.h"

cmd_args* new_cmdargs(void) {
  cmd_args* args = (cmd_args*)malloc(sizeof(cmd_args));
  args->clicount = 0;
  args->vsize = 0;
  args->dlevel = 0;
  args->fname = NULL;
  args->mqname = NULL;
  return args;
}

void free_cmdargs(cmd_args* args) {
  if (args != NULL) {
    if (args->fname != NULL) {
      free(args->fname);
    }

    if (args->mqname != NULL) {
      free(args->mqname);
    }
    free(args);
  }
}

void print_cmdargs(cmd_args* args) {
  printf("-n [clicount]: %d\n", args->clicount);
  if (args->fname) printf("-f [fname]: %s\n", args->fname);
  printf("-s [vsize]: %ld\n", args->vsize);
  if (args->mqname) printf("-m [mqname]: %s\n", args->mqname);
  printf("-d [dlevel]: %d\n", args->dlevel);
}


bool validate_cmdargs(cmd_args* process_args, const int argc, const char* argv[]) {
  bool hasDLevel = false;
  bool hasCliCount = false;
  bool hasVSize = false;
  bool hasFName = false;
  bool hasMQName = false;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      /* validate Dlevel */
      if (i == argc - 1) {
        fprintf(stderr, "-d requires a value\n");
        return false;
      }
      int dlevel = atoi(argv[i+1]);
      if (dlevel <= 0) {
        fprintf(stderr, "-d requires a positive non-zero integer\n");
        return false;
      }
      process_args->dlevel = dlevel;
      hasDLevel = true;
    } else if (strcmp(argv[i], "-n") == 0) {
      /* validate CliCount */
      if (i == argc - 1) {
        fprintf(stderr, "-n requires a value\n");
        return false;
      }
      int clicount = atoi(argv[i+1]);
      if (clicount <= 0) {
        fprintf(stderr, "-n requires a positive non-zero integer\n");
        return false;
      }
      process_args->clicount = clicount;
      hasCliCount = true;
    } else if (strcmp(argv[i], "-s") == 0) {
      /* validate VSize */
      if (i == argc - 1) {
        fprintf(stderr, "-s requires a value\n");
        return false;
      }
      size_t vsize = atoi(argv[i+1]);
      process_args->vsize = vsize;
      hasVSize = true;
    } else if (strcmp(argv[i], "-f") == 0) {
      if (i == argc - 1) {
        fprintf(stderr, "-f requires a value\n");
        return false;
      }
      int name_length = sizeof(argv[i+1]);
      process_args->fname = (char*)malloc(name_length);
      strcpy(process_args->fname, argv[i+1]);
      hasFName = true;
    } else if (strcmp(argv[i], "-m") == 0) {
      if (i == argc - 1) {
        fprintf(stderr, "-m requires a value\n");
        return false;
      }
      int name_length = sizeof(argv[i+1]);
      process_args->mqname = (char*)malloc(name_length);
      strcpy(process_args->mqname, argv[i+1]);
      hasMQName = true;
    }
    /* silently ignore invalid flags */
  }

  if (!hasDLevel) fprintf(stderr, "ERROR: -d [dlevel] required a positive integer value\n");
  if (!hasCliCount) fprintf(stderr, "ERROR: -n [clicount] requires a positive integer value\n");
  if (!hasVSize) fprintf(stderr, "ERROR: -s [vsize] requires a positive integer value\n");
  if (!hasFName) fprintf(stderr, "ERROR: -f [fname] requires a string for files prefix name\n");
  if (!hasMQName) fprintf(stderr, "ERROR: -m [mqname] requires a string for message queue prefix name\n");

  return hasDLevel && hasCliCount && hasVSize && hasFName && hasMQName;
}
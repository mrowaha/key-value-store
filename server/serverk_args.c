#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serverk_args.h"

cmd_args *new_cmdargs(void)
{
  cmd_args *args = (cmd_args *)malloc(sizeof(cmd_args));
  args->dcount = 0;
  args->tcount = 0;
  args->vsize = 0;
  args->fname = NULL;
  args->mqname = NULL;
  return args;
};

void free_cmdargs(cmd_args *args)
{
  if (args != NULL)
  {
    if (args->fname != NULL)
    {
      free(args->fname);
    }

    if (args->mqname != NULL)
    {
      free(args->mqname);
    }
    free(args);
  }
}

void print_cmdargs(cmd_args *args)
{
  printf("-d [dcount]: %d\n", args->dcount);
  if (args->fname)
    printf("-f [fname]: %s\n", args->fname);
  printf("-t [tcount]: %d\n", args->tcount);
  printf("-s [vsize]: %ld\n", args->vsize);
  if (args->mqname)
    printf("-m [mqname]: %s\n", args->mqname);
}

bool validate_cmdargs(cmd_args *process_args, const int argc, const char *argv[], const bool inc_vsize)
{
  bool hasDCount = false;
  bool hasTCount = false;
  bool hasVSize = false;
  bool hasFName = false;
  bool hasMQName = false;

  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-d") == 0)
    {
      /* validate DCount */
      if (i == argc - 1)
      {
        fprintf(stderr, "-d requires a value\n");
        return false;
      }
      int dcount = atoi(argv[i + 1]);
      if (dcount <= 0)
      {
        fprintf(stderr, "-d requires a positive non-zero integer\n");
        return false;
      }
      process_args->dcount = dcount;
      hasDCount = true;
    }
    else if (strcmp(argv[i], "-t") == 0)
    {
      /* validate TCount */
      if (i == argc - 1)
      {
        fprintf(stderr, "-t requires a value\n");
        return false;
      }
      int tcount = atoi(argv[i + 1]);
      if (tcount <= 0)
      {
        fprintf(stderr, "-t requires a positive non-zero integer\n");
        return false;
      }
      process_args->tcount = tcount;
      hasTCount = true;
    }
    else if (strcmp(argv[i], "-s") == 0)
    {
      /* validate VSize */
      if (i == argc - 1)
      {
        fprintf(stderr, "-s requires a value\n");
        return false;
      }
      size_t vsize = atoi(argv[i + 1]);
      if (inc_vsize)
        vsize++;
      process_args->vsize = vsize;
      hasVSize = true;
    }
    else if (strcmp(argv[i], "-f") == 0)
    {
      if (i == argc - 1)
      {
        fprintf(stderr, "-f requires a value\n");
        return false;
      }
      int name_length = sizeof(argv[i + 1]);
      process_args->fname = (char *)malloc(name_length);
      strcpy(process_args->fname, argv[i + 1]);
      hasFName = true;
    }
    else if (strcmp(argv[i], "-m") == 0)
    {
      if (i == argc - 1)
      {
        fprintf(stderr, "-m requires a value\n");
        return false;
      }
      int name_length = sizeof(argv[i + 1]);
      process_args->mqname = (char *)malloc(name_length);
      strcpy(process_args->mqname, argv[i + 1]);
      hasMQName = true;
    }
    /* silently ignore invalid flags */
  }

  if (!hasDCount)
    fprintf(stderr, "ERROR: -d [dcount] required a positive integer value\n");
  if (!hasTCount)
    fprintf(stderr, "ERROR: -t [tcount] requires a positive integer value\n");
  if (!hasVSize)
    fprintf(stderr, "ERROR: -s [vsize] requires a positive integer value\n");
  if (!hasFName)
    fprintf(stderr, "ERROR: -f [fname] requires a string for files prefix name\n");
  if (!hasMQName)
    fprintf(stderr, "ERROR: -m [mqname] requires a string for message queue prefix name\n");

  return hasDCount && hasTCount && hasVSize && hasFName && hasMQName;
}

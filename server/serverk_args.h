#ifndef SERVER_ARGS_0x123
#define SERVER_ARGS_0x123

#include<stddef.h>
#include<stdbool.h>

/**
 * Command line args
 * @dcount: number of datafiles to make
 * @tcount: nunmber of worker threads
 * @vsize: size of the value of the data item
 * @fname: suffix name of the datafiles
 * @mqname: suffix name of the message queues
*/
typedef struct {
  int dcount; /* the number of datafiles */
  int tcount; /* the number of worker threads */
  size_t vsize; /* value of the data item */
  char* fname; /* file names */
  char* mqname; /* message queue names */  
} cmd_args;

/**
 * make new command line args
 * all integers are set to 0 and char* to NULL
 * @return cmd_args pointer
*/
cmd_args* new_cmdargs(void);

void free_cmdargs(cmd_args*);

void print_cmdargs(cmd_args*);

bool validate_cmdargs(cmd_args*, const int argc, const char* argv[]);

#endif
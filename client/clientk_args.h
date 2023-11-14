#include<stddef.h>
#include<stdbool.h>

/**
 * command line args for the client
 * @clicount: number of client threads
 * @vsize: size of the value in the data item
 * @fname: name prefix for the input text files
 * @mqname: name prefix of the message queues
 * @dlevel: debug level
*/
typedef struct {
  int clicount;
  size_t vsize;
  char* fname;
  char* mqname;
  int dlevel;
} cmd_args;

cmd_args* new_cmdargs(void);

void free_cmdargs(cmd_args*);

void print_cmdargs(cmd_args*);

bool validate_cmdargs(cmd_args*, const int argc, const char* argv[]);
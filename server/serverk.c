#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <signal.h>

#include "serverk_args.h"
#include "dataset_mng.h"
#include "index_builder.h"
#include "message_parser.h"

#define DEBUG

typedef struct
{
  char *mqname;
  mqd_t receiver;
  struct mq_attr attributes;
  char *bufferp;
  int bufferlen;
} connection;

typedef struct
{
  index_builder *idxbuilder;
  dataset_mng *dbmng;
  message_parser *parser;
  connection connection;
} server;

static cmd_args *args;
static server *appserver;

server *new_server(cmd_args *args)
{
  if (args == NULL)
  {
    fprintf(stderr, "new_server: command line args cannot be null");
    return NULL;
  }

  server *newserver = (server *)malloc(sizeof(server));
  newserver->dbmng = new_datasetmng(
      args->fname,
      args->dcount,
      args->vsize);
  newserver->idxbuilder = new_indexbuilder(newserver->dbmng);
  newserver->parser = new_message_parser(args->vsize);
  newserver->connection.mqname = args->mqname;

  for (int i = 0; i < args->dcount; i++)
  {
    int dataset;
    write_dataitem(newserver->dbmng, i + 1, "THIS", &dataset);
  }

  return newserver;
}

void open_serverconnections(server *server)
{

  connection *connection = &(server->connection);
  connection->receiver = mq_open(connection->mqname, O_RDWR | O_CREAT, 0666, NULL);
  if (connection->receiver == -1)
  {
    perror("open_serverconnections");
    exit(1);
  }

  mq_getattr(connection->receiver, &connection->attributes);
  connection->bufferlen = connection->attributes.mq_msgsize;
  connection->bufferp = (char *)malloc(connection->bufferlen);
}

void begin_serverloop(server *server)
{

  size_t request_size = get_request_msg_size(server->parser);
  connection *connection = &(server->connection);
  ssize_t n;
  int x = 0;
  while (x <= 10)
  {
    x++;
    n = mq_receive(connection->receiver, connection->bufferp, connection->bufferlen, NULL);
    if (n == -1)
    {
      perror("mq_receive");
      exit(1);
    }

    int key;
    int method;
    char *value = decode_request_msg(
        server->parser,
        (void *)connection->bufferp,
        &key,
        &method);
    printf("key: %d\n", key);
    char *method_str = method_int_to_str(method);
    printf("method: %s\n", method_str);
    printf("value: %s\n", value);
    if (method_str)
      free(method_str);
    if (value)
      free(value);
  }
}

void free_server(server *server)
{
  if (server != NULL)
  {
    mq_close(server->connection.receiver);
    free_message_parser(server->parser);
    free_indexbuilder(server->idxbuilder);
    free_datasetmng(server->dbmng);
    free(server);
    printf("\n*****server exiting*****\n");
  }
}

void signal_handler(int signum)
{
  if (signum == SIGTERM || signum == SIGINT)
  {
    free_server(appserver);
    free_cmdargs(args);
    exit(0);
  }
}

int main(const int argc, const char *argv[])
{
  args = new_cmdargs();
  if (!validate_cmdargs(args, argc, argv, true))
  {
    return EXIT_FAILURE;
  }
#ifdef DEBUG
  print_cmdargs(args);
#endif

  appserver = new_server(args);
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
  open_serverconnections(appserver);
  begin_serverloop(appserver);
  return EXIT_SUCCESS;
}
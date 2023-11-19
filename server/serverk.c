#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <signal.h>
#include <pthread.h>

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
  pthread_t *workerthreads;
  int *threadnumber;
  int workerthreadcount; /* connection pool size */

} server;

typedef struct
{
  int key;
  int method;
  char *value;
  bool isnew;
} mq_request;

static cmd_args *args;
static server *appserver;
// global current request
static mq_request curr_request;
pthread_mutex_t curr_requestmtx;
pthread_cond_t curr_requestcond;

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

  newserver->workerthreadcount = args->tcount;
  newserver->workerthreads = (pthread_t *)malloc(sizeof(pthread_t) * args->tcount);
  newserver->threadnumber = (int *)malloc(sizeof(int) * args->tcount);

  pthread_mutex_init(&curr_requestmtx, NULL);
  pthread_cond_init(&curr_requestcond, NULL);
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

  connection *connection = &(server->connection);
  ssize_t n;
  while (true)
  {
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
    pthread_mutex_lock(&curr_requestmtx);
    curr_request.key = key;
    curr_request.method = method;
    if (value != NULL)
    {
      curr_request.value = (char *)malloc(strlen(value));
      strcpy(curr_request.value, value);
      int length = strlen(curr_request.value);
      if (length > 0 && curr_request.value[length - 1] == '\n')
      {
        curr_request.value[length - 1] = '\0'; // Set the last character to null-terminator
      }
    }
    else
    {
      curr_request.value = NULL;
    }

#ifdef DEBUG
    printf("key: %d, method: %s, value: %s\n",
           curr_request.key,
           method_int_to_str(curr_request.method),
           curr_request.value);
#endif

    pthread_mutex_unlock(&curr_requestmtx);
    pthread_cond_signal(&curr_requestcond);

    if (value != NULL)
      free(value);
  }
}

void *responserunner(void *workerargs)
{
  int number = (*(int *)workerargs) + 1;
  mq_request thread_request;
  while (true)
  {
    pthread_mutex_lock(&curr_requestmtx);
    while (curr_request.isnew == false)
    {
      // if there was no new request, wait for the new
      // request signal
      pthread_cond_wait(&curr_requestcond, &curr_requestmtx);
      // unlocks the curr request
    }
    // gets the lock again for curr request
    thread_request.key = curr_request.key;
    thread_request.method = curr_request.method;
    if (curr_request.value == NULL)
    {
      thread_request.value = NULL;
    }
    else
    {
      thread_request.value = (char *)malloc(strlen(curr_request.value));
      strcpy(thread_request.value, curr_request.value);
    }
    curr_request.isnew = false;
    pthread_mutex_unlock(&curr_requestmtx);

// handle request
#ifdef DEBUG
    printf("key: %d, method: %s, value: %s\n",
           thread_request.key,
           method_int_to_str(thread_request.method),
           thread_request.value);
#endif

    free(thread_request.value);
    thread_request.value = NULL;
  }
}

void make_connectionpool(server *server)
{

  int ret;
  for (int i = 0; i < server->workerthreadcount; i++)
  {
    server->threadnumber[i] = i;
    ret = pthread_create(&(server->workerthreads[i]), NULL, responserunner, &(server->threadnumber[i]));
  }
  if (ret != 0)
  {
    fprintf(stderr, "worker thread create failed");
    return;
  }
}

void free_server(server *server)
{
  if (server != NULL)
  {
    pthread_mutex_destroy(&curr_requestmtx);
    pthread_cond_destroy(&curr_requestcond);
    mq_close(server->connection.receiver);
    free_message_parser(server->parser);
    free_indexbuilder(server->idxbuilder);
    free_datasetmng(server->dbmng);
    free(server->workerthreads);
    free(server->threadnumber);
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

  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);

  appserver = new_server(args);
  open_serverconnections(appserver);
  make_connectionpool(appserver);
  begin_serverloop(appserver);
  return EXIT_SUCCESS;
}
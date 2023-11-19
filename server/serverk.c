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
  char *resp_mqname;
  mqd_t responder;
  pthread_mutex_t *transactional_mtx;
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
static pthread_mutex_t curr_requestmtx;
static pthread_cond_t curr_requestcond;

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
  newserver->resp_mqname = (char *)malloc(sizeof(args->mqname) + sizeof("-resp"));
  sprintf(newserver->resp_mqname, "%s-resp", args->mqname);
  newserver->responder = mq_open(newserver->resp_mqname, O_WRONLY | O_CREAT, 0666, NULL);
  if (newserver->responder == -1)
  {
    perror("new_server");
    exit(1);
  }

  newserver->workerthreadcount = args->tcount;
  newserver->workerthreads = (pthread_t *)malloc(sizeof(pthread_t) * args->tcount);
  newserver->threadnumber = (int *)malloc(sizeof(int) * args->tcount);

  pthread_mutex_init(&curr_requestmtx, NULL);
  pthread_cond_init(&curr_requestcond, NULL);

  newserver->transactional_mtx = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * args->dcount);
  for (int i = 0; i < args->dcount; i++)
  {
    pthread_mutex_init(&(newserver->transactional_mtx[i]), NULL);
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
    printf("[main server] key: %d, method: %s, value: %s\n",
           curr_request.key,
           method_int_to_str(curr_request.method),
           curr_request.value);
#endif
    curr_request.isnew = true;
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
  message_parser *parser = appserver->parser;
  size_t resp_msgsize = get_response_msg_size(parser);
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

    // use transactional mutexes on atomic operations
    // such as dataset writes and index updates
    bool success = true; // optimistic update
    char *value = NULL;
    int transactional_idx = (thread_request.key % args->dcount);
    pthread_mutex_lock(&(appserver->transactional_mtx[transactional_idx]));
    index_builder *idxbuilder = appserver->idxbuilder;
    dataset_mng *dbmng = appserver->dbmng;
    dataset_offset result;
    switch (thread_request.method)
    {
    case (PUT_int):
      // handles the put request
      // if the key already exists
      // then the value is updated
      // else a new value is created
      bool status;
      result = getb_offset(idxbuilder, thread_request.key, &status);
      if (status)
      {
        // value exists, update
        int dataset;
        update_dataitem(dbmng, thread_request.key, thread_request.value, result.offset, &dataset);
      }
      else
      {
        // value does not exist, add to dataset
        // and update idxbuilder accordingly
        int dataset;
        write_dataitem(dbmng, thread_request.key, thread_request.value, &dataset);
        result = insertb_key(idxbuilder, thread_request.key);
      }
      break;
    case (GET_int):
      result = getb_offset(idxbuilder, thread_request.key, &status);
      printf("offset %d, dataset: %d\n", result.offset, result.dataset);
      if (status)
      {
        // value exists, return value
        success = true;
        int key = 0;
        char tempvalue[args->vsize];
        get_value_by_offset(dbmng, result.dataset, result.offset, tempvalue, &key);
        printf(" %d : %s (vsize %ld)\n", key, tempvalue, sizeof(tempvalue));
        value = (char *)malloc(sizeof(tempvalue));
        strcpy(value, tempvalue);
        // ignores key
      }
      else
      {
        success = false;
        value = NULL;
      }
      break;
    case (DEL_int):
      // if it does not exist error
      // delete by offset in dataset
      // delete in the index list
      bool exists;
      result = getb_offset(idxbuilder, thread_request.key, &exists);
      if (!exists)
      {
        success = false;
        value = NULL;
      }
      else
      {
        delete_dataitem(dbmng, thread_request.key, result.offset, result.dataset);
        deleteb_key(idxbuilder, thread_request.key);
        success = true;
        value = NULL;
      }
      break;
    }
    pthread_mutex_unlock(&(appserver->transactional_mtx[transactional_idx]));
// done handling request
#ifdef DEBUG
    printf("[worker] key: %d, method: %s, value: %s\n",
           thread_request.key,
           method_int_to_str(thread_request.method),
           thread_request.value);
#endif

    // write request back
    void *bufferp = new_response_msg(
        parser,
        success,
        value);

    ssize_t n;
    n = mq_send(appserver->responder, bufferp, resp_msgsize, 0);
    if (n == -1)
    {
      perror("mq_send");
    }
    free(bufferp);
    free(thread_request.value);
    if (value != NULL)
      free(value);
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
    for (int i = 0; i < args->dcount; i++)
    {
      pthread_mutex_destroy(&(server->transactional_mtx[i]));
    }
    pthread_mutex_destroy(&curr_requestmtx);
    pthread_cond_destroy(&curr_requestcond);
    mq_close(server->connection.receiver);
    free_message_parser(server->parser);
    free_indexbuilder(server->idxbuilder);
    free_datasetmng(server->dbmng);
    free(server->workerthreads);
    free(server->threadnumber);
    free(server->resp_mqname);
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

  free_server(appserver);
  free_cmdargs(args);
  return EXIT_SUCCESS;
}
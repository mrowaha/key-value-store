#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <pthread.h>

#include "message_parser.h"
#include "clientk_args.h"

typedef struct
{
  char *mqname;
  mqd_t sender;
  struct mq_attr attrs;
} connection;

typedef struct
{
  message_parser *parser;
  connection connection;
  pthread_t *workerthreads;
  int *threadnumber;
  int workerthreadcount;
} client;

cmd_args *process_args;
client *appclient;
pthread_mutex_t messagemtx;

client *new_client(cmd_args *args)
{
  if (args == NULL)
  {
    fprintf(stderr, "new_server: command line args cannot be null");
    return NULL;
  }

  client *newclient = (client *)malloc(sizeof(client));
  newclient->parser = new_message_parser(args->vsize);
  newclient->connection.mqname = args->mqname;
  newclient->workerthreadcount = args->clicount;
  newclient->workerthreads = (pthread_t *)malloc(sizeof(pthread_t) * args->clicount);
  newclient->threadnumber = (int *)malloc(sizeof(int) * args->clicount);
  return newclient;
}

void free_client(client *appclient)
{
  if (appclient)
  {
    mq_close(appclient->connection.sender);
    free_message_parser(appclient->parser);
    free(appclient->workerthreads);
    free(appclient->threadnumber);
    free(appclient);
  }
}

void open_sendconnection(client *appclient)
{
  connection *connection = &(appclient)->connection;
  connection->sender = mq_open(connection->mqname, O_RDWR);
  if (connection->sender == -1)
  {
    perror("mq_open");
    exit(1);
  }
}

void send_message(int key, int method, char *value)
{
  pthread_mutex_lock(&messagemtx);
  connection *connection = &(appclient)->connection;
  size_t requestmsg_size = get_request_msg_size(appclient->parser);
  ssize_t n;
  mq_getattr(connection->sender, &(connection->attrs));

  void *bufferp = new_request_msg(appclient->parser, key, method, value);
  n = mq_send(connection->sender, bufferp, requestmsg_size, 0);
  if (n == -1)
  {
    free(bufferp);
    perror("mq_send");
    pthread_mutex_unlock(&messagemtx);
    exit(1);
  }
  free(bufferp);
  pthread_mutex_unlock(&messagemtx);
}

void *requestrunner(void *threadargs)
{
  int number = (*(int *)threadargs) + 1;
  char filename[sizeof(process_args->fname) + sizeof(".txt") + sizeof(int)];
  sprintf(filename, "%s%d.txt", process_args->fname, number);
  FILE *infile = fopen(filename, "r");
  if (infile == NULL)
  {
    perror("requestrunner");
    exit(1);
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  char *token;

  int method;
  int key;
  char *value;

  while ((read = getline(&line, &len, infile)) != -1)
  {
    value = NULL;
    token = strtok(line, " "); // get method
    method = method_str_to_int(token);
    token = strtok(NULL, " "); // get key
    key = atoi(token);
    if (token != NULL)
    {
      token = strtok(NULL, " "); // get value
      value = token;
    }
    send_message(key, method, value);
  }
  free(line);
  fclose(infile);
  return NULL;
}

void begin_workerthreads(client *appclient)
{
  pthread_mutex_init(&messagemtx, NULL);
  int ret;
  for (int i = 0; i < appclient->workerthreadcount; i++)
  {
    appclient->threadnumber[i] = i;
    ret = pthread_create(&(appclient->workerthreads[i]), NULL, requestrunner, &(appclient->threadnumber[i]));
    if (ret != 0)
    {
      fprintf(stderr, "worker thread create failed");
      return;
    }
  }

  for (int i = 0; i < appclient->workerthreadcount; i++)
  {
    pthread_join(appclient->workerthreads[i], NULL);
  }
}

int main(const int argc, const char *argv[])
{
  process_args = new_cmdargs();
  if (!validate_cmdargs(process_args, argc, argv))
  {
    return EXIT_FAILURE;
  }

  appclient = new_client(process_args);
  open_sendconnection(appclient);
  begin_workerthreads(appclient);

  free_client(appclient);
  free_cmdargs(process_args);
  pthread_mutex_destroy(&messagemtx);
  return EXIT_SUCCESS;
}
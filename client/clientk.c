#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <pthread.h>

#include "message_parser.h"
#include "clientk_args.h"

#define DEBUG

typedef struct
{
  char *mqname;
  mqd_t mq;
  struct mq_attr attrs;
  int bufferlen;
  void *bufferp;
} connection;

enum MODE
{
  SPEC,
  TERMINAL
};

typedef struct
{
  message_parser *parser;
  connection sender;
  connection receiver;
  pthread_t *workerthreads;
  int *threadnumber;
  int workerthreadcount;
  enum MODE mode;
} client;

cmd_args *process_args;
client *appclient;
pthread_mutex_t request_mtx;

client *new_client(cmd_args *args)
{
  if (args == NULL)
  {
    fprintf(stderr, "new_server: command line args cannot be null");
    return NULL;
  }

  client *newclient = (client *)malloc(sizeof(client));
  newclient->mode = SPEC;
  newclient->parser = new_message_parser(args->vsize);
  newclient->sender.mqname = args->mqname;

  newclient->workerthreadcount = args->clicount;
  newclient->workerthreads = (pthread_t *)malloc(sizeof(pthread_t) * args->clicount);
  newclient->threadnumber = (int *)malloc(sizeof(int) * args->clicount);
  pthread_mutex_init(&request_mtx, NULL);

  if (process_args->clicount == 0)
  {
    newclient->mode = TERMINAL;
  }

  return newclient;
}

void free_client(client *appclient)
{
  if (appclient)
  {
    pthread_mutex_destroy(&request_mtx);
    mq_close(appclient->sender.mq);
    free_message_parser(appclient->parser);
    free(appclient->workerthreads);
    free(appclient->threadnumber);
    free(appclient);
  }
}

void open_connections(client *appclient)
{
  connection *sending = &(appclient)->sender;
  sending->mq = mq_open(sending->mqname, O_RDWR);
  if (sending->mq == -1)
  {
    perror("mq_open");
    exit(1);
  }

  char *mqname_rec = (char *)malloc(sizeof(sending->mqname) + sizeof("-resp"));
  sprintf(mqname_rec, "%s-resp", sending->mqname);
  connection *receiving = &(appclient)->receiver;
  receiving->mqname = mqname_rec;
  receiving->mq = mq_open(receiving->mqname, O_RDONLY);
  if (sending->mq == -1)
  {
    perror("mq_open");
    exit(1);
  }

  mq_getattr(receiving->mq, &(receiving->attrs));
  int bufferlen = receiving->attrs.mq_msgsize;
  void *bufferp = malloc(bufferlen);
  receiving->bufferlen = bufferlen;
  receiving->bufferp = bufferp;
}

void send_message(int key, int method, char *value)
{
  connection *connection = &(appclient)->sender;
  size_t requestmsg_size = get_request_msg_size(appclient->parser);
  ssize_t n;
  mq_getattr(connection->mq, &(connection->attrs));

  void *bufferp = new_request_msg(appclient->parser, key, method, value);
  n = mq_send(connection->mq, bufferp, requestmsg_size, 0);
  if (n == -1)
  {
    free(bufferp);
    perror("mq_send");
    exit(1);
  }
  free(bufferp);
  n = mq_receive(appclient->receiver.mq, appclient->receiver.bufferp, appclient->receiver.bufferlen, NULL);
  if (n == -1)
  {
    perror("mq_send");
    exit(1);
  }
  bool success;
  char *respvalue = decode_response_msg(appclient->parser, appclient->receiver.bufferp, &success);
  printf("success: %d\n", success);
  printf("value: %s\n", respvalue);
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
    return NULL;
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
    pthread_mutex_lock(&request_mtx);
    send_message(key, method, value);
    pthread_mutex_unlock(&request_mtx);
  }
  free(line);
  fclose(infile);
  return NULL;
}

void begin_workerthreads(client *appclient)
{
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

void printmenu()
{
  printf("***MAKE REQUESTS***\n");
  printf("PUT [key] [value]\n");
  printf("GET [key]\n");
  printf("DEL [key]\n");
}

void begin_terminalinput(client *appclient)
{

  printf("Client is running in terminal mode\n");
  char input[sizeof(char) * process_args->vsize + sizeof(QUITSERVER) + sizeof(int)];
  while (strcmp(input, "QUIT") != 0)
  {
    printmenu();
    printf("input: ");
    fgets(input, sizeof(input), stdin);

    if (strcmp(input, "QUIT") == 0)
      break;
    ;

    int len = strlen(input);
    if (len > 0 && input[len - 1] == '\n')
    {
      input[len - 1] = '\0';
    }
    printf("%s\n", input);
    char *token;

    int method;
    int key;
    char *value;

    value = NULL;
    token = strtok(input, " "); // get method
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
}

int main(const int argc, const char *argv[])
{
  process_args = new_cmdargs();
  if (!validate_cmdargs(process_args, argc, argv))
  {
    return EXIT_FAILURE;
  }

  appclient = new_client(process_args);
  open_connections(appclient);
  if (appclient->mode == SPEC)
  {
    begin_workerthreads(appclient);
  }
  else
  {
    begin_terminalinput(appclient);
  }

  free_client(appclient);
  free_cmdargs(process_args);
  return EXIT_SUCCESS;
}
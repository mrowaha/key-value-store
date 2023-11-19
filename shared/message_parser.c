#include <stdio.h>
#include <string.h>
#include "message_parser.h"

#define DEBUG

typedef struct message_parser
{
  size_t vsize;
} message_parser;

message_parser *new_message_parser(const size_t vsize)
{
  message_parser *parser = (message_parser *)malloc(sizeof(message_parser));
  parser->vsize = vsize;
  return parser;
}

void free_message_parser(message_parser *parser)
{
  free(parser);
}

int method_str_to_int(const char *method)
{
  if (strcmp(method, PUT) == 0)
  {
    return PUT_int;
  }

  if (strcmp(method, GET) == 0)
  {
    return GET_int;
  }

  if (strcmp(method, DEL) == 0)
  {
    return DEL_int;
  }

  if (strcmp(method, DUMP) == 0)
  {
    return DUMP_int;
  }

  if (strcmp(method, QUIT) == 0)
  {
    return QUIT_int;
  }

  if (strcmp(method, QUITSERVER) == 0)
  {
    return QUITSERVER_int;
  }

  return -1;
}

char *method_int_to_str(const int method)
{
  char *method_str = NULL;
  switch (method)
  {
  case PUT_int:
    method_str = (char *)malloc(sizeof(PUT));
    strcpy(method_str, PUT);
    return method_str;
  case GET_int:
    method_str = (char *)malloc(sizeof(GET));
    strcpy(method_str, GET);
    return method_str;
  case DEL_int:
    method_str = (char *)malloc(sizeof(DEL));
    strcpy(method_str, DEL);
    return method_str;
  case DUMP_int:
    method_str = (char *)malloc(sizeof(DUMP));
    strcpy(method_str, DUMP);
    return method_str;
  case QUIT_int:
    method_str = (char *)malloc(sizeof(QUIT));
    strcpy(method_str, QUIT);
    return method_str;
  case QUITSERVER_int:
    method_str = (char *)malloc(sizeof(QUITSERVER));
    strcpy(method_str, QUITSERVER);
    return method_str;
  }

  return NULL;
}

size_t get_request_msg_size(const message_parser *parser)
{

  size_t vsize = parser->vsize;

  typedef struct
  {
    int key;
    int method;
    char value[vsize];
  } request_msg;

  return sizeof(request_msg);
}

size_t get_response_msg_size(const message_parser *parser)
{

  size_t vsize = parser->vsize;

  typedef struct
  {
    bool success;
    char value[vsize];
  } response_msg;

  return sizeof(response_msg);
}

void *new_request_msg(const message_parser *parser, const int key, const int method, const char *value)
{

  size_t vsize = parser->vsize;

  typedef struct
  {
    int key;
    int method;
    char value[vsize];
  } request_msg;

  request_msg msg;
  msg.key = key;
  msg.method = method;
  memset(msg.value, '\0', vsize);
  if (value != NULL && method == PUT_int)
  {
    strncpy(msg.value, value, vsize);
    msg.value[vsize - 1] = '\0';
  }
  void *returnmsg = malloc(sizeof(msg));
  memcpy(returnmsg, &msg, sizeof(msg));
#ifdef DEBUG
  request_msg *check = (request_msg *)returnmsg;
  printf("[request] key: %d\n", check->key);
  printf("method: %d\n", check->method);
  printf("value: %s\n", check->value);
#endif
  return returnmsg;
}

void *new_response_msg(const message_parser *parser, const bool success, const char *value)
{

  size_t vsize = parser->vsize;

  typedef struct
  {
    bool success;
    char value[vsize];
  } response_msg;

  response_msg *msg = (response_msg *)malloc(sizeof(response_msg));
  msg->success = success;
  if (value != NULL)
  {
    strcpy(msg->value, value);
  }
  else
  {
    memset(msg->value, '\0', vsize);
  }

  printf("[response] success: %d, value: %s\n", msg->success, msg->value);
  return (void *)msg;
}

char *decode_request_msg(const message_parser *parser, void *msg, int *key, int *method)
{

  size_t vsize = parser->vsize;

  typedef struct
  {
    int key;
    int method;
    char value[vsize];
  } request_msg;

  request_msg *tmp_msg = (request_msg *)msg;
  *key = tmp_msg->key;
  *method = tmp_msg->method;

  char *value = NULL;
  if (tmp_msg->method == PUT_int)
  {
    value = (char *)malloc(sizeof(char) * parser->vsize);
    strcpy(value, tmp_msg->value);
  }
  return value;
}

char *decode_response_msg(const message_parser *parser, void *msg, bool *success)
{

  size_t vsize = parser->vsize;

  typedef struct
  {
    bool success;
    char value[vsize];
  } response_msg;

  response_msg *tmp_msg = (response_msg *)msg;
  *success = tmp_msg->success;
  char *value = NULL;
  if (vsize > 0 && tmp_msg->value[0] != '\0')
  {
    value = (char *)malloc(sizeof(char) * vsize);
    strcpy(value, tmp_msg->value);
  }
  return value;
}
```
int main(const int argc, const char* argv[]) {

  struct cmd_args args;
  init_args(&args, argc, argv);

  message_parser* parser = new_message_parser(args->vsize);
  void* request_msg;
  {
    char* value = (char*)malloc((sizeof(char) * args->vsize)); 
    strcpy(value, "HELOS");
    int key = 2;
    int method = method_str_to_int(PUT);
    request_msg = new_request_msg(parser, key, method, value);
    free(value);
  }

  int key;
  int method;
  char* value = decode_request_msg(parser, request_msg, &key, &method);
  printf("key: %d\n", key);
  char* method_str = method_int_to_str(method);
  printf("method: %s\n", method_str);
  printf("value: %s\n", value);
  if (method_str) free(method_str);
  if (value) free(value);
  free(request_msg);
  free_message_parser(parser);
}
```
#ifndef MESSAGE_H
#define MESSAGE_H

#include<stdbool.h>
#include<stdlib.h>

/* METHODS */
#define PUT "PUT"
#define GET "GET"
#define DEL "DEL"
#define DUMP "DUMP"
#define QUIT "QUIT"
#define QUITSERVER "QUITSERVER"

#define PUT_int 1
#define GET_int 2
#define DEL_int 3
#define DUMP_int 4
#define QUIT_int 5
#define QUITSERVER_int 6

/**
 * message parser from raw bytes
*/
typedef struct message_parser message_parser;

message_parser* new_message_parser(const size_t vsize);

void free_message_parser(message_parser*);

/**
 * maps the string representation of a method to integer
 * @method: string representation of method
 * @return integer representation of method, else -1
*/
int method_str_to_int(const char* method);

/**
 * maps the integer representation of a method to string
 * @method: integer representation of method
 * @return string representation of method, else NULL
*/
char* method_int_to_str(const int method);

/**
 * get request message size from vsize
 * @return size of the request message
*/
size_t get_request_msg_size(const message_parser*);

/**
 * get response message size from vsize
 * @return size of the response message
*/
size_t get_response_msg_size(const message_parser*);

/**
 * make a new request message
 * @key: key in the message
 * @method: request method as integer
 * @value: value of the message [required for PUT]
 * @return pointer to request message, must deallocate
*/
void* new_request_msg(const message_parser*, const int key, const int method, const char* value);

/**
 * make a new response message
 * @success: success of the response
 * @value: value associated with the key
 * @return pointer to response message, must deallocate 
*/
void* new_response_msg(const message_parser*, const bool success, const char* value);

/**
 * decodes the request message into valid message
 * @request_msg: bytes received
 * @key: set key
 * @method: set method as integer
 * @return value or null
*/
char* decode_request_msg(const message_parser*, void* msg, int* key, int* method);

/**
 * decodes the response message into valid message
 * @response_msg: bytes received
 * @success: set success
 * @return value or null
*/
char* decode_response_msg(const message_parser*, void* msg, bool* success);

#endif
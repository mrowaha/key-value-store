/* METHODS */
#define PUT "PUT"
#define GET "GET"
#define DEL "DEL"

/**
 * message struct for request
 * @key: key of the data item
 * @method: PUT, GET, DEL
 * @value: char array of size VSIZE
*/
typedef struct {
  int key;
  char* value;
  char method[3];
} request_msg;

/**
 * message struct for response
 * @success: 0 if successful, 1 if not
 * @value: char array of size VSIZE
*/
typedef struct {
  int success;
  char* value;
} response_msg;
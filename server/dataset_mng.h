#ifndef DATASET_MNG_0x123
#define DATASET_MNG_0x123
/**
 * this header file includes method for managing dataset files
*/

#include<stdbool.h>


/**
 * this function makes the prefix name required by the dataset files
 * @fname: filename prefix of the files
 * @dcount: dcount of the args
 * @size: set the size of the filename prefix
 * @return filename dynamically set and filed with null characters
*/
char* make_dataset_prefix(const char* fname, const int dname, int* size);

/**
 * dataset manager
 * @dcount: dcount of the server
 * @fname: filename prefix
 * @vsize: vsize of the current server invocation
 * @datasethandlers: all open write connections to underlying dataset files
*/
typedef struct dataset_mng {
  int dcount;
  size_t vsize;
  char* fname;
  int* descriptors;
  int* sizes;
  void** mmaps;
} dataset_mng;

dataset_mng* new_datasetmng(const char* fname, const int dcount, const size_t vsize);

void free_datasetmng(dataset_mng*);

/**
 * create dataset files if they do not exist 
*/
void make_datasets(const dataset_mng*);

/**
 * writes the dataitem to the underlying dataset file
 * always appends the dataitem to the end of file
 * does not check existence of the key
*/
bool write_dataitem(const dataset_mng*, const int key, const char* value, int* dataset);

/**
 * print file content (key value pair of the datastore)
 * @dataset: the dataset number, must be less than dcount
*/
void print_dataset(const dataset_mng*, const int dataset);


#endif
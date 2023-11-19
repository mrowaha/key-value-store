#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "dataset_mng.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))

// enable to log debugs to console
// #define DEBUG

dataset_mng *new_datasetmng(const char *fname, const int dcount, const size_t vsize)
{
  dataset_mng *dbmng = (dataset_mng *)malloc(sizeof(dataset_mng));
  dbmng->dcount = dcount;
  dbmng->fname = (char *)malloc(strlen(fname) + 1);
  strcpy(dbmng->fname, fname);
  dbmng->vsize = vsize;
  dbmng->descriptors = (int *)malloc(sizeof(int) * dcount);
  dbmng->sizes = (int *)malloc(sizeof(int) * dcount);
  dbmng->mmaps = (void **)malloc(sizeof(void *) * dcount);
  make_datasets(dbmng);
  return dbmng;
}

void free_datasetmng(dataset_mng *dbmng)
{
  if (dbmng != NULL)
  {
    for (int i = 0; i < dbmng->dcount; i++)
    {
      munmap(dbmng->mmaps[i], dbmng->sizes[i]);
      close(dbmng->descriptors[i]);
    }
    free(dbmng->sizes);
    free(dbmng->mmaps);
    free(dbmng->descriptors);
    if (dbmng->fname != NULL)
    {
      free(dbmng->fname);
    }
    free(dbmng);
  }
  dbmng = NULL;
}

char *make_dataset_prefix(const char *fname, const int dcount, int *size)
{
  int number = dcount, count = 0;
  while (number > 0)
  {
    number /= 10;
    count++;
  }
  int index = 0;
  while (fname[index] != '\0')
  {
    count++;
    index++;
  }
  *size = count + sizeof(".bin");
  char *filename = (char *)malloc(*size);
  for (int i = 0; i < *size; i++)
  {
    filename[i] = '\0';
  }
  return filename;
}

void make_datasets(const dataset_mng *dbmng)
{
  int size;
  char *filename = make_dataset_prefix(dbmng->fname, dbmng->dcount, &size);
  struct stat sb;
  for (int i = 1; i <= dbmng->dcount; i++)
  {
    sprintf(filename, "%s%d.bin", dbmng->fname, i);
    // int fd = open(filename, O_RDWR | O_CREAT);
    FILE *file = fopen(filename, "r+b");
    if (file == NULL)
    {
      file = fopen(filename, "w+b");
    }
    if (file == NULL)
    {
      perror("make_datasets");
      exit(1);
    }

    int fd = fileno(file);
    if (fstat(fd, &sb) == -1)
    {
      perror("make_datasets: could not get file size");
    }
    printf("file size %ld\n", sb.st_size);
    dbmng->descriptors[i - 1] = fd;
    dbmng->mmaps[i - 1] = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    dbmng->sizes[i - 1] = sb.st_size;
  }
  free(filename);
}

bool write_dataitem(const dataset_mng *dbmng, const int key, const char *value, int *dataset)
{
  if (key <= 0)
  {
    fprintf(stderr, "write_dataitem: key can only be a positive nonzero integer");
    return false;
  }

  struct dataitem
  {
    int key;
    char value[dbmng->vsize];
  };

  int filenumber = (key % dbmng->dcount) + 1;
  int fd = dbmng->descriptors[filenumber - 1];
  int old_length = dbmng->sizes[filenumber - 1] / sizeof(struct dataitem);
  int new_length = old_length + 1;
  size_t new_size = new_length * sizeof(struct dataitem);

  if (ftruncate(fd, new_size) == -1)
  {
    perror("ftruncate");
    return false;
  }

  struct dataitem item;
  item.key = key;
  memset(item.value, '\0', dbmng->vsize);
  strncpy(item.value, value, min(dbmng->vsize, strlen(value)));
  item.value[dbmng->vsize - 1] = '\0';

  struct dataitem *newmap = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (newmap == MAP_FAILED)
  {
    fprintf(stderr, "failed to create new map");
    return false;
  }
  memcpy(newmap, (struct dataitem *)dbmng->mmaps[filenumber - 1], dbmng->sizes[filenumber - 1]);
  newmap[new_length - 1] = item;
  munmap(dbmng->mmaps[filenumber - 1], dbmng->sizes[filenumber - 1]);
  dbmng->sizes[filenumber - 1] = new_size;
  dbmng->mmaps[filenumber - 1] = (void *)newmap;
  *dataset = filenumber;
  return true;
}

void print_dataset(const dataset_mng *dbmng, const int dataset)
{
  if (dataset < 1 || dataset > dbmng->dcount)
  {
    fprintf(stderr, "print_dataset: dataset number invalid\n");
    return;
  }

  // define the dataitem
  struct dataitem
  {
    int key;
    char value[dbmng->vsize];
  };

  int dbindex = dataset - 1;
  struct dataitem *datasetmap = (struct dataitem *)dbmng->mmaps[dbindex];
  int mmaplength = dbmng->sizes[dbindex] / sizeof(struct dataitem);
  printf("dataset %d: [\n", dataset);
  for (int i = 0; i < mmaplength; i++)
  {
    printf("{ %d : %s (vsize: %ld) }\n", datasetmap[i].key, datasetmap[i].value, sizeof(datasetmap[i].value));
  }
  printf("]\n");
}

void get_dataset_idx(const dataset_mng *dbmng, const int dataset, index_list *idxlist)
{
  if (dataset < 1 || dataset > dbmng->dcount)
  {
    fprintf(stderr, "get_dataset_idx: dataset number invalid\n");
    return;
  }

  // define the dataitem
  struct dataitem
  {
    int key;
    char value[dbmng->vsize];
  };

  int dbindex = dataset - 1;
  struct dataitem *datasetmap = (struct dataitem *)dbmng->mmaps[dbindex];
  int mmaplength = dbmng->sizes[dbindex] / sizeof(struct dataitem);
  for (int i = 0; i < mmaplength; i++)
  {
    int offset;
    insert_key(idxlist, datasetmap[i].key, &offset);
  }
}

void get_value_by_offset(const dataset_mng *dbmng, const int dataset, int offset, char value[], int *key)
{
  if (dataset < 1 || dataset > dbmng->dcount)
  {
    fprintf(stderr, "get_value_by_offset: dataset number invalid\n");
    return;
  }

  // define the dataitem
  struct dataitem
  {
    int key;
    char value[dbmng->vsize];
  };

  int dbindex = dataset - 1;
  struct dataitem *datasetmap = (struct dataitem *)dbmng->mmaps[dbindex];
  int mmaplength = dbmng->sizes[dbindex] / sizeof(struct dataitem);
  if (offset >= mmaplength)
  {
    fprintf(stderr, "get_value_by_offset: offset %d cannot be greater than mmaplength %d (dataset %d)\n", offset, mmaplength, dataset);
    return;
  }

  struct dataitem item;
  memcpy(&item, (datasetmap + offset), sizeof(struct dataitem));
#ifdef DEBUG
  printf("{ %d : %s (vsize: %ld) }\n", item.key, item.value, sizeof(item.value));
#endif
  memset(value, '\0', dbmng->vsize);
  strncpy(value, item.value, dbmng->vsize);
  value[dbmng->vsize - 1] = '\0';
  *key = item.key;
}

#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include"serverk_args.h"
#include"index_list.h"

#define DEBUG

void make_datasets(const char* fname, const int dcount) {
  int number = dcount, count = 0;
  while (number > 0) {
    number /= 10;
    count++;
  }

  int index = 0;
  while (fname[index] != '\0') {
    count++;
    index++;
  }

  char filename[count + sizeof(".bin")]; // include null character
  for (int i = 1; i <= dcount; i++) {
    for (int j = 0; j < count+1; j++) {
      filename[j] = '\0';
    }
    sprintf(filename, "%s%d.bin", fname, i);
    // if (access(filename, F_OK) == 0) {
    //   /* the binary file exists */
    //   /* put existing file logic here */
    //   #ifdef DEBUG
    //   printf("%s already exists\n", filename);
    //   #endif
    // } else {
      #ifdef DEBUG
      printf("%s was created\n", filename);
      #endif
      FILE* tempPtr = fopen(filename, "wb"); // open for write bytes
      fclose(tempPtr);
    // }
  }
}

void write_content(const char* fname, const int dcount, const size_t vsize) {
  typedef struct {
    int key;
    char value[vsize];
  } dataitem;

  int number = dcount, count = 0;
  while (number > 0) {
    number /= 10;
    count++;
  }

  int index = 0;
  while (fname[index] != '\0') {
    count++;
    index++;
  }

  char filename[count + sizeof(".bin")]; // include null character
  for (int i = 1; i <= 1000; i++) {
    int fileNumber = (i % dcount) + 1;
    sprintf(filename, "%s%d.bin", fname, fileNumber);
    FILE* fptr = fopen(filename, "ab");
    dataitem item;
    item.key = i;
    switch(i % 5) {
      case 0:
      strcpy(item.value, "THIS");
      break;
      case 1:
      strcpy(item.value, "HELO");
      break;
      case 2:
      strcpy(item.value, "HEHE");
      break;
      case 3:
      strcpy(item.value, "OKAY");
      break;
      case 4:
      strcpy(item.value, "JOKE");
      break;
    }
    fwrite(&item, sizeof(dataitem), 1 , fptr);
    fclose(fptr);
  }
}

char* read_value(const int key, const char* fname, const int dcount, const size_t vsize) {
  typedef struct {
    int key;
    char value[vsize];
  } dataitem;

  int number = dcount, count = 0;
  while (number > 0) {
    number /= 10;
    count++;
  }

  int index = 0;
  while (fname[index] != '\0') {
    count++;
    index++;
  }

  char filename[count + sizeof(".bin")]; // include null character
  int fileNumber = (key % dcount) + 1;
  sprintf(filename, "%s%d.bin", fname, fileNumber);
  FILE* fptr = fopen(filename, "rb");

  dataitem item;
  while(fread(&item,  sizeof(dataitem), 1, fptr) == 1) {
    if (item.key == key) {
      char* value = (char*)malloc(sizeof(item.value));
      strcpy(value, item.value);
      return value;
    }
  }

  return NULL;
}

int main(const int argc, const char* argv[]) {
  cmd_args* args = new_cmdargs();
  if (!validate_cmdargs(args, argc, argv)) {
    return EXIT_FAILURE;
  }  
  #ifdef DEBUG
  print_cmdargs(args);
  #endif

  // make the dataset binary files if they do not exist
  make_datasets(args->fname, args->dcount);
  // write_content(args->fname, args->dcount, args->vsize);

  free_cmdargs(args);  
  return EXIT_SUCCESS;
}
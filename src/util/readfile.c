/* readFile.c */


#include "panic.h"
#include "readfile.h"

static size_t get_filesize(FILE* file) {
  fseek(file, 0, SEEK_END);
  size_t filesize = ftell(file);
  fseek(file, 0, SEEK_SET);
  return filesize + 1;
}

char* read_file(const char* filename) {
  FILE* file = fopen(filename, "r");
  if(!file) panic(1, "Failed to open file");

  size_t filesize = get_filesize(file);
  char* program = calloc(filesize, sizeof(char));

  fread(program, filesize, sizeof(char), file);
  fclose(file);

  return program;
}

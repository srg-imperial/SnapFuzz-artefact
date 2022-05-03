#include <assert.h>
#include <stdio.h>
#include <string.h>

// gcc benchmark-fs.c -o benchmark-fs
// time ../../build/sabre ../../build/plugins/sbr-afl/libsbr-afl.so -- \
// ./benchmark-fs ./fftplog
// time ./benchmark-fs ./fftplog

#define CREATE_DELETE_REPEATS 10000
#define RW_REPEATS 10000

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    puts("You forgot to give a path.");
    return 1;
  }

  FILE *fp;
  const char *filename = argv[1];
  char buff[255];
  char const_str[] = "This is testing for fputs...\n";

  for (size_t i = 0; i < CREATE_DELETE_REPEATS; i++) {
    fp = fopen(filename, "w+");

    for (size_t i = 0; i < RW_REPEATS; i++) {
      fputs(const_str, fp);
    }

    rewind(fp);

    for (size_t g = 0; g < RW_REPEATS; g++) {
      fgets(buff, 255, fp);
      // assert(strcmp(buff, const_str) == 0);
    }

    fclose(fp);

    int rc = remove(filename);
    // assert(rc == 0);
  }

  return 0;
}

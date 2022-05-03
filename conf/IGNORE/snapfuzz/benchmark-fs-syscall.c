#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// gcc benchmark-fs.c -o benchmark-fs
// time ../../build/sabre ../../build/plugins/sbr-afl/libsbr-afl.so -- \
// ./benchmark-fs ./fftplog
// time ./benchmark-fs ./fftplog

#define CREATE_DELETE_REPEATS 1000
#define RW_REPEATS 10000

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    puts("You forgot to give a path.");
    return 1;
  }

  const char *filename = argv[1];
  char buff[255];
  char const_str[] = "This is testing for fputs...\n";

  for (size_t i = 0; i < CREATE_DELETE_REPEATS; i++) {
    int fd = open(filename, O_RDWR | O_CREAT, 0777);

    for (size_t i = 0; i < RW_REPEATS; i++) {
      write(fd, const_str, sizeof(const_str));
    }

    lseek(fd, 0, SEEK_SET);

    for (size_t g = 0; g < RW_REPEATS; g++) {
      read(fd, buff, sizeof(const_str));
      // assert(strcmp(buff, const_str) == 0);
    }

    close(fd);

    int rc = remove(filename);
    // assert(rc == 0);
  }

  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Init: echo -n "hello" > mydb.txt
// Build 1: clang -g -w test_files.c -o test_files
// Build 2: AFL_HARDEN=1 ../afl/afl-clang-fast -g -w test_files.c -o test_files
// Run: ../afl/afl-fuzz -i in -o out ./test_files

int main(int argc, char *argv[]) {
  FILE *f = fopen("mydb.txt", "rb+");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  rewind(f);

  char fstring[50];
  fread(fstring, sizeof(char), fsize, f);
  fstring[fsize] = '\0';
  printf("DB content Begining: %s\n", fstring);

  // #ifdef __AFL_HAVE_MANUAL_CONTROL
  // __AFL_INIT();
  // #endif

  // Read stdin and AFL should start from here
  char input[100] = {0};
  if (read(STDIN_FILENO, input, 100) < 0) {
    fprintf(stderr, "Couldn't read stdin.\n");
  }

  if (strcmp(fstring, "hello") == 0 && input[0] == 'a') {
    // Here is a bug
    printf("Bug found!\n");
    fflush(stdout);
    *(char *)1 = 2;
  } else {
    // Oh! File is written!
    rewind(f);
    char *n = "goodbye";
    fwrite(n, sizeof(char), strlen(n), f);
  }

  fseek(f, 0, SEEK_END);
  fsize = ftell(f);
  rewind(f);
  fread(fstring, sizeof(char), fsize, f);
  printf("DB content End: %s\n", fstring);

  fclose(f);
  return 0;
}

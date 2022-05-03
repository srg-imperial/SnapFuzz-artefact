#include <stdbool.h>
#include <stdio.h>

// export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/SaBRe/tasos_tmp_tests/snapfuzz
// gcc -g -O0 learn_tls.c -o learn_tls -llearn_tls -L.

extern bool lala();
extern void lalatrue();
extern void lalafalse();

static _Thread_local bool from_plugin = false;

int main() {
  from_plugin = true;
  printf("la %d\n", from_plugin);
  from_plugin = false;
  printf("la %d\n", from_plugin);

  lalatrue();
  printf("la %d\n", lala());
  lalafalse();
  printf("la %d\n", lala());
  return 0;
}

#include <stdbool.h>
#include <stdio.h>

// gcc -fPIC -c liblearn_tls.c
// gcc -shared -o liblearn_tls.so liblearn_tls.o
// Single:
// gcc -g -O0 -shared -fPIC -ftls-model=local-dynamic -o liblearn_tls.so liblearn_tls.c
// gcc -g -O0 -shared -fPIC -ftls-model=initial-exec -o liblearn_tls.so liblearn_tls.c

static _Thread_local bool local1 = false; // 0x7ffff7fe673e

bool lala() { return local1; }

void lalatrue() { local1 = true; }

void lalafalse() { local1 = false; }

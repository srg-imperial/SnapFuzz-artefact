#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// gcc -fsanitize=address -g test-vdso-preinit.c -o test-vdso-preinit && ./test-vdso-preinit

// vDSO cannot be used in preinit: https://reviews.llvm.org/D40679
//
// vDSO are initialized in __vdso_platform_setup () at
// ../sysdeps/unix/sysv/linux/x86_64/init-first.c:36
//
// Newer glibc 2.31 fixes this issue:
// 1) https://sourceware.org/bugzilla/show_bug.cgi?id=24967
// 2) https://sourceware.org/git/?p=glibc.git;a=commitdiff;h=1bdda52fe92fd01b424cd6fbb63e3df96a95015c
// 3) https://sourceware.org/pipermail/glibc-cvs/2020q1/068454.html

void my_preinit_array(int argc, char *argv[], char *env[]) {
  struct timespec start;
  if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
    perror("clock gettime");
    exit(EXIT_FAILURE);
  }
  printf("Called my_preinit_array() and %ld\n", start.tv_sec);
}

__attribute__((section(".preinit_array"),
               used)) typeof(my_preinit_array) *my_preinit_array_p =
    my_preinit_array;

int main(void) { printf("Called main()\n"); }

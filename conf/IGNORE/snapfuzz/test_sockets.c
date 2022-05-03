// #ifdef __AFL_HAVE_MANUAL_CONTROL
//   __AFL_INIT();
// #endif

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define INVALID_SOCKET -1

// Build 1: clang -g -w socket-test.c -o socket-test
// Build 2: AFL_HARDEN=1 ../afl/afl-clang-fast -g -w socket-test.c -o
// socket-test
//
// Run:  echo "TEST" | LD_PRELOAD=/vagrant/preeny/src/desock.so ./socket-test

int main() {
  int listensocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listensocket == INVALID_SOCKET) {
    perror("Socket create error");
    return 0;
  }

  int rv = 1;
  setsockopt(listensocket, SOL_SOCKET, SO_REUSEADDR, &rv, sizeof(rv));

  struct sockaddr_in laddr = {0};
  laddr.sin_family = AF_INET;
  laddr.sin_port = htons(2432);
  laddr.sin_addr.s_addr = htonl(INADDR_ANY);
  int rc = bind(listensocket, (struct sockaddr *)&laddr, sizeof(laddr));
  if (rc != 0) {
    perror("bind()");
    exit(EXIT_FAILURE);
  }

  rc = listen(listensocket, SOMAXCONN);
  if (rc != 0) {
    perror("listen()");
    exit(EXIT_FAILURE);
  }

  // If no pending connections are present on the queue, and the socket is
  // not marked as nonblocking, accept() blocks the caller until a
  // connection is present.  If the socket is marked nonblocking and no
  // pending connections are present on the queue, accept() fails with the
  // error EAGAIN or EWOULDBLOCK.
  int datasocket = accept(listensocket, NULL, NULL);
  if (datasocket == INVALID_SOCKET) {
    perror("Socket create error");
    return 0;
  }

  struct sockaddr_in clientaddr = {0};
  socklen_t asz = sizeof(clientaddr);
  rc = getsockname(datasocket, (struct sockaddr *)&clientaddr, &asz);
  if (rc != 0) {
    perror("getsockname()");
    exit(EXIT_FAILURE);
  }

  printf("getsockname: %s %d\n", inet_ntoa(clientaddr.sin_addr),
         clientaddr.sin_port);

  memset(&clientaddr, 0, sizeof(asz));
  rc = getpeername(datasocket, (struct sockaddr *)&clientaddr, &asz);
  if (rc != 0) {
    perror("getpeername()");
    exit(EXIT_FAILURE);
  }

  printf("getpeername: %s %d\n", inet_ntoa(clientaddr.sin_addr),
         clientaddr.sin_port);

  assert(send(datasocket, "HI!\n", 4, 0) == 4); // sys: sendto

  char buf[1024] = {0};
  recv(datasocket, buf, 1024, 0); // sys: recvfrom
  printf("You wrote: %s", buf);

  close(datasocket);
  close(listensocket);
}

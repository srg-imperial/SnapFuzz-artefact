#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DATA1 "In Xanadu, did Kublai Khan . . ."
#define DATA2 "A stately pleasure dome decree . . ."

/*
 * This program creates a pair of connected sockets, then forks and
 * communicates over them.  This is very similar to communication with pipes;
 * however, socketpairs are two-way communications objects. Therefore,
 * this program can send messages in both directions.
 */

int main() {
  int sockets[2], child;
  char buf[1024];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    perror("opening stream socket pair");
    exit(1);
  }

  if ((child = fork()) == -1)
    perror("fork");
  else if (child) { /* This is the parent. */
    close(sockets[0]);

    for (size_t i = 0; i < 3; i++) {
      if (recv(sockets[1], buf, 1024, 0) < 0)
        perror("reading stream message");
      printf("-->%s\n", buf);

      if (send(sockets[1], DATA2, sizeof(DATA2), 0) < 0)
        perror("writing stream message");
    }

    close(sockets[1]);
  } else { /* This is the child. */
    close(sockets[1]);

    for (size_t i = 0; i < 3; i++) {
      if (send(sockets[0], DATA1, sizeof(DATA1), 0) < 0)
        perror("writing stream message");

      if (recv(sockets[0], buf, 1024, 0) < 0)
        perror("reading stream message");
      printf("-->%s\n", buf);
    }

    close(sockets[0]);
  }
}

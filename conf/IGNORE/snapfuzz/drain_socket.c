#include <assert.h>
#include <errno.h>
#include <linux/sockios.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define DATA2 "A stately pleasure dome decree . . ."

int sockets[2], child;
char buf[1024] = {0};

void child_process() {
  int rc = recv(sockets[0], buf, 1024, MSG_DONTWAIT);
  assert(rc == 37);

  printf("Child got it! %d\n", rc);
}

int main(int argc, char const *argv[]) {

  if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, sockets) < 0) {
    perror("opening stream socket pair");
    exit(1);
  }

  if (fork() == 0) {
    child_process();
    return 0;
  }

  if (send(sockets[1], DATA2, sizeof(DATA2), MSG_DONTWAIT) < 0) {
    perror("writing stream message");
    return 1;
  }
  if (send(sockets[1], DATA2, sizeof(DATA2), MSG_DONTWAIT) < 0) {
    perror("writing stream message");
    return 1;
  }

  int value;
  int error = ioctl(sockets[0], SIOCINQ, &value);
  printf("value: %d\n", value);

  error = ioctl(sockets[1], SIOCINQ, &value);
  printf("value: %d\n", value);

  sleep(1);

  int rc = recv(sockets[0], buf, 1024, MSG_DONTWAIT);
  assert(rc == 37);
  printf("Main got it! %d\n", rc);

  return 0;
}

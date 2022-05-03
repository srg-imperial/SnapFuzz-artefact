#include <assert.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define DATA1 "In Xanadu, did Kublai Khan . . ."
#define DATA2 "A stately pleasure dome decree . . ."

// The following indeed doesn't block on parent's recv.
void scenario1() {
  int sockets[2], child;
  char buf[1024] = {0};

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    perror("opening stream socket pair");
    exit(1);
  }

  if ((child = fork()) == -1)
    perror("fork");
  else if (child) { /* This is the parent. */
    close(sockets[0]);
    struct timeval timeout = {.tv_sec = 0, .tv_usec = 100};
    setsockopt(sockets[1], SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
               sizeof(timeout));

    if (recv(sockets[1], buf, 1024, 0) < 0)
      perror("reading stream message");
    printf("-->%s\n", buf);

    if (send(sockets[1], DATA2, sizeof(DATA2), 0) < 0)
      perror("writing stream message");

    close(sockets[1]);
  } else { /* This is the child. */
    close(sockets[1]);

    // There is a missing send here.

    if (recv(sockets[0], buf, 1024, 0) < 0)
      perror("reading stream message");
    printf("-->%s\n", buf);

    close(sockets[0]);
  }
}

// Multiple sends are combined into a single recv.
void scenario2() {
  int sockets[2], child;
  char buf[1024] = {0};

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    perror("opening stream socket pair");
    exit(1);
  }

  if ((child = fork()) == -1)
    perror("fork");
  else if (child) { /* This is the parent. */
    close(sockets[0]);

    if (send(sockets[1], DATA2, sizeof(DATA2), 0) < 0)
      perror("writing stream message");
    if (send(sockets[1], DATA2, sizeof(DATA2), 0) < 0)
      perror("writing stream message");
    if (send(sockets[1], DATA2, sizeof(DATA2), 0) < 0)
      perror("writing stream message");

    close(sockets[1]);
  } else { /* This is the child. */
    close(sockets[1]);

    usleep(100);

    int rc = recv(sockets[0], buf, 1024, 0);
    if (rc < 0)
      perror("reading stream message");

    assert(rc == sizeof(DATA2) * 3);
    printf("--> Bytes gotten: %d\n", rc);

    close(sockets[0]);
  }
}

// After a socket is closed, recv returns always zero. If the parent sleeps,
// the chield is blocked after the first recv.
// https://stackoverflow.com/questions/3091010/recv-socket-function-returning-data-with-length-as-0
void scenario3() {
  int sockets[2], child;
  char buf[1024] = {0};

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    perror("opening stream socket pair");
    exit(1);
  }

  if ((child = fork()) == -1)
    perror("fork");
  else if (child) { /* This is the parent. */
    close(sockets[0]);

    if (send(sockets[1], DATA2, sizeof(DATA2), 0) < 0)
      perror("writing stream message");
    if (send(sockets[1], DATA2, sizeof(DATA2), 0) < 0)
      perror("writing stream message");
    if (send(sockets[1], DATA2, sizeof(DATA2), 0) < 0)
      perror("writing stream message");
    printf("--> Send: made 3 sends\n");
    fflush(stdout);

    close(sockets[1]);
  } else { /* This is the child. */
    close(sockets[1]);

    usleep(100);

    int rc = recv(sockets[0], buf, 1024, 0);
    if (rc < 0)
      perror("reading stream message");
    assert(rc == sizeof(DATA2) * 3);
    printf("--> Bytes gotten: %d\n", rc);

    rc = recv(sockets[0], buf, 1024, 0);
    if (rc < 0)
      perror("reading stream message");
    assert(rc == 0);
    printf("--> Bytes gotten: %d\n", rc);

    close(sockets[0]);
  }
}

// Does setsockopt on the parent interfer with the socket options on the child?
// Ans: No
void scenario4() {
  int sockets[2], child;
  char buf[1024] = {0};

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    perror("opening stream socket pair");
    exit(1);
  }

  if ((child = fork()) == -1)
    perror("fork");
  else if (child) { /* This is the parent. */
    close(sockets[0]);
    struct timeval timeout = {.tv_sec = 0, .tv_usec = 100};
    setsockopt(sockets[1], SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
               sizeof(timeout));

    if (recv(sockets[1], buf, 1024, 0) < 0)
      perror("reading stream message");
    printf("-->%s\n", buf);

    if (send(sockets[1], DATA2, sizeof(DATA2), 0) < 0)
      perror("writing stream message");

    sleep(1);

    close(sockets[1]);
  } else { /* This is the child. */
    close(sockets[1]);

    // There is a missing send here.

    if (recv(sockets[0], buf, 1024, 0) < 0)
      perror("reading stream message");
    printf("-->%s\n", buf);

    if (recv(sockets[0], buf, 1024, 0) < 0)
      perror("reading stream message");
    printf("-->%s\n", buf);

    close(sockets[0]);
  }
}

// The following can produce this problematic scenario:
// poll: skipped recv
// poll: sent some data
// poll: skipped recv
// poll: sent some data
// poll: skipped recv
// poll: sent some data
// -->A stately pleasure dome decree . . .
// In the above example recv on the parent stays open for quite a long time.
// This won't work for us as we will send many times the same command.
//
// MSG_NOSIGNAL is required.
void scenario5() {
  int sockets[2], child;
  char buf[1024] = {0};

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    perror("opening stream socket pair");
    exit(1);
  }

  if ((child = fork()) == -1)
    perror("fork");
  else if (child) { /* This is the parent. */
    close(sockets[0]);

    if (recv(sockets[1], buf, 1024, 0) < 0)
      perror("reading stream message");
    printf("-->%s\n", buf);

    if (send(sockets[1], DATA2, sizeof(DATA2), 0) < 0)
      perror("writing stream message");

    close(sockets[1]);
  } else { /* This is the child. */
    close(sockets[1]);

    struct pollfd pfd[1] = {{.fd = sockets[0], .events = POLLOUT | POLLIN}};

    for (int timeouts = 0; timeouts < 3; timeouts++) {
      int rv = poll(pfd, 1, 100);

      if (rv == 0) {
        printf("poll: timeout\n");
        continue;
      } else if (rv < 0) { // an error was returned
        perror("poll");
        return;
      }

      if (pfd[0].revents & POLLIN) {
        if (recv(sockets[0], buf, 1024, 0) < 0)
          perror("reading stream message");
        printf("poll: received some data\n");
      } else {
        printf("poll: skipped recv\n");
      }

      if (pfd[0].revents & POLLOUT) {
        if (send(sockets[0], DATA2, sizeof(DATA2), MSG_NOSIGNAL) < 0)
          perror("writing stream message");
        else {
          printf("poll: sent some data\n");
          // usleep(100); // Uncomment this to see different behavior.
        }
      } else {
        printf("poll: skipped send\n");
      }
    }

    fflush(stdout);
    close(sockets[0]);
  }
}

void scenario6() {
  int sockets[2], child;
  char buf[1024] = {0};

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    perror("opening stream socket pair");
    exit(1);
  }

  if ((child = fork()) == -1)
    perror("fork");
  else if (child) { /* This is the parent. */
    close(sockets[0]);

    usleep(100);

    printf("--> Parent: I'm sending!\n");
    fflush(stdout);

    if (send(sockets[1], DATA2, sizeof(DATA2), MSG_NOSIGNAL) < 0)
      perror("Parent: writing stream message");

    printf("--> Parent: I'm receiving!\n");
    fflush(stdout);

    // if (recv(sockets[1], buf, 1024, 0) < 0)
    //   perror("Parent: reading stream message");
    // else
    //   printf("--> Parent: %s\n", buf);

    printf("--> Parent: I'm done!\n");
    fflush(stdout);

    sleep(2);
    close(sockets[1]);
  } else { /* This is the child. */
    close(sockets[1]);

    struct pollfd pfd[1] = {{.fd = sockets[0], .events = POLLOUT | POLLIN}};

    for (int timeouts = 0; timeouts < 10; timeouts++) {
      int rv = poll(pfd, 1, 100);

      if (rv == 0) {
        printf("poll: timeout\n");
        continue;
      } else if (rv < 0) { // an error was returned
        perror("poll");
        return;
      }

      if (pfd[0].revents & POLLIN) {
        if (recv(sockets[0], buf, 1024, 0) < 0)
          perror("reading stream message");
        else
          printf("poll: received some data\n");
      } else
        printf("poll: skipped recv\n");

      if (pfd[0].revents & POLLOUT) {
        if (send(sockets[0], DATA2, sizeof(DATA2), MSG_NOSIGNAL) < 0)
          perror("writing stream message");
        else
          printf("poll: sent some data\n");
      } else
        printf("poll: skipped send\n");
    }

    fflush(stdout);
    close(sockets[0]);
  }
}

// This scenario shows what will happen if we submit the same fd twice.
// ans: The first occurnace keeps tha value?
// We can use -1 as the fd.
void scenario7() {
  int sockets[2], child;
  char buf[1024] = {0};

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    perror("opening stream socket pair");
    exit(1);
  }

  if ((child = fork()) == -1)
    perror("fork");
  else if (child) { /* This is the parent. */
    close(sockets[0]);

    if (send(sockets[1], DATA2, sizeof(DATA2), 0) < 0)
      perror("writing stream message");

    close(sockets[1]);
  } else { /* This is the child. */
    close(sockets[1]);

    struct pollfd pfd[2] = {{.fd = sockets[0], .events = POLLOUT | POLLIN},
                            {.fd = -1, .events = POLLOUT | POLLIN}};

    for (int timeouts = 0; timeouts < 3; timeouts++) {
      int rv = poll(pfd, 1, 100);

      if (rv == 0) {
        printf("poll: timeout\n");
        continue;
      } else if (rv < 0) { // an error was returned
        perror("poll");
        return;
      }

      printf("poll: rv = %d\n", rv);
      printf(" %d %d\n", pfd[0].revents, pfd[1].revents);

      if (pfd[0].revents & POLLIN) {
        if (recv(sockets[0], buf, 1024, 0) < 0)
          perror("reading stream message");
        printf("poll: received some data\n");
      } else {
        printf("poll: skipped recv\n");
      }
    }

    fflush(stdout);
    close(sockets[0]);
  }
}

// Q1: Both sends and receives from both threads are non-blocking. what will
// happen?
// Q2: Do i need to connect?

// Curious case: if socket is closed, poll will return events and we need
// to check send's errno and receieve will always return 0 len strings.

int main() {
  scenario7();

  while (wait(NULL) > 0)
    ;

  return 0;
}

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define MAXBUF 256

void child_process(void) {
  char msg[MAXBUF];
  struct sockaddr_in addr = {0};
  int n, sockfd, num = 1;

  /* Create socket and connect to server */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(2000);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

  printf("child {%d} connected \n", getpid());
  num++;
  sprintf(msg, "Test message %d from client %d", num, getpid());
  n = send(sockfd, msg, strlen(msg), 0); /* Send message */
}

// void setnonblocking(int sockfd) {
//   int opts;

//   opts = fcntl(sockfd, F_GETFL);
//   if (opts < 0) {
//     perror("fcntl(F_GETFL)");
//     exit(EXIT_FAILURE);
//   }
//   opts = (opts | O_NONBLOCK);
//   if (fcntl(sockfd, F_SETFL, opts) < 0) {
//     perror("fcntl(F_SETFL)");
//     exit(EXIT_FAILURE);
//   }
//   return;
// }

int main() {
  char buffer[MAXBUF];
  int fds[5];
  struct sockaddr_in addr;
  struct sockaddr_in client;
  int n, i, max = 0;
  int sockfd, commfd;
  fd_set rset;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  /* So that we can re-bind to it without TIME_WAIT problems */
  int reuse_addr = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
  // setnonblocking(sockfd);

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(2000);
  addr.sin_addr.s_addr = INADDR_ANY;
  bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
  listen(sockfd, 5);

  // Create clients
  for (i = 0; i < 5; i++) {
    if (fork() == 0) {
      child_process();
      return 0;
    }
  }

  for (i = 0; i < 5; i++) {
    memset(&client, 0, sizeof(client));
    uint addrlen = sizeof(client);
    fds[i] = accept(sockfd, (struct sockaddr *)&client, &addrlen);
    if (fds[i] > max)
      max = fds[i];
  }

  int msgs = 0;
  while (1) {
    FD_ZERO(&rset);
    for (i = 0; i < 5; i++) {
      FD_SET(fds[i], &rset);
    }

    puts("round again");
    select(max + 1, &rset, NULL, NULL, NULL);

    for (i = 0; i < 5; i++) {
      if (FD_ISSET(fds[i], &rset)) {
        memset(buffer, 0, MAXBUF);
        recv(fds[i], buffer, MAXBUF, 0);
        puts(buffer);
        msgs++;
      }
    }

    if (msgs >= 5) {
      puts("I'm done!");
      break;
    }
  }
  return 0;
}

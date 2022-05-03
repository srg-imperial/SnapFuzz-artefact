#include <pthread.h>
#include <unistd.h>
#include <wait.h>

void *threadFunc(void *arg) {
  *(int *)0 = 0;
  return NULL;
}

void child_process() {
  pthread_t threadId;
  pthread_create(&threadId, NULL, &threadFunc, NULL);
  pthread_join(threadId, NULL);
}

int main() {
  if (fork() == 0) {
    child_process();
    return 0;
  }

  while (wait(NULL) > 0)
    ;

  return 0;
}

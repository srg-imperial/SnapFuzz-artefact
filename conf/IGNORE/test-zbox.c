#include <assert.h>
#include <fcntl.h>
#include <libgen.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zbox.h>

zbox_file file;

void *thread_f(void *ignored) {
  char buf[] = "24-05-2020 04:06:27 : 220 LightFTP server v2.0a ready\r\n\r\n";
  zbox_file_write(file, buf, strlen(buf));

  printf("Hello from thread\n");
  return NULL;
}

int main(int argc, char const *argv[]) {

  char pathname[] = "/home/vagrant/fftplog/lala";

  int ret = zbox_init_env();
  assert(!ret);

  // opener
  zbox_opener opener = zbox_create_opener();
  zbox_opener_ops_limit(opener, ZBOX_OPS_INTERACTIVE);
  zbox_opener_mem_limit(opener, ZBOX_MEM_INTERACTIVE);
  zbox_opener_cipher(opener, ZBOX_CIPHER_XCHACHA);
  zbox_opener_create(opener, true);
  zbox_opener_version_limit(opener, 1);

  // open repo
  zbox_repo repo;
  ret = zbox_open_repo(&repo, opener, "mem://sabre", "password");
  assert(!ret);
  zbox_free_opener(opener);

  // zbox_file file;

  if (zbox_repo_path_exists(repo, pathname)) {
    // open the existing file
    int ret = zbox_repo_open_file(&file, repo, pathname);
    assert(!ret);
  } else {
    // create file
    char *pathname_dup = strdup(pathname);
    assert(pathname_dup != NULL);

    int ret = zbox_repo_create_dir_all(repo, dirname(pathname_dup));
    assert(!ret);
    free(pathname_dup);

    ret = zbox_repo_create_file(&file, repo, pathname);
    assert(!ret);
  }

  // char buf[] = "24-05-2020 04:06:27 : 220 LightFTP server v2.0a
  // ready\r\n\r\n"; zbox_file_write(file, buf, strlen(buf));

  pthread_t thread;

  /* create a second thread which executes inc_x(&x) */
  if (pthread_create(&thread, NULL, thread_f, NULL)) {
    printf("Error creating thread\n");
    return 1;
  }

  /* wait for the second thread to finish */
  if (pthread_join(thread, NULL)) {
    printf("Error joining thread\n");
    return 2;
  }

  printf("Hello from main\n");

  return 0;
}

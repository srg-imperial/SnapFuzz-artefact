#include <assert.h>
#include <fcntl.h>
#include <libgen.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zbox.h>

// gcc test-zbox.c -o test-zbox -lzbox
// gcc -g -fsanitize=address,undefined test-zbox.c -o test-zbox -lzbox &&
// ./test-zbox

int main(int argc, char const *argv[]) {

  char pathname[] = "/home/vagrant/fftplog";

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

  zbox_file file;

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

  // If file exists in the real FS we need to copy it's content.
  char *buf;
  bool file_found = false;
  if (access(pathname, F_OK) != -1) {
    file_found = true;

    // TODO:
    // https://eklausmeier.wordpress.com/2016/02/03/performance-comparison-mmap-versus-read-versus-fread/
    FILE *real_file = fopen(pathname, "rb");
    fseek(real_file, 0, SEEK_END);
    long fsize = ftell(real_file);
    rewind(real_file);
    buf = (char *)malloc(sizeof(char) * (fsize + 1));
    size_t result = fread(buf, sizeof(char), fsize, real_file);
    assert(result == fsize);
    buf[fsize] = '\0';
    fclose(real_file);

    ret = zbox_file_write(file, (const unsigned char *)buf, fsize);
    assert(ret == fsize);
    ret = zbox_file_finish(file);
    assert(!ret);
  }
  assert(file_found);

  ret = zbox_file_seek(file, 0, SEEK_SET);
  assert(ret == 0);

  int bytes_to_read = 20;
  uint8_t dst[100] = {0};
  ret = zbox_file_read(dst, bytes_to_read, file);
  assert(ret == bytes_to_read);
  assert(!memcmp(dst, buf, bytes_to_read));

  free(buf);
  zbox_close_file(file);
  zbox_close_repo(repo);

  return 0;
}

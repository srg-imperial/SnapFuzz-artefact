#include <assert.h>
#include <limits.h>
#include <sqlfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// gcc test-libsqlfs-itself.c -o test-libsqlfs-itself -lsqlfs-1.0

void randomfilename(char *buf, int size, char *prefix) {
  snprintf(buf, size, "/%s-random-%i", prefix, rand());
}

void test_write_n_bytes(sqlfs_t *sqlfs, int testsize) {
  printf("Testing writing %d bytes of data...", testsize);
  int i;
  char testfilename[PATH_MAX];
  char randombuf[testsize];
  char randomdata[testsize];
  struct fuse_file_info fi = {0};
  randomfilename(testfilename, PATH_MAX, "write_n_bytes");
  for (i = 0; i < testsize; ++i)
    randomdata[i] = (i % 90) + 32;
  randomdata[testsize - 1] = 0;
  sqlfs_proc_write(sqlfs, testfilename, randomdata, testsize, 0, &fi);
  // sleep(1);
  i = sqlfs_proc_read(sqlfs, testfilename, randombuf, testsize, 0, &fi);
  randombuf[i] = 0;
  assert(!strcmp(randombuf, randomdata));
  printf("passed\n");
}

static int fill_dir(void *buf, const char *name, const struct stat *statp,
                    off_t off) {
  printf("%s, %s\n", (char *)buf, name);
  fflush(stdout);
  return 0;
}

void test_mkdir_without_sleep(sqlfs_t *sqlfs) {
  printf("Testing mkdir without sleep...");
  char *testfilename = "/mkdir-without-sleep0";
  sqlfs_proc_mkdir(sqlfs, testfilename, 0777);
  assert(sqlfs_is_dir(sqlfs, testfilename));
  printf("passed\n");

  printf("Testing whether mkdir does not make nested dirs...");
  testfilename = "/a/b/c/d/e/f/g";
  int rc = sqlfs_proc_mkdir(sqlfs, testfilename, 0777);

  char lala[1000] = {0};
  rc = sqlfs_proc_readdir(sqlfs, testfilename, lala, (fuse_fill_dir_t)fill_dir,
                          0, NULL);

  assert(sqlfs_is_dir(sqlfs, testfilename) == 1);
  printf("passed\n");
}

int main(int argc, char const *argv[]) {
  char *database_filename = ":memory:";
  int rc;
  sqlfs_t *sqlfs = 0;

  printf("Opening %s...", database_filename);
  rc = sqlfs_open(database_filename, &sqlfs);
  assert(rc);
  assert(sqlfs != 0);
  printf("passed\n");

  test_mkdir_without_sleep(sqlfs);
  test_write_n_bytes(sqlfs, 10000);

  printf("Closing database...");
  assert(sqlfs_close(sqlfs));
  printf("passed\n");

  return 0;
}

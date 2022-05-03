#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

static char mydir[] = "./somedirectory";
static char myfile[] = "./somefile";

static void print_stat(struct stat sb) {
  printf("-----------------------------------------\n");
  printf("ID of containing device:  [%lx,%lx]\n", (long)major(sb.st_dev),
         (long)minor(sb.st_dev));

  printf("File type:                ");

  switch (sb.st_mode & S_IFMT) {
  case S_IFBLK:
    printf("block device\n");
    break;
  case S_IFCHR:
    printf("character device\n");
    break;
  case S_IFDIR:
    printf("directory\n");
    break;
  case S_IFIFO:
    printf("FIFO/pipe\n");
    break;
  case S_IFLNK:
    printf("symlink\n");
    break;
  case S_IFREG:
    printf("regular file\n");
    break;
  case S_IFSOCK:
    printf("socket\n");
    break;
  default:
    printf("unknown?\n");
    break;
  }

  printf("I-node number:            %ld\n", (long)sb.st_ino);

  printf("Mode:                     %lo (octal)\n", (unsigned long)sb.st_mode);

  printf("Link count:               %ld\n", (long)sb.st_nlink);
  printf("Ownership:                UID=%ld   GID=%ld\n", (long)sb.st_uid,
         (long)sb.st_gid);

  printf("Preferred I/O block size: %ld bytes\n", (long)sb.st_blksize);
  printf("File size:                %lld bytes\n", (long long)sb.st_size);
  printf("Blocks allocated:         %lld\n", (long long)sb.st_blocks);

  printf("Last status change:       %s", ctime(&sb.st_ctime));
  printf("Last file access:         %s", ctime(&sb.st_atime));
  printf("Last file modification:   %s", ctime(&sb.st_mtime));
}

void create_dir() {
  struct stat st = {0};
  int rc = stat(mydir, &st);
  assert(rc == -1);
  rc = mkdir(mydir, 0700);
  assert(rc == 0);
}

void open_dir() {
  DIR *dir = opendir(mydir);
  assert(dir != NULL);
  closedir(dir);
}

void check_dir_exists() {
  struct stat sb;
  int rc = stat(mydir, &sb);
  assert(S_ISDIR(sb.st_mode));
  assert(sb.st_nlink > 0);
  assert(rc == 0);
}

void delete_empty_dir() {
  int rc = rmdir(mydir);
  assert(rc == 0);

  struct stat sb;
  rc = stat(mydir, &sb);
  assert(rc == -1);
}

void create_file_and_fstat_it() {
  char fn[] = "./temp.file";
  struct stat sb;
  int file_descriptor;

  if ((file_descriptor = creat(fn, S_IWUSR)) < 0)
    perror("creat() error");
  else {
    if (fstat(file_descriptor, &sb) != 0)
      perror("fstat() error");
    else {
      assert(S_ISREG(sb.st_mode));
      // print_stat(sb);
    }
    close(file_descriptor);
    unlink(fn);
  }
}

void delete_file() {
  int rc = remove(myfile);
  assert(rc == 0);

  struct stat sb;
  rc = lstat(myfile, &sb);
  assert(rc == -1);
}

void create_file() {
  FILE *fp = fopen(myfile, "a");
  assert(fp != NULL);
  fclose(fp);
}

void stat_and_fstat_file() {
  struct stat sb;

  if (lstat(myfile, &sb) == -1) {
    perror("lstat");
    exit(EXIT_FAILURE);
  }
  assert(S_ISREG(sb.st_mode));
  // print_stat(sb);

  int fd = open(myfile, O_RDONLY);

  if (fstat(fd, &sb) == -1) {
    perror("lstat");
    exit(EXIT_FAILURE);
  }
  assert(S_ISREG(sb.st_mode));
  // print_stat(sb);

  close(fd);
}

void create_write_close_open_read_delete() {
  FILE *f = fopen(myfile, "w");
  if (f == NULL) {
    printf("Error opening file!\n");
    exit(EXIT_FAILURE);
  }
  fprintf(f, "Some text\n");
  fclose(f);

  FILE *fp;
  char buff[255];
  char *line = NULL;
  size_t len = 0;
  ssize_t read = 0;
  int times = 0;

  fp = fopen(myfile, "r");
  while ((read = getline(&line, &len, fp)) != -1) {
    assert(read == 10);
    assert(strcmp(line, "Some text\n") == 0);
    times++;
  }
  assert(times == 1);
  fclose(fp);

  int rc = remove(myfile);
  assert(rc == 0);

  struct stat sb;
  rc = lstat(myfile, &sb);
  assert(rc == -1);
}

void create_write_read_close_delete() {
  FILE *f = fopen(myfile, "w+");
  if (f == NULL) {
    printf("Error opening file!\n");
    exit(EXIT_FAILURE);
  }
  fprintf(f, "Some text\n");

  rewind(f);
  clearerr(f);

  // TODO: I couldn't make getline and fread to work...
  char buff[255] = {0};
  ssize_t read = pread(f->_fileno, buff, 255, 0);
  assert(read == 10);
  assert(strcmp(buff, "Some text\n") == 0);
  fclose(f);

  int rc = remove(myfile);
  assert(rc == 0);

  struct stat sb;
  rc = lstat(myfile, &sb);
  assert(rc == -1);
}

void multiple_opens() {
  FILE *f = fopen(myfile, "w");
  if (f == NULL) {
    printf("Error opening file!\n");
    exit(EXIT_FAILURE);
  }
  FILE *f2 = fopen(myfile, "w");
  if (f2 == NULL) {
    printf("Error opening file!\n");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char const *argv[]) {
  // TODO: Read file from real FS and check content, then edit content
  // TODO: List files in dir

  create_dir();
  check_dir_exists();
  delete_empty_dir();
  create_file_and_fstat_it();

  create_file();
  stat_and_fstat_file();
  delete_file();

  create_write_read_close_delete();
  create_write_close_open_read_delete();

  // XFAIL
  // open_dir();
  // multiple_opens();

  printf("-----------------------------------------\n");
  printf("Success\n");
  return 0;
}

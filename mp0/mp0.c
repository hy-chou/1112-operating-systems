#include "kernel/types.h"

#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

int occurrence(char *path, char *key) {
  int i = 0;
  int count = 0;

  while (path[i]) {
    if (path[i] == *key)
      count += 1;
    i += 1;
  }
  return count;
}

void traverse(char *path, char *key, int *dir_num, int *file_num, int is_root_directory) {
  char buf[512], *ptr;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, 0)) < 0) {
    // fprintf(2, "mp0: cannot open %s\n", path);
    printf("%s [error opening dir]\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    // fprintf(2, "mp0: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if (is_root_directory && st.type != T_DIR) {
    printf("%s [error opening dir]\n", path);
    close(fd);
    return;
  }

  if (st.type == T_FILE) {
    printf("%s %d\n", path, occurrence(path, key));
    *(file_num) += 1;
  } else if (st.type == T_DIR) {
    printf("%s %d\n", path, occurrence(path, key));
    if (! is_root_directory)
      *(dir_num) += 1;

    strcpy(buf, path);
    ptr = buf + strlen(buf);
    *(ptr++) = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {

      if (de.inum == 0)
        continue;
      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;
      memmove(ptr, de.name, DIRSIZ);
      ptr[DIRSIZ] = 0;
      if (stat(buf, &st) < 0) {
        // fprintf(2, "mp0: cannot stat %s\n", buf);
        continue;
      }

      strcpy(buf + strlen(buf) + 1, de.name);
      traverse(buf, key, dir_num, file_num, 0);
    }
  }

  close(fd);
  return;
}

int main(int argc, char *argv[]) {
  int pid;
  int fds[2];

  if (argc != 3) {
    // fprintf(2, "Usage: mp0 root_directory key\n");
    exit(1);
  }

  if (pipe(fds) < 0) {
      // fprintf(2, "mp0: pipe failed\n");
      exit(1);
  }

  //
  // STEP 2
  //
  pid = fork();
  if (pid == 0) {
    // child process starts
    int dir_num = 0;
    int file_num = 0;

    //
    // STEP 3
    //
    traverse(argv[1], argv[2], &dir_num, &file_num, 1);

    //
    // STEP 4
    //
    write(fds[1], &file_num, sizeof(int));
    write(fds[1], &dir_num, sizeof(int));

    exit(0);
    // child process ends
  } else if (pid > 0) {
    int dir_num;
    int file_num;

    //
    // STEP 5
    //
    wait(0);
    printf("\n");
    if (read(fds[0], &file_num, sizeof(int)) != sizeof(int)) {
        // fprintf(2, "mp0: pipe read failed (file_num)\n");
    }
    if (read(fds[0], &dir_num, sizeof(int)) != sizeof(int)) {
        // fprintf(2, "mp0: pipe read failed (dir_num)\n");
    }
    printf("%d directories, %d files\n", dir_num, file_num);
  } else {
    // fprintf(2, "mp0: fork failed\n");

    close(fds[0]);
    close(fds[1]);
    exit(1);
  }

  close(fds[0]);
  close(fds[1]);
  exit(0);
}

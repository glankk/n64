#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "gru.h"
#include "os.h"

static int asprintf(char **strp, const char *fmt, ...)
{
  va_list arg;
  va_start(arg, fmt);
  int size = vsnprintf(NULL, 0, fmt, arg);
  va_end(arg);
  if (size >= 0) {
    char *s = malloc(size + 1);
    if (!s)
      return -1;
    va_start(arg, fmt);
    vsnprintf(s, size + 1, fmt, arg);
    va_end(arg);
    s[size] = 0;
    *strp = s;
  }
  return size;
}

static enum gru_error xlat_errno_in(void)
{
  if (errno == ENOMEM)
    return GRU_ERROR_MEMORY;
  else if (errno == EACCES)
    return GRU_ERROR_FILEIN;
  else if (errno == EFAULT || errno == ELOOP || errno == ENOTDIR ||
           errno == ENAMETOOLONG || errno == EINVAL)
  {
    return GRU_ERROR_PARAM;
  }
  else if (errno == EOVERFLOW || errno == EMFILE || errno == ENFILE)
    return GRU_ERROR_RANGE;
  else
    return GRU_SUCCESS;
}

static enum gru_error xlat_errno_out(void)
{
  if (errno == ENOMEM)
    return GRU_ERROR_MEMORY;
  else if (errno == EACCES || errno == EBUSY || errno == ENOTEMPTY ||
           errno == EPERM || errno == EROFS || errno == EIO)
  {
    return GRU_ERROR_FILEOUT;
  }
  else if (errno == EFAULT || errno == ELOOP || errno == ENOTDIR ||
           errno == EISDIR || errno == ENAMETOOLONG || errno == EINVAL)
  {
    return GRU_ERROR_PARAM;
  }
  else if (errno == EOVERFLOW || errno == EMFILE || errno == ENFILE)
    return GRU_ERROR_RANGE;
  else
    return GRU_SUCCESS;
}

enum gru_error gru_os_rm(const char *path)
{
  enum gru_error e = GRU_SUCCESS;
  int old_errno = errno;
  struct stat st;
#ifndef _WIN32
  if (lstat(path, &st) == -1) {
#else
  if (stat(path, &st) == -1) {
#endif
    e = xlat_errno_in();
  }
  else {
    switch (st.st_mode & S_IFMT) {
      case S_IFDIR: {
        errno = 0;
        DIR *d = opendir(path);
        if (d) {
          struct dirent *ent;
          while (!e && (ent = readdir(d))) {
            if (strcmp(ent->d_name, ".") == 0 ||
                strcmp(ent->d_name, "..") == 0)
            {
              continue;
            }
            char *p;
            if (asprintf(&p, "%s/%s", path, ent->d_name) < 0)
              e = GRU_ERROR_MEMORY;
            else {
              e = gru_os_rm(p);
              free(p);
            }
          }
          closedir(d);
        }
        if (!e)
          e = xlat_errno_in();
        if (!e && rmdir(path))
          e = xlat_errno_out();
        break;
      }
#ifndef _WIN32
      case S_IFLNK:
#endif
      case S_IFREG: {
        if (unlink(path))
          e = xlat_errno_out();
        break;
      }
    }
  }
  errno = old_errno;
  return e;
}

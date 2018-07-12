#include "file.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "output.h"

#define min(a, b) \
  ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define max(a, b) \
  ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

FILE* open_file()
{
  FILE* file = NULL;
  DIR* dir = opendir(".");
  struct dirent* dirent;
  if (!dir) {
    fatal("Failed to open '%s' directory", ".");
    return NULL;
  }
  static char* files[] = { "build.facio", "build", "facio" };
  struct stat sb;
  while ((dirent = readdir(dir)) != NULL && file == NULL) {
    for (uint8_t i = 0; i < 3; ++i) {
      if (strncmp(dirent->d_name, files[i], max(strlen(files[i]), strlen(dirent->d_name))) == 0 && stat(dirent->d_name, &sb) == 0 && !(sb.st_mode & S_IXUSR)) {
        file = fopen(dirent->d_name, "r");
        if (!file) {
          warning("Failed to open '%s'", dirent->d_name);
        } else {
          return file;
        }
      }
    }
  }
  fatal("Found no facio files");
  closedir(dir);
  return file;
}

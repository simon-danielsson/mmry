#include "main.h"

static char *get_home_dir() {
  char buff[256] = {0};
  char *d = getcwd(buff, 256);
  int brack_encount = 0;
  char *part1 = (char *)malloc(256);
  for (int i = 0; d[i] != '\0'; i++) {
    if (d[i] == '/') {
      brack_encount++;
    }
    if (brack_encount == 3) {
      strncpy(part1, d, i);
      part1[i] = '\0';
      break;
    }
  }
  return part1;
}

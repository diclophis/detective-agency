#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  FILE *f = 0;
  f = fopen("Detectivefile", "r");
  if (0 == f) {
    return 1;
  }

  fclose(f);

  return 0;
}

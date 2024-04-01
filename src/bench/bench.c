#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  long count = 0;

  long max = atol(argv[1]);

  for ( long i = 0 ; i < max ; i++ ) count++;

  printf("%ld\n", count);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  long count = 0;

  long max = atol(argv[1]);

  for ( long i = 0 ; i < 1000000000l ; i++ ) count++;

  printf("%ld\n", count);
}

#include <stdlib.h>
#include <stdio.h>

int main() {
  int scannedNumberFromA;
  int matchedA = sscanf("Aaa", "%d", &scannedNumberFromA);

  if(matchedA != 0) {
    printf("%d\n", matchedA);
  }

  return 0;
}

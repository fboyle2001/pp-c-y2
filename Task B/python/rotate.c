#include <stdlib.h>
#include <stdio.h>

void rotate_row(char* array, int length, int shift);
void print_array(char* array, int length);

int main() {
  int length = 9;
  char* row = malloc(length * sizeof(char));

  row[0] = '1';
  row[1] = '2';
  row[2] = '3';
  row[3] = '4';
  row[4] = '5';
  row[5] = '6';
  row[6] = '7';
  row[7] = '8';
  row[8] = '9';

  print_array(row, length);
  rotate_row(row, length, -2);
  print_array(row, length);

  free(row);
}

/*
def rotate_row(row, shift):
    orig = row[:]
    copy = row[0]

    for i in range(len(row)):
        a = (i + shift) % len(row)
        print(a)
        temp = row[a]
        row[a] = orig[i]
        copy = temp
*/

void rotate_row(char* row, int length, int shift) {
  char* copy = malloc(length * sizeof(char));

  for(int i = 0; i < length; i++) {
    copy[i] = row[i];
  }

  int shifted;

  for(int i = 0; i < length; i++) {
    shifted = (i + shift) % length;

    if(shifted < 0) {
      shifted += length;
    }

    row[shifted] = copy[i];
  }

  free(copy);
}

void print_array(char* array, int length) {
  for(int i = 0; i < length; i++) {
    printf("%c ", array[i]);
  }

  printf("\n");
}

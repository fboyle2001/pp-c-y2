//You can include any of headers defined in the C11 standard here. They are:
//assert.h, complex.h, ctype.h, errno.h, fenv.h, float.h, inttypes.h, iso646.h, limits.h, locale.h, math.h, setjmp.h, signal.h, stdalign.h, stdarg.h, stdatomic.h, stdbool.h, stddef.h, stdint.h, stdio.h, stdlib.h, stdnoreturn.h, string.h, tgmath.h, threads.h, time.h, uchar.h, wchar.h or wctype.h
//You may not include any other headers.
#include <stdio.h>
#include <stdlib.h>
#include"connect4.h"

void print_double_array(char** array, int globalLen, int innerLen);
void print_array(char* array, int length);

struct board_structure {
  char** positions;
  int toMove;
  int rows;
  int columns;
};

board setup_board() {
  // we typedef to a pointer so malloc it
  board board = malloc(sizeof(board));

  board->positions = NULL;
  board->toMove = 0;
  board->rows = 0;
  board->columns = 0;

  return board;
}

void cleanup_board(board u){
  //You may put code here
  free(u);
}

void read_in_file(FILE *infile, board u){
  //You may put code here
  // Board data
  char **boardData = NULL;
  int numberOfRows = 0;
  int charactersPerRow = -1;

  // Row data
  char readInChar;
  char *rowArray = NULL;
  int rowLength = 0;

  while(fscanf(infile, "%c", &readInChar) != EOF) {
    if(readInChar == '\n') {
      if(charactersPerRow == -1) {
        charactersPerRow = rowLength;
      } else {
        if(charactersPerRow != rowLength) {
          printf("Non-matched lengths");
          exit(-1);
        }
      }

      numberOfRows++;
      char** tempBoardPointer = realloc(boardData, sizeof(char) * charactersPerRow * numberOfRows);

      if(!tempBoardPointer) {
        printf("Bad 2\n");
        exit(1);
      }

      boardData = tempBoardPointer;
      boardData[numberOfRows - 1] = rowArray;

      // Clean up for next row
      rowLength = 0;
      // free(rowArray); // is this needed?? might break stuff actually
      rowArray = NULL;
      continue;
    }

    rowLength++;
    char* tempPointer = realloc(rowArray, sizeof(char) * rowLength);

    if(!tempPointer) {
      printf("Bad\n");
      exit(1);
    }

    rowArray = tempPointer;
    rowArray[rowLength - 1] = readInChar;
  }

  u->positions = boardData;
  u->toMove = 0;
  u->rows = numberOfRows;
  u->columns = charactersPerRow;
}

void write_out_file(FILE *outfile, board u){
  for(int i = 0; i < u->rows; i++) {
    for(int j = 0; j < u->columns; j++) {
      fputc(u->positions[i][j], outfile);
    }

    fputc('\n', outfile);
  }
}

char next_player(board u){
//You may put code here
}

char current_winner(board u){
//You may put code here
}

struct move read_in_move(board u){
//You may put code here
  printf("Player %c enter column to place your token: ",next_player(u)); //Do not edit this line
//You may put code here
  printf("Player %c enter row to rotate: ",next_player(u)); // Do not edit this line
//You may put code here
}

int is_valid_move(struct move m, board u){
//You may put code here
}

char is_winning_move(struct move m, board u){
//You may put code here
}

void play_move(struct move m, board u){
//You may put code here
}

//You may put additional functions here if you wish.

void print_array(char* array, int length) {
  for(int i = 0; i < length; i++) {
    printf("%c", array[i]);
  }

  printf("\n");
}

void print_double_array(char** array, int globalLen, int innerLen) {
  for(int i = 0; i < globalLen; i++) {
    for(int j = 0; j < innerLen; j++) {
      printf("%c", array[i][j]);
    }
    printf("\n");
  }
}

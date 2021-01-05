//You can include any of headers defined in the C11 standard here. They are:
//assert.h, complex.h, ctype.h, errno.h, fenv.h, float.h, inttypes.h, iso646.h, limits.h, locale.h, math.h, setjmp.h, signal.h, stdalign.h, stdarg.h, stdatomic.h, stdbool.h, stddef.h, stdint.h, stdio.h, stdlib.h, stdnoreturn.h, string.h, tgmath.h, threads.h, time.h, uchar.h, wchar.h or wctype.h
//You may not include any other headers.
#include <stdio.h>
#include <stdlib.h>
#include"connect4.h"

// Defined at bottom of the file
char* get_column(board u, int column);
void set_column(board u, int column, char* newColData);
int shift_to_end(char* array, int length);
void rotate_array(char* array, int length, int shift);
int apply_gravity(board u);
void rotate_row(board u, int row, int direction);
struct diagonal get_diagonal(board u, int row, int column, int direction);

struct board_structure {
  char** positions;
  int toMove;
  int rows;
  int columns;
  struct move* oWinningLine;
  struct move* xWinningLine;
};

// Used by get_diagonal to return the data about the diagonal
struct diagonal {
  char* data;
  int length;
  struct move* positions;
};

board setup_board() {
  board board = malloc(sizeof(*board));

  board->positions = NULL;
  board->toMove = 0;
  board->rows = 0;
  board->columns = 0;
  board->oWinningLine = NULL;
  board->xWinningLine = NULL;

  return board;
}

void cleanup_board(board u){
  // We need to free each element within the char** since they are char* that are malloc'd
  for(int i = 0; i < u->rows; i++) {
    free(u->positions[i]);
    u->positions[i] = NULL;
  }

  // Then free the char**
  free(u->positions);
  u->positions = NULL;

  // Free the winning lines for each
  // free is fine for NULLs so don't have to worry
  free(u->oWinningLine);
  u->oWinningLine = NULL;

  free(u->xWinningLine);
  u->xWinningLine = NULL;

  // Finally free the actual board pointer
  free(u);
  u = NULL;
}

/*
* Valid files will:
* - Have at most 512 columns
* - Contain only '.', 'x', 'o'
* - Gravity will already have been applied
*
* Need to also determine which player's turn it is
*/
void read_in_file(FILE *infile, board u){
  // If the infile pointer is NULL (e.g. file does not exist)
  if(infile == NULL) {
    fprintf(stderr, "Unable to read the input file\n");
    exit(1);
  }

  // Board data
  char **boardData = NULL;
  int numberOfRows = 0;
  int charactersPerRow = -1;

  // Row data
  char readInChar;
  char *rowArray = NULL;
  int rowLength = 0;

  // Counters to track the moves
  int oMoves = 0;
  int xMoves = 0;

  // Scan each character from the file
  while(fscanf(infile, "%c", &readInChar) != EOF) {
    // If it is \n then we have an end of line
    if(readInChar == '\n') {
      // If this is the first row set the characters per row
      if(charactersPerRow == -1) {
        charactersPerRow = rowLength;
      } else {
        // Each row must have the same number of characters
        if(charactersPerRow != rowLength) {
          fprintf(stderr, "Rows are not of equal length\n");
          exit(1);
        }
      }

      // If the row length isn't at least 4 then the board is invalid
      if(rowLength < 4) {
        fprintf(stderr, "Input files must have at least 4 columns\n");
        exit(1);
      }

      // Track the number of rows since char** doesn't have an inherent length
      numberOfRows++;
      // Extend the memory allocated for the board
      char** tempBoardPointer = realloc(boardData, sizeof(char*) * numberOfRows);

      if(tempBoardPointer == NULL) {
        fprintf(stderr, "Unable to allocate space for the board\n");
        exit(1);
      }

      boardData = tempBoardPointer;
      // Set the row on the board
      boardData[numberOfRows - 1] = rowArray;

      // Clean up for next row
      rowLength = 0;
      rowArray = NULL;
      continue;
    }

    // We only accept x, o, . and \n
    if(readInChar != 'x' && readInChar != 'o' && readInChar != '.') {
      fprintf(stderr, "Input file is not valid (contains %c)\n", readInChar);
      exit(1);
    }

    rowLength++;

    // We only accept 4 <= rows <= 512
    if(rowLength > 512) {
      fprintf(stderr, "Input files must have at most 512 columns\n");
      exit(1);
    }

    // We need to extend the memory allocated for the row
    char* tempPointer = realloc(rowArray, sizeof(char) * rowLength);

    if(tempPointer == NULL) {
      fprintf(stderr, "Unable to allocate space for the row\n");
      exit(1);
    }

    // If it is an x or o then count the move
    if(readInChar == 'x') {
      xMoves++;
    } else if(readInChar == 'o') {
      oMoves++;
    }

    // Set the character on the row
    rowArray = tempPointer;
    rowArray[rowLength - 1] = readInChar;
  }

  // We now validate the number of moves made
  int moveDif = xMoves - oMoves;

  // o can never make more moves than x (only equal or one less)
  if(moveDif < 0) {
    fprintf(stderr, "The input file is invalid: o has made more moves than x\n");
    exit(1);
  }

  // x can only make equal or one more move than o
  if(moveDif > 1) {
    fprintf(stderr, "The input file is invalid: x has made at least 2 more moves than o\n");
    exit(1);
  }

  // Set the values for the board
  u->positions = boardData;
  u->toMove = moveDif; // If moveDif == 0 then it's x, if moveDif == 1 then it's o
  u->rows = numberOfRows;
  u->columns = charactersPerRow;
  u->xWinningLine = NULL;
  u->oWinningLine = NULL;

  // Now check gravity has already been applied
  // apply_gravity returns the number of elements affected
  int shiftedTotal = apply_gravity(u);

  // Specification says these files are invalid
  if(shiftedTotal != 0) {
    fprintf(stderr, "Input files must already have had gravity applied\n");
    exit(1);
  }
}

void write_out_file(FILE *outfile, board u){
  // If the outfile pointer is NULL (e.g. file locked)
  if(outfile == NULL) {
    fprintf(stderr, "Unable to open the output file\n");
    exit(1);
  }

  // Check if someone has won so we can capitialise if necessary
  char winner = current_winner(u);
  struct move *oWinningLine = NULL;
  struct move *xWinningLine = NULL;

  if(winner == 'x' || winner == 'd') {
    xWinningLine = u->xWinningLine;
  }

  if(winner == 'o' || winner == 'd') {
    oWinningLine = u->oWinningLine;
  }

  // Go row by row, character by character
  for(int i = 0; i < u->rows; i++) {
    for(int j = 0; j < u->columns; j++) {
      if(oWinningLine != NULL) {
        // Acts a boolean to prevent double writing
        int set = 0;

        // Capitialise the winning line for O
        for(int k = 0; k < 4; k++) {
          if(oWinningLine[k].row == i && oWinningLine[k].column == j) {
            fputc('O', outfile);
            set = 1;
            break;
          }
        }

        if(set == 1) {
          continue;
        }
      }

      if(xWinningLine != NULL) {
        int set = 0;

        // Capitialise the winning line for X
        for(int k = 0; k < 4; k++) {
          if(xWinningLine[k].row == i && xWinningLine[k].column == j) {
            fputc('X', outfile);
            set = 1;
            break;
          }
        }

        if(set == 1) {
          continue;
        }
      }

      // Put the character in the file
      fputc(u->positions[i][j], outfile);
    }

    // Once we reach the end of a line put a new line
    fputc('\n', outfile);
  }
}

char next_player(board u){
  // 0 == 'x', 1 == 'o'
  return u->toMove == 0 ? 'x' : 'o';
}

char current_winner(board u){
  /*
  * Need to check:
  * columns (no wrapping)
  * rows (with wrapping)
  * diagonals (no wrapping since get_diagonal handles for us)
  * Also need to check for draws
  */

  int hasXWon = 0;
  int hasOWon = 0;

  // A winning line will consist of 4 coordinates
  // struct move is used for this as it has the two int variables we want
  struct move* xWinningLine = calloc(4, sizeof(struct move));

  if(xWinningLine == NULL) {
    fprintf(stderr, "Unable to allocate space for xWinningLine\n");
    exit(1);
  }

  struct move* oWinningLine = calloc(4, sizeof(struct move));

  if(oWinningLine == NULL) {
    fprintf(stderr, "Unable to allocate space for oWinningLine\n");
    exit(1);
  }

  char* colData = NULL;
  // Keeps track of a potential win
  char lastSeen = '.';
  int run = 0;

  // If the board is full then it is a draw if no player has won
  int full = 1;

  // Check columns
  for(int col = 0; col < u->columns; col++) {
    colData = get_column(u, col);
    run = 0;
    lastSeen = '.';

    for(int i = 0; i < u->rows; i++) {
      char pos = colData[i];

      // If any . are present that it is not yet full
      if(pos == '.') {
        full = 0;
      }

      // If we are on a run of the same type and it's not blanks
      if(pos == lastSeen && lastSeen != '.') {
        // Increment the run
        run++;

        // If we now have 4 they have won
        if(run == 4) {
          if(lastSeen == 'x') {
            hasXWon = 1;
            xWinningLine[0] = (struct move) { .column = col, .row = i - 3 };
            xWinningLine[1] = (struct move) { .column = col, .row = i - 2 };
            xWinningLine[2] = (struct move) { .column = col, .row = i - 1 };
            xWinningLine[3] = (struct move) { .column = col, .row = i     };
          } else if (lastSeen == 'o') {
            hasOWon = 1;
            oWinningLine[0] = (struct move) { .column = col, .row = i - 3 };
            oWinningLine[1] = (struct move) { .column = col, .row = i - 2 };
            oWinningLine[2] = (struct move) { .column = col, .row = i - 1 };
            oWinningLine[3] = (struct move) { .column = col, .row = i     };
          }
        }
      } else {
        // Otherwise reset the run
        lastSeen = pos;
        run = 1;
      }
    }

    // Must free the colData each time otherwise we will get a memory leak
    free(colData);
    colData = NULL;
  }

  char* rowData;
  lastSeen = '.';
  run = 0;

  // Check rows need to take with wrapping
  for(int row = 0; row < u->rows; row++) {
    rowData = u->positions[row];
    run = 0;
    lastSeen = '.';

    // This handles non-wrapping cases
    for(int i = 0; i < u->columns; i++) {
      char pos = rowData[i];

      // If we are on a run of the same type and it's not blanks
      if(pos == lastSeen && lastSeen != '.') {
        // Increment the run
        run++;

        // If we now have 4 they have won
        if(run == 4) {
          if(lastSeen == 'x') {
            hasXWon = 1;
            xWinningLine[0] = (struct move) { .column = i - 3, .row = row };
            xWinningLine[1] = (struct move) { .column = i - 2, .row = row };
            xWinningLine[2] = (struct move) { .column = i - 1, .row = row };
            xWinningLine[3] = (struct move) { .column = i,     .row = row };
          } else if (lastSeen == 'o') {
            hasOWon = 1;
            oWinningLine[0] = (struct move) { .column = i - 3, .row = row };
            oWinningLine[1] = (struct move) { .column = i - 2, .row = row };
            oWinningLine[2] = (struct move) { .column = i - 1, .row = row };
            oWinningLine[3] = (struct move) { .column = i,     .row = row };
          }
        }
      } else {
        // Otherwise reset the run
        lastSeen = pos;
        run = 1;
      }
    }

    // Now handle the wrapping cases
    // The possibilites are
    // 0, 1, 2, 3... (No)
    // Len-1, 0, 1, 2
    // Len-2, Len-1, 0, 1
    // Len-3, Len-2, Len-1, 0
    // ...Len-4, Len-3, Len-2, Len-1 (No)

    for(int start = u->columns - 4; start < u->columns; start++) {
      if(rowData[start % u->columns] == rowData[(start + 1) % u->columns] && rowData[(start + 1) % u->columns] == rowData[(start + 2) % u->columns] && rowData[(start + 2) % u->columns] == rowData[(start + 3) % u->columns]) {
        if(rowData[start % u->columns] == 'x') {
          hasXWon = 1;
          xWinningLine[0] = (struct move) { .column = start % u->columns,       .row = row };
          xWinningLine[1] = (struct move) { .column = (start + 1) % u->columns, .row = row };
          xWinningLine[2] = (struct move) { .column = (start + 2) % u->columns, .row = row };
          xWinningLine[3] = (struct move) { .column = (start + 3) % u->columns, .row = row };
        } else if (rowData[start % u->columns] == 'o') {
          hasOWon = 1;
          oWinningLine[0] = (struct move) { .column = start % u->columns,       .row = row };
          oWinningLine[1] = (struct move) { .column = (start + 1) % u->columns, .row = row };
          oWinningLine[2] = (struct move) { .column = (start + 2) % u->columns, .row = row };
          oWinningLine[3] = (struct move) { .column = (start + 3) % u->columns, .row = row };
        }
      }
    }

    // Don't free as it is still being used by the board
  }

  // Finally check diagonals, a bit more difficult but no need for wrapping
  // The get_diagonal will return the diagonals

  for(int i = 0; i < u->columns; i++) {
    struct diagonal left_diagonal = get_diagonal(u, u->rows - 1, i, -1);

    // If it's less than 4 then they can't win on the diagonal
    if(left_diagonal.length >= 4) {
      for(int start = 0; start < left_diagonal.length - 3; start++) {
        // Check the characters are all the same and are 'x' or 'o'
        if(left_diagonal.data[start] == left_diagonal.data[start + 1] && left_diagonal.data[start + 1] == left_diagonal.data[start + 2] && left_diagonal.data[start + 2] == left_diagonal.data[start + 3]) {
          if(left_diagonal.data[start] == 'x') {
            hasXWon = 1;
            xWinningLine[0] = (struct move) { .column = left_diagonal.positions[start].column,     .row = left_diagonal.positions[start].row     };
            xWinningLine[1] = (struct move) { .column = left_diagonal.positions[start + 1].column, .row = left_diagonal.positions[start + 1].row };
            xWinningLine[2] = (struct move) { .column = left_diagonal.positions[start + 2].column, .row = left_diagonal.positions[start + 2].row };
            xWinningLine[3] = (struct move) { .column = left_diagonal.positions[start + 3].column, .row = left_diagonal.positions[start + 3].row };
          } else if (left_diagonal.data[start] == 'o') {
            hasOWon = 1;
            oWinningLine[0] = (struct move) { .column = left_diagonal.positions[start].column,     .row = left_diagonal.positions[start].row     };
            oWinningLine[1] = (struct move) { .column = left_diagonal.positions[start + 1].column, .row = left_diagonal.positions[start + 1].row };
            oWinningLine[2] = (struct move) { .column = left_diagonal.positions[start + 2].column, .row = left_diagonal.positions[start + 2].row };
            oWinningLine[3] = (struct move) { .column = left_diagonal.positions[start + 3].column, .row = left_diagonal.positions[start + 3].row };
          }
        }
      }
    }

    // Need to free
    free(left_diagonal.data);
    left_diagonal.data = NULL;
    free(left_diagonal.positions);
    left_diagonal.positions = NULL;

    struct diagonal right_diagonal = get_diagonal(u, u->rows - 1, i, 1);

    // If it's less than 4 then they can't win on the diagonal
    if(right_diagonal.length >= 4) {
      for(int start = 0; start < right_diagonal.length - 3; start++) {
        if(right_diagonal.data[start] == right_diagonal.data[start + 1] && right_diagonal.data[start + 1] == right_diagonal.data[start + 2] && right_diagonal.data[start + 2] == right_diagonal.data[start + 3]) {
          if(right_diagonal.data[start] == 'x') {
            hasXWon = 1;
            xWinningLine[0] = (struct move) { .column = right_diagonal.positions[start].column,     .row = right_diagonal.positions[start].row     };
            xWinningLine[1] = (struct move) { .column = right_diagonal.positions[start + 1].column, .row = right_diagonal.positions[start + 1].row };
            xWinningLine[2] = (struct move) { .column = right_diagonal.positions[start + 2].column, .row = right_diagonal.positions[start + 2].row };
            xWinningLine[3] = (struct move) { .column = right_diagonal.positions[start + 3].column, .row = right_diagonal.positions[start + 3].row };
          } else if (right_diagonal.data[start] == 'o') {
            hasOWon = 1;
            oWinningLine[0] = (struct move) { .column = right_diagonal.positions[start].column,     .row = right_diagonal.positions[start].row     };
            oWinningLine[1] = (struct move) { .column = right_diagonal.positions[start + 1].column, .row = right_diagonal.positions[start + 1].row };
            oWinningLine[2] = (struct move) { .column = right_diagonal.positions[start + 2].column, .row = right_diagonal.positions[start + 2].row };
            oWinningLine[3] = (struct move) { .column = right_diagonal.positions[start + 3].column, .row = right_diagonal.positions[start + 3].row };
          }
        }
      }
    }

    // Need to free
    free(right_diagonal.data);
    right_diagonal.data = NULL;
    free(right_diagonal.positions);
    right_diagonal.positions = NULL;
  }

  // First check for a draw where they both have 4
  if(hasXWon == 1 && hasOWon == 1) {
    // Have to free before in case they have been set
    free(u->xWinningLine);
    free(u->oWinningLine);
    u->xWinningLine = xWinningLine;
    u->oWinningLine = oWinningLine;
    return 'd';
  }

  // Now check for individual wins
  if(hasXWon == 1) {
    free(u->xWinningLine);
    u->xWinningLine = xWinningLine;
    free(oWinningLine);
    oWinningLine = NULL;
    return 'x';
  }

  if(hasOWon == 1) {
    free(u->oWinningLine);
    u->oWinningLine = oWinningLine;
    free(xWinningLine);
    xWinningLine = NULL;
    return 'o';
  }

  // Full board is a draw only if neither of the players have won
  if(full == 1) {
    free(u->xWinningLine);
    free(u->oWinningLine);

    u->xWinningLine = NULL;
    u->oWinningLine = NULL;

    free(xWinningLine);
    xWinningLine = NULL;

    free(oWinningLine);
    oWinningLine = NULL;

    return 'd';
  }

  // Otherwise the game continues
  free(oWinningLine);
  oWinningLine = NULL;
  free(xWinningLine);
  xWinningLine = NULL;
  return '.';
}

struct move read_in_move(board u){
  printf("Player %c enter column to place your token: ",next_player(u)); //Do not edit this line

  // Read in an integer
  // Don't validate any further other than that it is an integer
  int column;
  int scanfResult = scanf("%d", &column);

  if(scanfResult == 0) {
    fprintf(stderr, "Column must be an integer\n");
    exit(1);
  }

  printf("Player %c enter row to rotate: ",next_player(u)); // Do not edit this line

  // Same for the row
  int row;
  scanfResult = scanf("%d", &row);

  if(scanfResult == 0) {
    fprintf(stderr, "Row must be an integer\n");
    exit(1);
  }

  // Don't do any checking or normalisation here

  struct move inputtedMove = { .column = column, .row = row };
  return inputtedMove;
}

int is_valid_move(struct move m, board u){
  // 1 <= m.column <= u->columns
  if(m.column > u->columns) {
    return 0;
  }

  if(m.column < 1) {
    return 0;
  }

  int positiveRow = m.row;

  if(positiveRow < 0) {
    positiveRow *= -1;
  }

  // 0 = NO ROTATE, 1 <= positiveRow <= u->rows
  if(positiveRow > u->rows) {
    return 0;
  }

  // Shouldn't be possible but just in case
  if(positiveRow < 0) {
    return 0;
  }

  // Now lets check they can actually drop in the column (e.g. it's not full)
  char* columnData = get_column(u, m.column - 1);

  // If the top element isn't '.' then they can't drop in this column
  if(columnData[0] != '.') {
    free(columnData);
    columnData = NULL;
    return 0;
  }

  free(columnData);
  columnData = NULL;
  return 1;
}

char is_winning_move(struct move m, board u){
  /*
  * 1. Copy board
  * 2. Apply move
  * 3. Check winner
  * 4. Cleanup duplicated board
  */

  // 1. Copy board
  board copy = setup_board();
  char** boardData = calloc(u->columns * u->rows, sizeof(char*));

  if(boardData == NULL) {
    fprintf(stderr, "Unable to allocate space to store the duplicated board\n");
    exit(1);
  }

  // We don't want to pass the pointers in the original board
  // we will accidentally make changes to it if we do so
  for(int row = 0; row < u->rows; row++) {
    char* rowData = calloc(u->columns, sizeof(char));

    if(rowData == NULL) {
      fprintf(stderr, "Unable to allocate space to store the duplicated row\n");
      exit(1);
    }

    // Copy over so we don't affect the original board
    for(int col = 0; col < u->columns; col++) {
      rowData[col] = u->positions[row][col];
    }

    boardData[row] = rowData;
  }

  copy->positions = boardData;
  copy->toMove = u->toMove;
  copy->rows = u->rows;
  copy->columns = u->columns;
  copy->xWinningLine = NULL;
  copy->oWinningLine = NULL;

  // 2. Apply move
  play_move(m, copy);

  // 3. Check winner
  char winner = current_winner(copy);

  // 4. Cleanup duplicated board
  cleanup_board(copy);
  return winner;
}

void play_move(struct move m, board u){
  // Start by fixing the move so we can actually use it with the board structure
  int column = m.column - 1;
  int row = m.row;
  // If the row is negative then we want to rotate left. Positive is right. 0 is none
  int rowDirection = m.row < 0 ? -1 : (m.row > 0 ? 1 : 0);

  // Now get the positive row value
  if(row < 0) {
    row *= -1;
  }

  // We have the fixed column, positive row (not 0 to rows - 1) and the row direction
  // We also know they are all valid at this point so lets make the move

  // Drop into the top of the column
  char* selectedColumn = get_column(u, column);
  selectedColumn[0] = next_player(u);
  set_column(u, column, selectedColumn);
  free(selectedColumn);
  selectedColumn = NULL;

  // Now apply gravity
  apply_gravity(u);

  // If they selected a row to rotate now do it
  // And apply gravity again
  if(row != 0) {
    // u-rows - row will make it match the board structure
    rotate_row(u, u->rows - row, rowDirection);
    apply_gravity(u);
  }

  // Swap who's turn it is to move
  u->toMove = (u->toMove + 1) % 2;
}

//You may put additional functions here if you wish.

// Select a column from the board
// Make sure to free once done with the column
char* get_column(board u, int column) {
  // no. of rows here since depth of a column = rows
  char* colData = calloc(u->rows, sizeof(char));

  if(colData == NULL) {
    fprintf(stderr, "Unable to allocate space to store the column\n");
    exit(1);
  }

  for(int row = 0; row < u->rows; row++) {
    colData[row] = u->positions[row][column];
  }

  return colData;
}

// Sets the values of a column in the board
void set_column(board u, int column, char* newColData) {
  for(int row = 0; row < u->rows; row++) {
    u->positions[row][column] = newColData[row];
  }
}

// Takes an array, shifts to the end and removes gaps between elements while maintaining length
// Returns the number of shifted elements
int shift_to_end(char* array, int length) {
  int filledSpaces = 0;
  int moved = 1;
  int i, shiftIndex;
  int shiftedTotal = 0;

  // Check each time we make a move
  while(moved == 1) {
    moved = 0;

    // Start from the final space - filled spaces
    for(i = length - 1 - filledSpaces; i >= 0; i--) {
      if(array[i] != '.') {
        shiftIndex = length - 1 - filledSpaces;
        array[shiftIndex] = array[i];

        if(shiftIndex != i) {
          array[i] = '.';
          // Only want to count those that actually led to a change
          shiftedTotal++;
        }

        moved = 1;
        filledSpaces++;
      }
    }
  }

  return shiftedTotal;
}

// Shifts everything by 'shift' and wraps around
// Affects the array passed to it so returns void
void rotate_array(char* array, int length, int shift) {
  char* copy = malloc(length * sizeof(char));

  // Copy the array so we can use its values when rotating
  for(int i = 0; i < length; i++) {
    copy[i] = array[i];
  }

  // Will store the new index
  int shifted;

  for(int i = 0; i < length; i++) {
    // Calculate the new index
    shifted = (i + shift) % length;

    // % doesn't wrap the negative values so do it manually
    if(shifted < 0) {
      shifted += length;
    }

    // Update the array in place
    array[shifted] = copy[i];
  }

  // Free the copy we made
  free(copy);

  // Affects the char* passed in so don't return
}

// Applies gravity to the board
// Returns number of affected cells
int apply_gravity(board u) {
  char* colData = NULL;
  int shiftedTotal = 0;

  // Go by column and apply shift_to_end
  for(int col = 0; col < u->columns; col++) {
    colData = get_column(u, col);
    shiftedTotal += shift_to_end(colData, u->rows);
    set_column(u, col, colData);
    free(colData);
    colData = NULL;
  }

  return shiftedTotal;
}

// Rotates a row in place on the board
void rotate_row(board u, int row, int direction) {
  rotate_array(u->positions[row], u->columns, direction < 0 ? -1 : (direction > 0 ? 1 : 0));
}

// Gets a diagonal from the board
// Row should always be u->rows - 1
struct diagonal get_diagonal(board u, int row, int column, int direction) {
  char* diagonal = NULL;
  struct move *diagonalPositions = NULL;
  int diagonalLength = 0;

  while(row != -1) {
    // Length of the diagonal is initially unknown so we work it out as we go
    diagonalLength++;
    diagonal = realloc(diagonal, sizeof(char) * diagonalLength);
    // Store the position on the real board of each element in the diagonal
    diagonalPositions = realloc(diagonalPositions, sizeof(struct move) * diagonalLength);

    if(diagonal == NULL) {
      fprintf(stderr, "Unable to allocate space for the diagonal\n");
      exit(1);
    }

    diagonal[diagonalLength - 1] = u->positions[row][column];
    // Use struct move for the positions as it stores the row and column
    diagonalPositions[diagonalLength - 1] = (struct move) { .row = row, .column = column };

    // Go down the rows
    // We always pass u->rows - 1 to the function anyway
    row -= 1;
    column += direction;
    column %= u->columns;

    // % doesn't handle negatives as expected
    if(column < 0) {
      column = column + u->columns;
    }
  }

  // We need to return more information than just the data so wrap it in a struct
  struct diagonal returnDiagonal = { .data = diagonal, .length = diagonalLength, .positions = diagonalPositions };
  return returnDiagonal;
}

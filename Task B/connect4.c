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
struct move *find_winning_line(board u, char player);

struct board_structure {
  char** positions;
  int toMove;
  int rows;
  int columns;
};

// Used to return the length and data of a diagonal
struct diagonal {
  char* data;
  int length;
};

board setup_board() {
  // we typedef to a pointer so malloc it
  // TODO is this right??
  board board = malloc(sizeof(*board));

  board->positions = NULL;
  board->toMove = 0;
  board->rows = 0;
  board->columns = 0;

  return board;
}

void cleanup_board(board u){
  for(int i = 0; i < u->rows; i++) {
    free(u->positions[i]);
    u->positions[i] = NULL;
  }

  free(u->positions);
  u->positions = NULL;
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
  // Board data
  char **boardData = NULL;
  int numberOfRows = 0;
  int charactersPerRow = -1;

  // Row data
  char readInChar;
  char *rowArray = NULL;
  int rowLength = 0;

  int oMoves = 0;
  int xMoves = 0;

  while(fscanf(infile, "%c", &readInChar) != EOF) {
    if(readInChar == '\n') {
      if(charactersPerRow == -1) {
        charactersPerRow = rowLength;
      } else {
        if(charactersPerRow != rowLength) {
          fprintf(stderr, "Rows are not of equal length\n");
          exit(1);
        }
      }

      if(rowLength < 4) {
        fprintf(stderr, "Input files must have at least 4 columns\n");
        exit(1);
      }

      numberOfRows++;
      char** tempBoardPointer = realloc(boardData, sizeof(char*) * numberOfRows);

      if(tempBoardPointer == NULL) {
        fprintf(stderr, "Unable to allocate space for the board\n");
        exit(1);
      }

      boardData = tempBoardPointer;
      boardData[numberOfRows - 1] = rowArray;

      // Clean up for next row
      rowLength = 0;
      rowArray = NULL;
      continue;
    }

    if(readInChar != 'x' && readInChar != 'o' && readInChar != '.') {
      fprintf(stderr, "Input file is not valid (contains %c)\n", readInChar);
      exit(1);
    }

    rowLength++;

    if(rowLength > 512) {
      fprintf(stderr, "Input files must have at most 512 columns\n");
      exit(1);
    }

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

    rowArray = tempPointer;
    rowArray[rowLength - 1] = readInChar;
  }

  int moveDif = xMoves - oMoves;

  if(moveDif < 0) {
    fprintf(stderr, "The input file is invalid: o has made more moves than x\n");
    exit(1);
  }

  if(moveDif > 1) {
    fprintf(stderr, "The input file is invalid: x has made at least 2 more moves than o\n");
    exit(1);
  }

  u->positions = boardData;
  u->toMove = moveDif; // If moveDif == 0 then it's x, if moveDif == 1 then it's o
  u->rows = numberOfRows;
  u->columns = charactersPerRow;

  // Now check gravity has already been applied

  int shiftedTotal = apply_gravity(u);

  if(shiftedTotal != 0) {
    fprintf(stderr, "Input files must already have had gravity applied\n");
    exit(1);
  }
}

void write_out_file(FILE *outfile, board u){
  char winner = current_winner(u);
  struct move *oWinningLine = NULL;
  struct move *xWinningLine = NULL;

  if(winner == 'x' || winner == 'd') {
    xWinningLine = find_winning_line(u, 'x');
  }

  if(winner == 'o' || winner == 'd') {
    oWinningLine = find_winning_line(u, 'o');
  }

  for(int i = 0; i < u->rows; i++) {
    for(int j = 0; j < u->columns; j++) {
      if(oWinningLine != NULL) {
        int set = 0;

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

      fputc(u->positions[i][j], outfile);
    }

    fputc('\n', outfile);
  }

  free(xWinningLine);
  xWinningLine = NULL;
  free(oWinningLine);
  oWinningLine = NULL;
}

char next_player(board u){
  return u->toMove == 0 ? 'x' : 'o';
}

char current_winner(board u){
  /*
  * Need to check:
  * columns (no wrapping)
  * rows (with wrapping)
  * diagonals (no wrapping)
  * Also need to check for draws
  */

  int hasXWon = 0;
  int hasOWon = 0;

  char* colData = NULL;
  char lastSeen = '.';
  int run = 0;

  int full = 1;

  // Check columns
  for(int col = 0; col < u->columns; col++) {
    colData = get_column(u, col);

    for(int i = 0; i < u->rows; i++) {
      char pos = colData[i];

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
          } else if (lastSeen == 'o') {
            hasOWon = 1;
          }
        }
      } else {
        // Otherwise reset the run
        lastSeen = pos;
        run = 1;
      }
    }

    free(colData);
    colData = NULL;
  }

  // Full board is a draw
  if(full == 1) {
    return 'd';
  }

  char* rowData;
  lastSeen = '.';
  run = 0;

  // Check rows need to take with wrapping
  for(int row = 0; row < u->rows; row++) {
    rowData = u->positions[row];

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
          } else if (lastSeen == 'o') {
            hasOWon = 1;
          }
        }
      } else {
        // Otherwise reset the run
        lastSeen = pos;
        run = 1;
      }
    }

    // Now handle the wrapping cases
    // The possibilites are [][][][][][][][][]
    // 0, 1, 2, 3... (No)
    // Len-1, 0, 1, 2
    // Len-2, Len-1, 0, 1
    // Len-3, Len-2, Len-1, 0
    // ...Len-4, Len-3, Len-2, Len-1 (No)

    for(int start = u->columns - 4; start < u->columns; start++) {
      if(rowData[start % u->columns] == rowData[(start + 1) % u->columns] && rowData[(start + 1) % u->columns] == rowData[(start + 2) % u->columns] && rowData[(start + 2) % u->columns] == rowData[(start + 3) % u->columns]) {
        if(rowData[start % u->columns] == 'x') {
          hasXWon = 1;
        } else if (rowData[start % u->columns] == 'o') {
          hasOWon = 1;
        }
      }
    }

    // Don't free as it is still being used by the board
  }

  // Finally check diagonals, going to be a bit difficult but no need for wrapping
  // The get_diagonal will return the diagonals

  for(int i = 0; i < u->columns; i++) {
    struct diagonal left_diagonal = get_diagonal(u, u->rows - 1, i, -1);

    // If it's less than 4 then they can't win on the diagonal
    if(left_diagonal.length >= 4) {
      for(int start = 0; start < left_diagonal.length - 3; start++) {
        if(left_diagonal.data[start] == left_diagonal.data[start + 1] && left_diagonal.data[start + 1] == left_diagonal.data[start + 2] && left_diagonal.data[start + 2] == left_diagonal.data[start + 3]) {
          if(left_diagonal.data[start] == 'x') {
            hasXWon = 1;
          } else if (left_diagonal.data[start] == 'o') {
            hasOWon = 1;
          }
        }
      }
    }

    // Need to free
    free(left_diagonal.data);
    left_diagonal.data = NULL;

    struct diagonal right_diagonal = get_diagonal(u, u->rows - 1, i, 1);

    // If it's less than 4 then they can't win on the diagonal
    if(right_diagonal.length >= 4) {
      for(int start = 0; start < right_diagonal.length - 3; start++) {
        if(right_diagonal.data[start] == right_diagonal.data[start + 1] && right_diagonal.data[start + 1] == right_diagonal.data[start + 2] && right_diagonal.data[start + 2] == right_diagonal.data[start + 3]) {
          if(right_diagonal.data[start] == 'x') {
            hasXWon = 1;
          } else if (right_diagonal.data[start] == 'o') {
            hasOWon = 1;
          }
        }
      }
    }

    // Need to free
    free(right_diagonal.data);
    right_diagonal.data = NULL;
  }

  if(hasXWon == 1 && hasOWon == 1) {
    return 'd';
  }

  if(hasXWon == 1) {
    return 'x';
  }

  if(hasOWon == 1) {
    return 'o';
  }

  return '.';
}

struct move read_in_move(board u){
  printf("Player %c enter column to place your token: ",next_player(u)); //Do not edit this line

  int column;
  int scanfResult = scanf("%d", &column);

  if(scanfResult == 0) {
    fprintf(stderr, "Column must be an integer\n");
    exit(1);
  }

  printf("Player %c enter row to rotate: ",next_player(u)); // Do not edit this line

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

  // Shouldn't be possible but whatever
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

// Need to test
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
  int rowDirection = m.row < 0 ? -1 : (m.row > 0 ? 1 : 0);

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
  u->toMove = (u -> toMove + 1) % 2;
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

  while(moved == 1) {
    moved = 0;

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

// Shifts everything one index and wraps
// Affects the array passed to it so returns void
void rotate_array(char* array, int length, int shift) {
  char copy = array[0];
  char temp;

  for(int i = 0; i < length; i++) {
    temp = array[(i + shift) % length];
    array[(i + shift) % length] = copy;
    copy = temp;
  }
}

// Applies gravity to the board
// Returns number of affected cells
int apply_gravity(board u) {
  char* colData = NULL;
  int shiftedTotal = 0;

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
  int diagonalLength = 0;

  while(row != -1) {
    diagonalLength++;
    diagonal = realloc(diagonal, sizeof(char) * diagonalLength);

    if(diagonal == NULL) {
      fprintf(stderr, "Unable to allocate space for the diagonal\n");
      exit(1);
    }

    diagonal[diagonalLength - 1] = u->positions[row][column];
    row -= 1;
    column += direction;
    column %= u->columns;

    // % doesn't handle negatives as expected
    if(column < 0) {
      column = column + u->columns;
    }
  }

  struct diagonal returnDiagonal = { .data = diagonal, .length = diagonalLength };
  return returnDiagonal;
}

// Finds the indices that define the winning line for a player
// Used to output the winning line in capitals
// The output will be stored in the move[4] array
struct move *find_winning_line(board u, char player) {
  struct move *winningLine = calloc(4, sizeof(struct move));

  if(winningLine == NULL) {
    fprintf(stderr, "Unable to allocate space to store the winning line for %c\n", player);
    exit(1);
  }

  char* colData = NULL;
  char lastSeen = '.';
  int run = 0;

  // Check columns
  for(int col = 0; col < u->columns; col++) {
    colData = get_column(u, col);

    for(int i = 0; i < u->rows; i++) {
      char pos = colData[i];

      // If we are on a run of the same type and it's not blanks
      if(pos == lastSeen && lastSeen != '.') {
        // Increment the run
        run++;

        // If we now have 4 they have won
        if(run == 4) {
          if(lastSeen == player) {
            winningLine[0] = (struct move) { .column = col, .row = i - 3 };
            winningLine[1] = (struct move) { .column = col, .row = i - 2 };
            winningLine[2] = (struct move) { .column = col, .row = i - 1 };
            winningLine[3] = (struct move) { .column = col, .row = i     };
            free(colData);
            colData = NULL;
            return winningLine;
          }
        }
      } else {
        // Otherwise reset the run
        lastSeen = pos;
        run = 1;
      }
    }

    free(colData);
    colData = NULL;
  }

  char* rowData;
  lastSeen = '.';
  run = 0;

  // Check rows need to take with wrapping
  for(int row = 0; row < u->rows; row++) {
    rowData = u->positions[row];

    // This handles non-wrapping cases
    for(int i = 0; i < u->columns; i++) {
      char pos = rowData[i];

      // If we are on a run of the same type and it's not blanks
      if(pos == lastSeen && lastSeen != '.') {
        // Increment the run
        run++;

        // If we now have 4 they have won
        if(run == 4) {
          if(lastSeen == player) {
            winningLine[0] = (struct move) { .column = i - 3, .row = row };
            winningLine[1] = (struct move) { .column = i - 2, .row = row };
            winningLine[2] = (struct move) { .column = i - 1, .row = row };
            winningLine[3] = (struct move) { .column = i,     .row = row };
            return winningLine;
          }
        }
      } else {
        // Otherwise reset the run
        lastSeen = pos;
        run = 1;
      }
    }

    // Now handle the wrapping cases
    // The possibilites are [][][][][][][][][]
    // 0, 1, 2, 3... (No)
    // Len-1, 0, 1, 2
    // Len-2, Len-1, 0, 1
    // Len-3, Len-2, Len-1, 0
    // ...Len-4, Len-3, Len-2, Len-1 (No)

    for(int start = u->columns - 4; start < u->columns; start++) {
      if(rowData[start % u->columns] == rowData[(start + 1) % u->columns] && rowData[(start + 1) % u->columns] == rowData[(start + 2) % u->columns] && rowData[(start + 2) % u->columns] == rowData[(start + 3) % u->columns]) {
        if(rowData[start % u->columns] == player) {
          winningLine[0] = (struct move) { .column = start % u->columns,       .row = row };
          winningLine[1] = (struct move) { .column = (start + 1) % u->columns, .row = row };
          winningLine[2] = (struct move) { .column = (start + 2) % u->columns, .row = row };
          winningLine[3] = (struct move) { .column = (start + 3) % u->columns, .row = row };
          return winningLine;
        }
      }
    }

    // Don't free as it is still being used by the board
  }

  // Finally check diagonals, going to be a bit difficult but no need for wrapping
  // The get_diagonal will return the diagonals

  for(int i = 0; i < u->columns; i++) {
    struct diagonal left_diagonal = get_diagonal(u, u->rows - 1, i, -1);

    // If it's less than 4 then they can't win on the diagonal
    if(left_diagonal.length >= 4) {
      for(int start = 0; start < left_diagonal.length - 3; start++) {
        if(left_diagonal.data[start] == left_diagonal.data[start + 1] && left_diagonal.data[start + 1] == left_diagonal.data[start + 2] && left_diagonal.data[start + 2] == left_diagonal.data[start + 3]) {
          if(left_diagonal.data[start] == player) {
            int firstCol = i % u->columns;

            if(firstCol < 0) {
              firstCol += u->columns;
            }

            int secondCol = (i - 1) % u->columns;

            if(secondCol < 0) {
              secondCol += u->columns;
            }

            int thirdCol = (i - 2) % u->columns;

            if(thirdCol < 0) {
              thirdCol += u->columns;
            }

            int fourthCol = (i - 3) % u->columns;

            if(fourthCol < 0) {
              fourthCol += u->columns ;
            }

            winningLine[0] = (struct move) { .column = firstCol,  .row = u->rows - start - 1 };
            winningLine[1] = (struct move) { .column = secondCol, .row = u->rows - start - 2 };
            winningLine[2] = (struct move) { .column = thirdCol,  .row = u->rows - start - 3 };
            winningLine[3] = (struct move) { .column = fourthCol, .row = u->rows - start - 4 };
            free(left_diagonal.data);
            left_diagonal.data = NULL;
            return winningLine;
          }
        }
      }
    }

    free(left_diagonal.data);
    left_diagonal.data = NULL;
    struct diagonal right_diagonal = get_diagonal(u, u->rows - 1, i, 1);

    // If it's less than 4 then they can't win on the diagonal
    if(right_diagonal.length >= 4) {
      for(int start = 0; start < right_diagonal.length - 3; start++) {
        if(right_diagonal.data[start] == right_diagonal.data[start + 1] && right_diagonal.data[start + 1] == right_diagonal.data[start + 2] && right_diagonal.data[start + 2] == right_diagonal.data[start + 3]) {
          if(right_diagonal.data[start] == player) {
            int firstCol = i % u->columns;

            if(firstCol < 0) {
              firstCol += u->columns;
            }

            int secondCol = (i + 1) % u->columns;

            if(secondCol < 0) {
              secondCol += u->columns;
            }

            int thirdCol = (i + 2) % u->columns;

            if(thirdCol < 0) {
              thirdCol += u->columns;
            }

            int fourthCol = (i + 3) % u->columns;

            if(fourthCol < 0) {
              fourthCol += u->columns;
            }

            winningLine[0] = (struct move) { .column = firstCol,  .row = u->rows - start - 1 };
            winningLine[1] = (struct move) { .column = secondCol, .row = u->rows - start - 2 };
            winningLine[2] = (struct move) { .column = thirdCol,  .row = u->rows - start - 3 };
            winningLine[3] = (struct move) { .column = fourthCol, .row = u->rows - start - 4 };
            free(right_diagonal.data);
            right_diagonal.data = NULL;
            return winningLine;
          }
        }
      }
    }

    // Need to free
    free(right_diagonal.data);
    right_diagonal.data = NULL;
  }

  free(winningLine);
  winningLine = NULL;
  return winningLine;
}

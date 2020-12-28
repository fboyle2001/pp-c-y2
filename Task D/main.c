#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/*
* Should accept many files
* -n numeric sort
* -r reverse
* -o output file
*/

void display_help();
char** read_file_lines(char* fileName, int* lines);

int main(int argc, char** argv) {
  char** inputLines = NULL;
  int totalInputLines = 0;

  if(argc <= 1) {
    // No args
    printf("No arguments specified\n");
    display_help();
    return 1;
  }

  for(int i = 1; i < argc; i++) {
    char* arg = argv[i];
    int argLength = strlen(arg);

    if(argLength == 0) {
      printf("Invalid argument\n");
      display_help();
      return 1;
    }

    if(arg[0] == '-') {
      // parse the options
      continue;
    }

    // Add in something here to intercept the output file

    // Read the input file
    int lines = 0;
    char** fileLines = read_file_lines(arg, &lines);

    inputLines = realloc(inputLines, (totalInputLines + lines) * sizeof(char*));

    for(int j = 0; j < lines; j++) {
      inputLines[totalInputLines + j] = fileLines[j];
      //free(fileLines[j]);
    }

    free(fileLines);
    totalInputLines += lines;
  }

  for(int i = 0; i < totalInputLines; i++) {
    free(inputLines[i]);
  }

  free(inputLines);

  return 0;
}

void display_help() {
  printf("==Help==\n");
}

char** read_file_lines(char* fileName, int* lines) {
  FILE* fp = fopen(fileName, "r");
  char** fileLines = NULL;
  // Whole file reading
  int totalLines = 0;

  // Single line reading
  char* currentLine = NULL;
  int currentLineLength = 0;

  char readInChar;
  int c = 0;

  while(fscanf(fp, "%c", &readInChar) != EOF) {
    if(readInChar == '\n') {
      c++;
      currentLineLength++;
      currentLine = realloc(currentLine, currentLineLength * sizeof(char));
      currentLine[currentLineLength - 1] = '\0';

      totalLines++;

      fileLines = realloc(fileLines, totalLines * sizeof(char*));
      fileLines[totalLines - 1] = currentLine;

      currentLine = NULL;
      currentLineLength = 0;
      continue;
    }

    currentLineLength++;
    currentLine = realloc(currentLine, currentLineLength * sizeof(char));
    currentLine[currentLineLength - 1] = readInChar;
  }

  *lines = totalLines;
  fclose(fp);
  return fileLines;
}

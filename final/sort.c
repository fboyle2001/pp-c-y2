#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>

/*
* Should accept many files
* -n numeric sort
* -r reverse
* -o output file
*/

void display_help();
char** read_file_lines(char* fileName, int* lines);
void write_file_lines(char* fileName, char** outputLines, int lines);
char** read_stdin(int* lines);

int compareStrings(const void *a, const void *b);
int compareStringsReverse(const void *a, const void *b);
int compareStringsNumeric(const void *a, const void *b);
int compareStringsNumericReverse(const void *a, const void *b);

int main(int argc, char** argv) {
  // Resolves issue with locale differences affecting GNU sort
  // Takes the environment's locale
  setlocale(LC_ALL, "");

  // Going to read the data into here
  char** inputLines = NULL;
  int totalInputLines = 0;

  // true if the flag is set
  bool reverse = false;
  bool numeric = false;

  // Output file name
  char* outputFile = NULL;
  // If -o is set then we should expect an output file
  bool expectingOutputFile = false;

  // If we have options then
  if(argc > 1) {
    // Iterate over each option
    for(int i = 1; i < argc; i++) {
      // Get the argument
      char* arg = argv[i];
      int argLength = strlen(arg);

      if(argLength == 0) {
        printf("Invalid argument\n");
        display_help();
        return 1;
      }

      // Add in something here to intercept the output file
      if(expectingOutputFile) {
        outputFile = arg;
        expectingOutputFile = false;
        continue;
      }

      // If it starts with a - then it is a potential option
      // Unix sort does not accept files with names like '-test.txt'
      // So don't worry about those
      if(arg[0] == '-') {
        if(argLength != 2) {
          // Invalid option it's just a '-' or too many characters
          printf("Invalid option %s\n", arg);
          display_help();
          return 1;
        }

        // Parse the options
        switch(arg[1]) {
          case 'o':
            if(outputFile != NULL) {
              printf("Already set output file\n");
              return 1;
            }

            expectingOutputFile = true;
            break;
          case 'n':
            numeric = true;
            break;
          case 'r':
            reverse = true;
            break;
          case 'h':
            display_help();
            return 0;
          default:
            printf("Invalid option -%c\n", arg[1]);
            display_help();
            return 1;
        }

        continue;
      }

      // Read the input file
      int lines = 0;
      char** fileLines = read_file_lines(arg, &lines);

      inputLines = realloc(inputLines, (totalInputLines + lines) * sizeof(char*));

      if(inputLines == NULL) {
        fprintf(stderr, "Unable to allocate space for the input lines\n");
        return 1;
      }

      // Store the read input file lines
      for(int j = 0; j < lines; j++) {
        inputLines[totalInputLines + j] = fileLines[j];
      }

      free(fileLines);
      // Track the number of lines
      totalInputLines += lines;
    }
  }

  // If we were waiting for an output file but never received one then don't keep going
  if(expectingOutputFile) {
    // put -o but no file
    printf("-o requires an argument but no output file was specified\n");
    return 1;
  }

  // Read from stdin instead if we haven't got any input lines
  if(inputLines == NULL) {
    inputLines = read_stdin(&totalInputLines);
  }

  if(reverse) {
    if(numeric) {
      // -r -n
      qsort(inputLines, totalInputLines, sizeof(char *), compareStringsNumericReverse);
    } else {
      // -r
      qsort(inputLines, totalInputLines, sizeof(char *), compareStringsReverse);
    }
  } else {
    if(numeric) {
      // -n
      qsort(inputLines, totalInputLines, sizeof(char *), compareStringsNumeric);
    } else {
      // no options
      qsort(inputLines, totalInputLines, sizeof(char *), compareStrings);
    }
  }

  // Write the data out to stdout or the file
  if(outputFile != NULL) {
    write_file_lines(outputFile, inputLines, totalInputLines);
  } else {
    for(int i = 0; i < totalInputLines; i++) {
      printf("%s\n", inputLines[i]);
    }
  }

  // Free the input lines
  for(int i = 0; i < totalInputLines; i++) {
    free(inputLines[i]);
  }

  free(inputLines);
  return 0;
}

int compareStrings(const void *a, const void *b) {
  // Just use strcoll to do the comparison. Prefer strcoll over strcmp as it respects locale
  // the cast is void *a -> char **a then dereference char **a to get the char* which is the string
  return strcoll(*(char **)a, *(char **)b);
}

int compareStringsReverse(const void *a, const void *b) {
  // reverse just needs to be the negative
  return -compareStrings(a, b);
}

int compareStringsNumeric(const void *a, const void *b) {
  /*
  * 1. Extract the number from the front
  * 2. Compare them
  * 3. If they are equal return strcoll(a, b) instead
  */
  int scannedNumberFromA;
  int matchedA = sscanf(*(char **)a, "%d", &scannedNumberFromA);

  int scannedNumberFromB;
  int matchedB = sscanf(*(char **)b, "%d", &scannedNumberFromB);

  /*
  * The way sort -n works is that if a string does not start with a number
  * it is assumed to start with 0 instead
  */

  // A didn't start with a number but B did
  if(matchedA <= 0 && matchedB > 0) {
    // B starts with < 0 so A is put ahead of it
    if(scannedNumberFromB < 0) {
      return 1;
    }

    // B > 0 so put it ahead
    if(scannedNumberFromB > 0) {
      return -1;
    }

    // Otherwise go to compare by strcoll like normal
  }

  // A starts with a number but B doesn't
  if(matchedA > 0 && matchedB <= 0) {
    // A starts with < 0 so B is put ahead of it
    if(scannedNumberFromA < 0) {
      return -1;
    }

    // A > 0 so put it ahead
    if(scannedNumberFromA > 0) {
      return 1;
    }

    // If they are both 0
  }

  // both have int starts

  if(matchedA > 0 && matchedB > 0) {
    // If the number in A > number in B then put it higher
    if(scannedNumberFromA > scannedNumberFromB) {
      return 1;
    }

    // Otherwise check the reverse
    if(scannedNumberFromA < scannedNumberFromB) {
      return -1;
    }

    // If they are equal keep going
  }

  // otherwise compare normally

  return strcoll(*(char **)a, *(char **)b);
}

int compareStringsNumericReverse(const void *a, const void *b) {
  // reverse just needs to be the negative
  return -compareStringsNumeric(a, b);
}

void display_help() {
  printf("I have implemented all options specified in Task D of the coursework\n");
  printf("Usage: ./sort [OPTION]... [FILE]...\n");
  printf("With no FILE it reads standard input until EOF [CTRL-D]\n");
  printf("\n");
  printf("Ordering options:\n");
  printf("\n");
  printf(" -n      compare according to string numerical value\n");
  printf(" -r      reverse the result of comparisons\n");
  printf("\n");
  printf("Other options:\n");
  printf("\n");
  printf(" -o FILE write result to FILE instead of standard output\n");
  printf(" -h      displays this help dialogue based off of sort --help\n");
}

char** read_file_lines(char* fileName, int* lines) {
  FILE* fp = fopen(fileName, "r");

  if(fp == NULL) {
    fprintf(stderr, "%s input file does not exist\n", fileName);
    exit(1);
  }

  char** fileLines = NULL;
  // Whole file reading
  int totalLines = 0;

  // Single line reading
  char* currentLine = NULL;
  int currentLineLength = 0;

  char readInChar;

  while(fscanf(fp, "%c", &readInChar) != EOF) {
    if(readInChar == '\n') {
      currentLineLength++;
      currentLine = realloc(currentLine, currentLineLength * sizeof(char));

      if(currentLine == NULL) {
        fprintf(stderr, "Unable to allocate space for the current line buffer\n");
        exit(1);
      }

      currentLine[currentLineLength - 1] = '\0';

      totalLines++;

      fileLines = realloc(fileLines, totalLines * sizeof(char*));

      if(fileLines == NULL) {
        fprintf(stderr, "Unable to allocate space to store the next line of text\n");
      }

      fileLines[totalLines - 1] = currentLine;

      currentLine = NULL;
      currentLineLength = 0;
      continue;
    }

    currentLineLength++;
    currentLine = realloc(currentLine, currentLineLength * sizeof(char));

    if(currentLine == NULL) {
      fprintf(stderr, "Unable to allocate space for the current line buffer\n");
      exit(1);
    }

    currentLine[currentLineLength - 1] = readInChar;
  }

  *lines = totalLines;
  fclose(fp);
  return fileLines;
}

char** read_stdin(int* lines) {
  char** fileLines = NULL;
  // Whole file reading
  int totalLines = 0;

  // Single line reading
  char* currentLine = NULL;
  int currentLineLength = 0;

  char readInChar;
  int c = 0;

  while(fscanf(stdin, "%c", &readInChar) != EOF) {
    if(readInChar == '\n') {
      c++;
      currentLineLength++;
      currentLine = realloc(currentLine, currentLineLength * sizeof(char));

      if(currentLine == NULL) {
        fprintf(stderr, "Unable to allocate space for the current line buffer\n");
        exit(1);
      }

      currentLine[currentLineLength - 1] = '\0';

      totalLines++;

      fileLines = realloc(fileLines, totalLines * sizeof(char*));

      if(fileLines == NULL) {
        fprintf(stderr, "Unable to allocate space to store the next line of text\n");
      }

      fileLines[totalLines - 1] = currentLine;

      currentLine = NULL;
      currentLineLength = 0;
      continue;
    }

    currentLineLength++;
    currentLine = realloc(currentLine, currentLineLength * sizeof(char));

    if(currentLine == NULL) {
      fprintf(stderr, "Unable to allocate space for the current line buffer\n");
      exit(1);
    }

    currentLine[currentLineLength - 1] = readInChar;
  }

  *lines = totalLines;
  return fileLines;
}

void write_file_lines(char* fileName, char** outputLines, int lines) {
  FILE* fp = fopen(fileName, "w");

  if(fp == NULL) {
    fprintf(stderr, "Unable to open the file %s in write mode", fileName);
    exit(1);
  }

  for(int line = 0; line < lines; line++) {
    fprintf(fp, "%s\n", outputLines[line]);
  }

  fclose(fp);
}

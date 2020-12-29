#!/bin/bash

rm -rf test_custom_normal.out test_gnu_normal.out test_custom_reverse.out test_gnu_reverse.out test_custom_numeric.out test_gnu_numeric.out test_custom_numeric_reverse.out test_gnu_numeric_reverse.out

#This should give no warnings or errors
gcc -Wextra -Wall -pedantic -std=c11 sort.c -o sort

./sort "$@" -o test_custom_normal.out
sort "$@" -o test_gnu_normal.out

diff test_custom_normal.out test_gnu_normal.out

if [ $? -eq 0 ]
then
  echo Regular sort passed
else
  echo Regular sort FAILED
fi

./sort "$@" -r -o test_custom_reverse.out
sort "$@" -r -o test_gnu_reverse.out

diff -q test_custom_reverse.out test_gnu_reverse.out

if [ $? -eq 0 ]
then
  echo Reverse sort passed
else
  echo Reverse sort FAILED
fi

./sort "$@" -n -o test_custom_numeric.out
sort "$@" -n -o test_gnu_numeric.out

diff -q test_custom_numeric.out test_gnu_numeric.out

if [ $? -eq 0 ]
then
  echo Numeric sort passed
else
  echo Numeric sort FAILED
fi

./sort "$@" -n -r -o test_custom_numeric_reverse.out
sort "$@" -n -r -o test_gnu_numeric_reverse.out

diff -q test_custom_numeric_reverse.out test_gnu_numeric_reverse.out

if [ $? -eq 0 ]
then
  echo Reverse numeric sort passed
else
  echo Reverse numeric sort FAILED
fi

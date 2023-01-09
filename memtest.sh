gcc shell.c -o test.out -g3
valgrind -s --leak-check=full ./test.out
rm test.out
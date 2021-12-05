
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
  // Common String Functions:

  // convert char* to int: atoi
  int num = atoi("1"); //  int 1
  printf("string to int %d\n", num);

  // compare two strings: strcmp
  int res_true = strcmp("a", "a"); // returns if strings are equal, returns 0 if equal, -1 if not equal
  int res_false = strcmp("a", "b");
  printf("strings are equal, %i\n", res_true);
  printf("strings are equal, %i\n", res_false);

  // find if 2nd is substring of first: strstr
  char *res_substring = strstr("abc", "b");
  printf("pointer to substring, %s\n", res_substring);

  return 0;
}

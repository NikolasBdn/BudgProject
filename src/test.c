#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main(int argc, char const *argv[]) {
#define NUMBER_OF_STRING 11
#define MAX_STRING_SIZE 8

// char arr[NUMBER_OF_STRING][MAX_STRING_SIZE] =
// {
//   "#lllll",
//   "#ED3811"
// };
// char arr[NUMBER_OF_STRING][MAX_STRING_SIZE];
  char arr[NUMBER_OF_STRING][MAX_STRING_SIZE]={
    "#7CFC00",
    "#ED3811",
    "#8B20A8",
    "#F3F00A",
    "#0A94F3",
    "#F30AD3",
    "#F30A11",
    "#EFE9E9",
    "#2AFC08",
    "#15F3D7",
    "#4EE49E"
  };
// for (int j = 0; j < NUMBER_OF_STRING; j++) {
//   for (int h = 0; h < MAX_STRING_SIZE; h++) {
//     arr[j][h] = 'l';
//   }
// }


for (int i = 0; i < NUMBER_OF_STRING; i++)
{
    printf("'%s' has length %ld\n", arr[i], strlen(arr[i]));
}
  return 0;
}

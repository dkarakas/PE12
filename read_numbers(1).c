#include <stdio.h>
#include <stdlib.h>

#define CHAR_SIZE 8

void print_char_in_bits(char number)
{
   int i;
   unsigned char mask = 1 << (CHAR_SIZE - 1);
   for (i = 0; i < CHAR_SIZE; i++) {
      // need to form a mask
      // mask = 1;
      // mask <<= (CHAR_SIZE - i - 1);
      int result = mask & number;
      if (result == 0) {
         printf("0");
      } else {
         printf("1");
      }
      mask >>= 1;
   }
}

int main(int argc, char *argv[])
{
   FILE *fptr;
   int i;
   unsigned char num = '0';
   unsigned char num1 = '!';
   unsigned char num2 = '6';
   num = num >> 3;
   num1 = num1 >> 3;
   num2 = num2 >> 3;
   print_char_in_bits(num);
   printf("\n");
   print_char_in_bits(num1);
   printf("\n");
   print_char_in_bits(num2);
   printf("\n");
   int random = num << 10 | num1 << 5 | num2 << 0;
   char *char_array = (char *)&(random);
   print_char_in_bits(char_array[1]);
   printf(" ");
   print_char_in_bits(char_array[0]);
   printf("\n");
   return EXIT_SUCCESS;
}


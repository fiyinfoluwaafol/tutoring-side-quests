#include <stdio.h>
#include <stdlib.h>

/* IMPLEMENT ME: Declare your functions here */
int add (int a, int b);
int subtract(int a, int b);
int multiply(int a, int b);
int divide(int a, int b);
int exit_program(int a, int b);

typedef int (*Operation)(int, int);

int main (void)
{
	/* IMPLEMENT ME: Insert your algorithm here */

  int a = 10;
  int b = 5;
  Operation operations[5] = { add, subtract, multiply, divide, exit_program };

  char input;
  
  while (1) {
    printf("Enter operation (0: add, 1: subtract, 2: multiply, 3: divide, 4: exit): ");
    if (scanf(" %c", &input) != 1) {
        fprintf(stderr, "Error reading input.\n");
        exit(1);
    }
    int index = input - '0';
    operations[index](a, b);
  }
	return 0;
}

/* IMPLEMENT ME: Define your functions here */
int add(int a, int b) {
    printf("Adding %d and %d: %d\n", a, b, a + b);
    return a + b;
}

int subtract(int a, int b) {
    printf("Subtracting %d from %d: %d\n", b, a, a - b);
    return a - b;
}

int multiply(int a, int b) {
    printf("Multiplying %d and %d: %d\n", a, b, a * b);
    return a * b;
}

int divide(int a, int b) {
    if (b == 0) {
        printf("Error: Division by zero.\n");
        return 0;
    }
    printf("Dividing %d by %d: %d\n", a, b, a / b);
    return a / b;
}
int exit_program(int a, int b) {
    printf("Exiting program.\n");
    exit(0);
    return 0;
}
// Modified pipes_processes1.c for 2-way communication with second user input

#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
  
int main() 
{ 
    // Two pipes: one for each direction of communication
    int fd1[2];  // Pipe 1: Parent to Child
    int fd2[2];  // Pipe 2: Child to Parent
  
    char fixed_str[] = "howard.edu"; 
    char input_str[100]; 
    char input_str2[100]; 
    pid_t p; 
  
    if (pipe(fd1) == -1 || pipe(fd2) == -1) 
    { 
        fprintf(stderr, "Pipe Failed" ); 
        return 1; 
    } 
  
    printf("Enter a string to concatenate: ");
    scanf("%s", input_str); 

    p = fork(); 
  
    if (p < 0) 
    { 
        fprintf(stderr, "fork Failed" ); 
        return 1; 
    } 
  
    // Parent process (P1)
    else if (p > 0) 
    { 
        close(fd1[0]); // Close reading end of first pipe
        close(fd2[1]); // Close writing end of second pipe

        // Send input string to child through first pipe
        write(fd1[1], input_str, strlen(input_str) + 1); 
        close(fd1[1]); // Close writing end after sending

        // Wait for child to complete
        wait(NULL); 

        // Read back the concatenated string from child through second pipe
        read(fd2[0], input_str2, 100);
        close(fd2[0]); // Close reading end

        // Prompt the user for a second string
        char second_input[100];
        printf("Enter another string to concatenate: ");
        scanf("%s", second_input);

        // Concatenate the second user input and print the final result
        strcat(input_str2, second_input);
        printf("Final concatenated string: %s\n", input_str2);
    } 
  
    // Child process (P2)
    else
    { 
        close(fd1[1]); // Close writing end of first pipe
        close(fd2[0]); // Close reading end of second pipe

        // Read the string from parent
        char concat_str[100]; 
        read(fd1[0], concat_str, 100);
        close(fd1[0]); // Close reading end after receiving

        // Concatenate "howard.edu" and print the result
        strcat(concat_str, fixed_str);
        printf("Concatenated string from child: %s\n", concat_str);

        // Send the concatenated string back to the parent through second pipe
        write(fd2[1], concat_str, strlen(concat_str) + 1);
        close(fd2[1]); // Close writing end after sending

        exit(0); 
    } 

    return 0; 
}

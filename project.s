.data
id:     .asciiz "@01234567"   # Replace with your own Howard ID

.text
.globl main

# $t2 represents m
# $t0 represents N
# $t3 represents the starting index
# $t5 represents the memory address of the the particular character to be printed
main:
    # Compute N = ID % 8, where ID is the decimal number in your Howard ID
    li   $t0, 7               # N = 1234567 % 8 (Example case)


    ####### PRINT FORWARD #######
    li   $t2, 1               # m = 1 (Loop counter for forward printing)
forward_loop:
    bge  $t2, 10, backward_start # If m >= 10, move to backward printing

    # Compute the starting index for this line: (m + N) % 9
    add  $t3, $t2, $t0        # t3 = (m + N)
    rem  $t3, $t3, 9          # t3 = (m + N) % 9

    # Print substring starting from computed index
    la   $a0, id              # Load base address of the string
    move $a1, $t3             # Store start index for printing
    jal  print_substring      # Call the substring printing function

    # Print newline after each substring
    la   $a0, 10              
    li   $v0, 11
    syscall                   # Print newline

    addi $t2, $t2, 1         # Increment m
    j    forward_loop        # Repeat loop

####### PRINT BACKWARD #######
backward_start:
    li   $t2, 1               # Reset counter for backward loop
backward_loop:
    bge  $t2, 10, exit         # If m >= 10, exit program

    # Compute the starting index for this line: (N + 20 - m) % 9
    li   $t4, 20             # Load constant 20
    add  $t3, $t4, $t0       # t3 = (N + 20)
    sub  $t3, $t3, $t2       # t3 = (N + 20 - m)
    rem  $t3, $t3, 9         # t3 = (N + 20 - m) % 9

    # Print substring starting from computed index
    la   $a0, id              # Load base address of the string
    move $a1, $t3             # Store start index for printing
    jal  print_substring_reversed      # Call the substring printing function

    # Print newline after each substring
    la   $a0, 10
    li   $v0, 11
    syscall                   # Print newline

    addi $t2, $t2, 1         # Increment m
    j    backward_loop       # Repeat loop

exit:
    li   $v0, 10             # Exit syscall
    syscall

####### FUNCTION TO PRINT A SUBSTRING #######
# Arguments:
#   $a0 = Base address of the string
#   $a1 = Start index for printing
# Behavior:
#   Prints 9 characters, looping back to the start if necessary.
print_substring:
    add $t5, $a0, $a1   # $t5 = Address of the starting character
    li  $t6, 9          # Counter for number of characters to print

print_loop:
    lb  $a0, 0($t5)     # Load byte (character) at current address
    li  $v0, 11         # syscall 11: print character
    syscall             # Print character

    addi $t5, $t5, 1    # Move to the next character
    addi $t6, $t6, -1    # Decrease counter
    beqz $t6, done      # If we've printed 9 chars, return

    # Wrap around if we reach the end of the string
    lb  $t7, 0($t5)     # Load next character to check for NULL
    beqz $t7, reset     # If we hit NULL, restart from index 0
    j    print_loop     # Otherwise, continue printing

reset:
    la  $t5, id         # Reset to the start of the string
    j   print_loop      # Continue printing

done:
    jr  $ra             # Return to caller


####### FUNCTION TO PRINT A SUBSTRING (REVERSED) #######
# Arguments:
#   $a0 = Base address of the string
#   $a1 = Start index for printing
# Behavior:
#   Prints 9 characters in reverse order.
print_substring_reversed:
    add $t5, $a0, $a1   # $t5 = Address of the starting character
    li  $t6, 9          # Counter for number of characters to print

reverse_loop:
    lb  $a0, 0($t5)     # Load byte (character) at current address
    li  $v0, 11         # syscall 11: print character
    syscall             # Print character

    addi $t5, $t5, -1   # Move to the PREVIOUS character (reverse order)
    addi $t6, $t6, -1   # Decrease counter
    beqz $t6, done_reverse # If we've printed 9 chars, return

    # Wrap around if we go before index 0
    la  $t7, id         # Load base address of the string
    bge  $t5, $t7, reverse_loop  # If $t5 >= base address, continue

    # If $t5 < base address, reset to last character (index 8)
    add  $t5, $t7, 8        # Move to last character
    j    reverse_loop       # Continue printing

done_reverse:
    jr  $ra                 # Return to caller
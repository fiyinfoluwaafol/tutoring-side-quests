.data
SpaceInput: .space 1002       # space for 1000 characters + newline + null terminator
null_msg:   .asciiz "NULL"
semicolon:  .asciiz ";"

.text
.globl main

main:                         # Start
    # Hard code N and compute M
    li   $t0, 26            # Base value for N = 26 + (X % 11); here N is hard coded to 26
    li   $t1, 10
    sub  $t2, $t0, $t1      # M = N - 10

    # Set up range for lowercase letters: 'a' to (a + M - 1)
    li   $t3, 0x61          # ASCII for 'a'
    add  $t4, $t3, $t2      # calculate a + M
    addi $t4, $t4, -1       # subtract 1 to get last valid letter

    # Set up range for uppercase letters: 'A' to (A + M - 1)
    li   $t5, 0x41          # ASCII for 'A'
    add  $t6, $t5, $t2      # calculate A + M
    addi $t6, $t6, -1       # subtract 1 to get last valid letter

    # Collect input from user
    li   $v0, 8             # syscall code for reading a string
    la   $a0, SpaceInput
    li   $a1, 1002          # maximum number of characters to read
    syscall

    # Initialize counters:
    # $s0: flag indicating if any output has been printed (used for semicolon handling)
    # $t2: current offset into SpaceInput (processed in blocks of 10)
    li   $t2, 0             # offset for substring start
    li   $s0, 0             # output printed flag

get_substrings:
    beq  $t2, 1000, exit    # If 1000 characters have been processed, exit

    # Check if the current substring (10 characters) has any input left
    la   $t7, SpaceInput
    add  $t7, $t7, $t2
    lb   $t8, 0($t7)
    beqz $t8, exit         # If the first character is null, no more input exists

    # Set pointer for the current substring and call get_substring_value
    la   $a0, SpaceInput
    add  $a0, $a0, $t2
    jal  get_substring_value

    # Check for NULL result indicator (0x7FFFFFFF)
    li   $t4, 0x7FFFFFFF
    beq  $v0, $t4, print_null

    # Print semicolon if this is not the first output
    beqz $s0, no_semicolon
    li   $v0, 4
    la   $a0, semicolon
    syscall

no_semicolon:
    li   $s0, 1           # mark that output has been printed

    # Print the computed result (G - H)
    move $t9, $v0         # preserve the computed result
    li   $v0, 1           # syscall code for printing integer
    move $a0, $t9
    syscall

    # Move to next substring (advance by 10 characters)
    addi $t2, $t2, 10
    j    get_substrings

print_null:
    # Print semicolon if not the first output
    beqz $s0, no_null_semicolon
    li   $v0, 4
    la   $a0, semicolon
    syscall

no_null_semicolon:
    li   $s0, 1
    li   $v0, 4
    la   $a0, null_msg
    syscall

    # Move to next substring
    addi $t2, $t2, 10
    j    get_substrings

exit:
    li   $v0, 10          # syscall code for exit
    syscall

# Subprogram: get_substring_value
# Processes a 10-character block, converting valid characters to their base-N numeric values,
# summing the first five digits as G and the last five as H, then returns G - H.
# If no valid digit is found, returns 0x7FFFFFFF (NULL indicator).
get_substring_value:
    li   $t5, 0           # Counter: index within substring (0 to 9)
    li   $s1, 0           # Sum for first half (G)
    li   $s2, 0           # Sum for second half (H)
    li   $s3, 0           # Valid digit counter

get_character:
    bge  $t5, 10, solve   # If index >= 10, finish processing
    lb   $t6, 0($a0)      # Load current character
    beqz $t6, solve       # If null terminator, finish processing

    # Check if character is a digit ('0' to '9')
    li   $t7, 0x30        # ASCII for '0'
    li   $t8, 0x39        # ASCII for '9'
    blt  $t6, $t7, check_if_lowercase
    bgt  $t6, $t8, check_if_lowercase

    # Valid digit: convert from ASCII to integer
    sub  $t9, $t6, $t7
    j    valid_digit

check_if_lowercase:
    # Check if character is lowercase letter ('a' to last valid letter)
    li   $t7, 0x61        # ASCII for 'a'
    blt  $t6, $t7, check_if_uppercase
    addi $t8, $t7, 15      # 'a' + 15 (for example, if M=16, valid range is 'a' to 'p')
    bgt  $t6, $t8, check_if_uppercase
    sub  $t9, $t6, $t7     # char - 'a'
    addi $t9, $t9, 10      # value is 10 + (char - 'a')
    j    valid_digit

check_if_uppercase:
    # Check if character is uppercase letter ('A' to last valid letter)
    li   $t7, 0x41        # ASCII for 'A'
    blt  $t6, $t7, invalid
    addi $t8, $t7, 15      # 'A' + 15 (for example, valid range is 'A' to 'P')
    bgt  $t6, $t8, invalid
    sub  $t9, $t6, $t7     # char - 'A'
    addi $t9, $t9, 10      # value is 10 + (char - 'A')

valid_digit:
    addi $s3, $s3, 1       # Increment count of valid digits

    # Add to first half (G) if index < 5, else add to second half (H)
    li   $t7, 5
    blt  $t5, $t7, add_first
    add  $s2, $s2, $t9      # add to H
    j    move_index

add_first:
    add  $s1, $s1, $t9      # add to G

move_index:
    addi $t5, $t5, 1       # increment character index
    addi $a0, $a0, 1       # move pointer to next character
    j    get_character

invalid:
    # Skip invalid characters
    addi $t5, $t5, 1
    addi $a0, $a0, 1
    j    get_character

solve:
    # If no valid digits were found, return the NULL indicator
    beqz $s3, save_null
    sub  $v0, $s1, $s2     # result = G - H
    jr   $ra

save_null:
    li   $v0, 0x7FFFFFFF    # NULL indicator
    jr   $ra

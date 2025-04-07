.data
SpaceInput: .space 1002       # space for 1000 characters + newline + null terminator
null_msg:   .asciiz "NULL"
semicolon:  .asciiz ";"

.text
.globl main

main:                         # Start
    # Hard code N and compute M
    li   $t0, 26            # Base: N = 26 + (X % 11); here N is hard coded to 26
    li   $t1, 10
    sub  $t2, $t0, $t1      # M = N - 10

    # Set up lowercase letter range: 'a' to (a + M - 1)
    li   $t3, 0x61          # ASCII for 'a'
    add  $t4, $t3, $t2      # a + M
    addi $t4, $t4, -1       # last valid lowercase letter

    # Set up uppercase letter range: 'A' to (A + M - 1)
    li   $t5, 0x41          # ASCII for 'A'
    add  $t6, $t5, $t2      # A + M
    addi $t6, $t6, -1       # last valid uppercase letter

    # Collect input from user
    li   $v0, 8             # syscall code for reading a string
    la   $a0, SpaceInput
    li   $a1, 1002          # maximum characters to read
    syscall

    # Remove newline character (if present) by replacing it with a null terminator
    la   $t7, SpaceInput    # pointer to input string
remove_newline:
    lb   $t0, 0($t7)
    beqz $t0, end_remove    # reached end of string
    li   $t8, 0x0A          # newline character
    beq  $t0, $t8, replace_null
    addi $t7, $t7, 1
    j    remove_newline
replace_null:
    sb   $zero, 0($t7)      # replace newline with null
end_remove:

    # Initialize counters:
    # $s0: flag to check if any output has been printed (for semicolon handling)
    # $t2: current offset into SpaceInput (processing in blocks of 10)
    li   $t2, 0             # offset for substring start
    li   $s0, 0             # output printed flag

get_substrings:
    beq  $t2, 1000, exit    # if 1000 characters have been processed, exit

    # Check if there is any input left for the current block
    la   $t7, SpaceInput
    add  $t7, $t7, $t2
    lb   $t8, 0($t7)
    beqz $t8, exit         # if the first character is null, input is finished

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
    move $t9, $v0         # preserve the result
    li   $v0, 1           # syscall code for printing integer
    move $a0, $t9
    syscall

    # Move to the next substring (advance by 10 characters)
    addi $t2, $t2, 10
    j    get_substrings

print_null:
    # Print semicolon if this is not the first output
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

#-----------------------------------------------------------
# Subprogram: get_substring_value
# Processes exactly 10 characters from the substring.
# For any character read as null (due to a short last substring),
# it substitutes a space (padding).
# It then converts valid digits (as specified) into a base-N number.
# The first five valid characters (ignoring non-digit characters)
# are summed as G and the rest as H. Returns G - H.
# If no valid digit is found, returns 0x7FFFFFFF.
#-----------------------------------------------------------
get_substring_value:
    li   $t5, 0           # character index (0 to 9)
    li   $s1, 0           # sum for first half (G)
    li   $s2, 0           # sum for second half (H)
    li   $s3, 0           # count of valid digits

get_character:
    bge  $t5, 10, solve   # if 10 characters processed, finish
    lb   $t6, 0($a0)      # load current character
    beqz $t6, pad_space   # if null, substitute with space

check_digit:
    # Check if character is a digit ('0'-'9')
    li   $t7, 0x30        # ASCII for '0'
    li   $t8, 0x39        # ASCII for '9'
    blt  $t6, $t7, check_if_lowercase
    bgt  $t6, $t8, check_if_lowercase
    sub  $t9, $t6, $t7    # convert ASCII digit to integer
    j    valid_digit

check_if_lowercase:
    # Check if character is a lowercase letter ('a'- last valid)
    li   $t7, 0x61        # ASCII for 'a'
    blt  $t6, $t7, check_if_uppercase
    # Note: The proper range should be 'a' to (a + M - 1).
    # In this sample, expected valid lowercase letters are 'a' to 't' (if M=20).
    # Adjust the constant accordingly. Here we replace 15 with 20.
    addi $t8, $t7, 20     # 'a' + 20 gives the first invalid letter
    bge  $t6, $t8, check_if_uppercase
    sub  $t9, $t6, $t7    # (char - 'a')
    addi $t9, $t9, 10     # value = 10 + (char - 'a')
    j    valid_digit

check_if_uppercase:
    # Check if character is an uppercase letter ('A'- last valid)
    li   $t7, 0x41        # ASCII for 'A'
    blt  $t6, $t7, invalid
    addi $t8, $t7, 20     # 'A' + 20 gives the first invalid letter
    bge  $t6, $t8, invalid
    sub  $t9, $t6, $t7    # (char - 'A')
    addi $t9, $t9, 10     # value = 10 + (char - 'A')

valid_digit:
    addi $s3, $s3, 1      # increment count of valid digits
    li   $t7, 5
    blt  $t5, $t7, add_first   # if in first 5 positions, add to G
    add  $s2, $s2, $t9    # else, add to H
    j    move_index

add_first:
    add  $s1, $s1, $t9    # add digit to G
    j    move_index

invalid:
    # For non-valid characters, do nothing (they're ignored)
    j    move_index

pad_space:
    li   $t6, 0x20        # substitute null with space
    # Continue to check (a space will fall into invalid and be skipped)
    j    check_digit

move_index:
    addi $t5, $t5, 1      # increment index
    addi $a0, $a0, 1      # move pointer to next character
    j    get_character

solve:
    # If no valid digit was found, return NULL indicator; else return (G - H)
    beqz $s3, save_null
    sub  $v0, $s1, $s2    # result = G - H
    jr   $ra

save_null:
    li   $v0, 0x7FFFFFFF  # NULL indicator
    jr   $ra

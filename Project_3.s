 .data
SpaceInput: .space 1002         # Up to 1000 characters + newline + null terminator
null_msg:   .asciiz "NULL"
semicolon:  .asciiz ";"
strint:     .space 4000         # Array for integer results (each is 4 bytes)

.text
.globl main

#------------------------------------------------------------
# main: Set up constants, get user input, call process_string,
#       and then print the resulting integers.
#------------------------------------------------------------
main:
    # Hard code N and compute M; here we assume:
    #   X % 11 = 4  so that N = 26 + 4 = 30; therefore M = 30 - 10 = 20.
    li   $t0, 30            # N = 30 (base)
    li   $t1, 10
    sub  $s7, $t0, $t1       # $s7 holds M = N - 10 (M = 20)

    # (Optional: Set up letter ranges in registers if needed.)
    li   $t3, 0x61          # ASCII for 'a'
    add  $t4, $t3, $s7      # 'a' + M
    addi $t4, $t4, -1       # last valid lowercase letter
    li   $t5, 0x41          # ASCII for 'A'
    add  $t6, $t5, $s7      # 'A' + M
    addi $t6, $t6, -1       # last valid uppercase letter

    # Get input string from the user.
    li   $v0, 8             # syscall: read string
    la   $a0, SpaceInput
    li   $a1, 1002          # maximum number of characters
    syscall

    # Remove newline: search the string and replace newline with null terminator.
    la   $t7, SpaceInput    # pointer into the input string
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

    # Call process_string with:
    #   $a0 = address of the input string
    #   $a1 = address of the integer array (strint)
    la   $a0, SpaceInput
    la   $a1, strint
    jal  process_string
    # Upon return, $v0 = number of substrings (i.e. number of computed integers)

    # Print the integers from the array.
    move $t0, $v0           # $t0 = number of substrings
    li   $t1, 0             # substring counter
    la   $t2, strint        # pointer to the integer array

print_loop:
    beq  $t1, $t0, exit_main  # if printed all entries, exit

    lw   $t3, 0($t2)         # load current integer result
    li   $t4, 0x7FFFFFFF     # compare to NULL indicator
    beq  $t3, $t4, print_null_main

    # Otherwise, print the integer.
    li   $v0, 1             # syscall: print integer
    move $a0, $t3
    syscall
    j    print_semicolon

print_null_main:
    li   $v0, 4             # syscall: print string
    la   $a0, null_msg
    syscall

print_semicolon:
    addi $t1, $t1, 1        # increment counter
    # Print semicolon if this is not the last output
    beq  $t1, $t0, no_semicolon_main
    li   $v0, 4
    la   $a0, semicolon
    syscall
no_semicolon_main:
    addi $t2, $t2, 4        # advance to next integer (each is 4 bytes)
    j    print_loop

exit_main:
    li   $v0, 10            # exit syscall
    syscall

#------------------------------------------------------------
# process_string:
#   Splits the input string into 10-character substrings.
#   For each substring, it pushes the substring address onto the
#   stack, calls get_substring_value, pops the returned integer,
#   and stores it in the array passed in $a1.
#   Returns in $v0 the count of substrings processed.
#------------------------------------------------------------
process_string:
    # $a0 = pointer to input string,
    # $a1 = pointer to integer array (strint)
    move $t0, $a0         # $t0 is our current pointer into the input string
    move $t7, $zero       # $t7 will count the number of substrings

proc_str_loop:
    lb  $t1, 0($t0)       # get first character of current substring
    beq  $t1, $zero, proc_str_end  # if null, no more input

    # Push current substring pointer onto stack.
    addi $sp, $sp, -4
    sw   $t0, 0($sp)

    # Call get_substring_value. Its parameter is via the stack.
    jal  get_substring_value

    # After return, the result has been pushed onto the stack.
    lw   $t2, 0($sp)      # pop the result from the stack
    addi $sp, $sp, 4

    # Store result into the integer array.
    sw   $t2, 0($a1)
    addi $a1, $a1, 4      # advance the array pointer
    addi $t7, $t7, 1      # increment the substring count

    # Move input pointer ahead by 10 characters.
    addi $t0, $t0, 10
    j    proc_str_loop

proc_str_end:
    move $v0, $t7         # return substring count in $v0
    jr   $ra

#------------------------------------------------------------
# get_substring_value:
#   Processes exactly 10 characters pointed to by the substring address
#   (which is passed via the stack). For each character:
#       - If it is null, substitutes a space.
#       - Valid digits are:
#             '0'-'9'   (digit = char - '0')
#             'a'-('a'+M–1) (digit = 10 + (char - 'a'))
#             'A'-('A'+M–1) (digit = 10 + (char - 'A'))
#       - In the first 5 positions, valid digits contribute to sum G.
#         In the last 5 positions, they contribute to sum H.
#   It returns (via the stack) G – H; if no valid digit is found, returns 0x7FFFFFFF.
#------------------------------------------------------------
get_substring_value:
    # Pop the substring pointer from the stack into $a0.
    lw   $a0, 0($sp)
    addi $sp, $sp, 4

    li   $t0, 0           # index counter from 0 to 9
    li   $s1, 0           # sum for first half (G)
    li   $s2, 0           # sum for second half (H)
    li   $s3, 0           # count of valid digits

gsv_loop:
    bge  $t0, 10, gsv_compute  # finished 10 characters

    lb   $t1, 0($a0)      # load the current character
    beq  $t1, $zero, gsv_pad  # if null, substitute with space
    move $t2, $t1         # else, use the character as is
    j    gsv_check

gsv_pad:
    li   $t2, 0x20       # pad with a space (ASCII 32)

gsv_check:
    # Check if character is a digit ('0' - '9')
    li   $t3, 0x30
    li   $t4, 0x39
    blt  $t2, $t3, gsv_check_lower
    bgt  $t2, $t4, gsv_check_lower
    sub  $t5, $t2, $t3   # digit value = t2 - '0'
    j    gsv_valid

gsv_check_lower:
    # Check if character is lowercase letter 'a' to ('a'+M-1)
    li   $t3, 0x61
    blt  $t2, $t3, gsv_check_upper
    add  $t4, $t3, $s7   # upper bound = 'a' + M
    bge  $t2, $t4, gsv_check_upper
    sub  $t5, $t2, $t3   # digit value = (char - 'a')
    addi $t5, $t5, 10    # add 10 to convert to proper digit value
    j    gsv_valid

gsv_check_upper:
    # Check if character is uppercase letter 'A' to ('A'+M-1)
    li   $t3, 0x41
    blt  $t2, $t3, gsv_invalid
    add  $t4, $t3, $s7   # upper bound = 'A' + M
    bge  $t2, $t4, gsv_invalid
    sub  $t5, $t2, $t3   # digit value = (char - 'A')
    addi $t5, $t5, 10
    j    gsv_valid

gsv_invalid:
    # Invalid character: do nothing
    j    gsv_next

gsv_valid:
    addi $s3, $s3, 1     # increment count of valid digits
    li   $t6, 5
    blt  $t0, $t6, gsv_add_first  # if position less than 5 add to G...
    add  $s2, $s2, $t5   # else, add to H
    j    gsv_next

gsv_add_first:
    add  $s1, $s1, $t5   # add digit value to sum G

gsv_next:
    addi $t0, $t0, 1
    addi $a0, $a0, 1    # advance pointer to next character
    j    gsv_loop

gsv_compute:
    beq  $s3, $zero, gsv_return_null  # if no valid digits, return NULL indicator
    sub  $t7, $s1, $s2   # result = G - H
    j    gsv_push

gsv_return_null:
    li   $t7, 0x7FFFFFFF

gsv_push:
    # Push result onto stack as the return value.
    addi $sp, $sp, -4
    sw   $t7, 0($sp)
    jr   $ra

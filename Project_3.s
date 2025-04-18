 .data
SpaceInput: .space 1002         # Up to 1000 characters + newline + null terminator
null_msg:   .asciiz "NULL"
semicolon:  .asciiz ";"
.align 2
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
#   For each substring, it calls get_substring_value,
#   and stores the result in the array passed in $a1.
#   Returns in $v0 the count of substrings processed.
#------------------------------------------------------------
process_string:
    # Save $ra for nested calls
    addi $sp, $sp, -4
    sw   $ra, 0($sp)
    
    # Save original $a1 value (array pointer) for later use 
    move $t8, $a1
    
    # Preserve the input pointer
    move $t0, $a0
    
    # Initialize counters
    li   $t7, 0           # substring count

proc_str_loop:
    # Check if string is exhausted
    lb   $t1, 0($t0)
    beqz $t1, proc_str_end
    
    # Set up argument and call get_substring_value
    move $a0, $t0
    jal  get_substring_value
    
    # Store result in array at current position
    sw   $v0, 0($t8)
    
    # Move to next array position and increment count
    addi $t8, $t8, 4
    addi $t7, $t7, 1
    
    # Move to next substring (10 characters ahead)
    addi $t0, $t0, 10
    j    proc_str_loop

proc_str_end:
    # Return count in $v0
    move $v0, $t7
    
    # Restore $ra and return
    lw   $ra, 0($sp)
    addi $sp, $sp, 4
    jr   $ra

#------------------------------------------------------------
# get_substring_value:
#   Processes exactly 10 characters from the substring in $a0.
#   For any character read as null (due to a short last substring),
#   it substitutes a space (padding).
#   It then converts valid digits (as specified) into a base-N number.
#   The first five valid characters (ignoring non-digit characters)
#   are summed as G and the rest as H. Returns G - H.
#   If no valid digit is found, returns 0x7FFFFFFF.
#------------------------------------------------------------
get_substring_value:
    # Save s-registers that will be modified
    addi $sp, $sp, -12
    sw   $s1, 8($sp)
    sw   $s2, 4($sp)
    sw   $s3, 0($sp)
    
    # Initialize counters for processing
    li   $t5, 0           # Character index (0 to 9)
    li   $s1, 0           # Sum for first half (G)
    li   $s2, 0           # Sum for second half (H)
    li   $s3, 0           # Count of valid digits

get_character:
    bge  $t5, 10, solve   # If 10 characters processed, finish
    lb   $t6, 0($a0)      # Load current character
    beqz $t6, pad_space   # If null, substitute with space
    j    check_digit

pad_space:
    li   $t6, 0x20        # Space character (ASCII 32)

check_digit:
    # Check if character is a digit ('0'-'9')
    li   $t7, 0x30        # ASCII for '0'
    li   $t8, 0x39        # ASCII for '9'
    blt  $t6, $t7, check_if_lowercase
    bgt  $t6, $t8, check_if_lowercase
    sub  $t9, $t6, $t7    # Convert ASCII digit to integer
    j    valid_digit

check_if_lowercase:
    # Check if character is a lowercase letter ('a'- last valid)
    li   $t7, 0x61        # ASCII for 'a'
    blt  $t6, $t7, check_if_uppercase
    add  $t8, $t7, $s7    # 'a' + M gives the first invalid letter
    bge  $t6, $t8, check_if_uppercase
    sub  $t9, $t6, $t7    # (char - 'a')
    addi $t9, $t9, 10     # Value = 10 + (char - 'a')
    j    valid_digit

check_if_uppercase:
    # Check if character is an uppercase letter ('A'- last valid)
    li   $t7, 0x41        # ASCII for 'A'
    blt  $t6, $t7, invalid
    add  $t8, $t7, $s7    # 'A' + M gives the first invalid letter
    bge  $t6, $t8, invalid
    sub  $t9, $t6, $t7    # (char - 'A')
    addi $t9, $t9, 10     # Value = 10 + (char - 'A')
    j    valid_digit

invalid:
    # Not a valid digit, skip to next character
    j    move_index

valid_digit:
    addi $s3, $s3, 1      # Increment count of valid digits
    li   $t7, 5
    blt  $t5, $t7, add_first   # If in first 5 positions, add to G
    add  $s2, $s2, $t9    # Else, add to H
    j    move_index

add_first:
    add  $s1, $s1, $t9    # Add digit to G

move_index:
    addi $t5, $t5, 1      # Increment index
    addi $a0, $a0, 1      # Move pointer to next character
    j    get_character

solve:
    beqz $s3, save_null   # If no valid digits found
    sub  $v0, $s1, $s2    # Result = G - H
    j    gsv_restore

save_null:
    li   $v0, 0x7FFFFFFF  # NULL indicator

gsv_restore:
    # Restore s-registers
    lw   $s3, 0($sp)
    lw   $s2, 4($sp)
    lw   $s1, 8($sp)
    addi $sp, $sp, 12
    jr   $ra

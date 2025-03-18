        .data
# Do NOT print any prompt because only the result (or "N/A") should appear.
inputBuffer: .space 12         # Buffer for 10 characters (plus possible newline and null terminator)
outputNA:    .asciiz "N/A"       # Output string if no valid digit is found

        .text
        .globl main
main:
        # -------------------------------
        # Compute base parameters (N and M)
        # -------------------------------
        # Assume your Howard ID (change the constant below as needed)
        li      $t0, 12345678      # Howard ID, X
        li      $t4, 11            # Constant 11 for modulo operation
        div     $t0, $t4           # Divide X by 11
        mfhi    $t1                # $t1 = X % 11

        li      $t5, 26            # Constant 26
        add     $t2, $t5, $t1      # N = 26 + (X % 11)
        li      $t6, 10
        sub     $t3, $t2, $t6      # M = N - 10

        # -------------------------------
        # Set up valid letter bounds for lowercase and uppercase digits.
        # Lowercase valid range: 'a' to ('a' + M - 1)
        # Uppercase valid range: 'A' to ('A' + M - 1)
        # -------------------------------
        li      $t7, 0x61          # ASCII code for 'a'
        add     $t8, $t7, $t3      # $t8 = 'a' + M
        addi    $t8, $t8, -1       # $t8 = 'a' + M - 1 (upper bound for lowercase)

        li      $t9, 0x41          # ASCII code for 'A'
        add     $t10, $t9, $t3     # $t10 = 'A' + M
        addi    $t10, $t10, -1      # $t10 = 'A' + M - 1 (upper bound for uppercase)

        # -------------------------------
        # Read the 10-character input from the user.
        # -------------------------------
        li      $v0, 8             # Syscall for reading a string
        la      $a0, inputBuffer   # Address of input buffer
        li      $a1, 12            # Maximum characters to read
        syscall

        # -------------------------------
        # Initialize loop variables and sums.
        # $s0: current index (0 to 9)
        # $s1: sum for first half (G)
        # $s2: sum for second half (H)
        # $s3: count of valid digits encountered
        # -------------------------------
        li      $s0, 0             # index i = 0
        li      $s1, 0             # G = 0 (first half sum)
        li      $s2, 0             # H = 0 (second half sum)
        li      $s3, 0             # Valid digit counter = 0

        # -------------------------------
        # Loop over the 10 characters of the input.
        # -------------------------------
loop:
        bge     $s0, 10, finish_loop   # If index >= 10, exit loop

        # Calculate address of current character: inputBuffer + i
        la      $t11, inputBuffer
        add     $t11, $t11, $s0
        lb      $t12, 0($t11)      # Load character into $t12

        # Check if character is a digit '0'-'9'
        li      $t13, 0x30        # ASCII code for '0'
        li      $t14, 0x39        # ASCII code for '9'
        blt     $t12, $t13, check_lowercase   # If char < '0', check lowercase range
        bgt     $t12, $t14, check_lowercase   # If char > '9', check lowercase range

        # -- Character is a digit --
        sub     $t15, $t12, $t13   # Convert ASCII digit to numeric value (char - '0')
        j       valid_digit

check_lowercase:
        # Check if character is a lowercase letter in range 'a' to ('a' + M - 1)
        li      $t13, 0x61        # ASCII code for 'a'
        blt     $t12, $t13, check_uppercase  # If char < 'a', not lowercase digit
        bgt     $t12, $t8, check_uppercase   # If char > upper bound, not lowercase digit

        # -- Valid lowercase digit: value = 10 + (char - 'a') --
        sub     $t15, $t12, $t13   # t15 = char - 'a'
        addi    $t15, $t15, 10     # t15 = 10 + (char - 'a')
        j       valid_digit

check_uppercase:
        # Check if character is an uppercase letter in range 'A' to ('A' + M - 1)
        li      $t13, 0x41        # ASCII code for 'A'
        blt     $t12, $t13, not_valid  # If char < 'A', not valid
        bgt     $t12, $t10, not_valid  # If char > upper bound, not valid

        # -- Valid uppercase digit: value = 10 + (char - 'A') --
        sub     $t15, $t12, $t13   # t15 = char - 'A'
        addi    $t15, $t15, 10     # t15 = 10 + (char - 'A')
        j       valid_digit

not_valid:
        # Character is not a valid digit. Skip processing.
        j       update_index

valid_digit:
        # A valid digit is found and its numeric value is in $t15.
        # Increment the valid digit counter.
        addi    $s3, $s3, 1

        # Depending on the index (first half: indices 0-4, second half: indices 5-9)
        li      $t16, 5
        blt     $s0, $t16, add_first

        # Add digit value to second half sum (H)
        add     $s2, $s2, $t15
        j       update_index

add_first:
        # Add digit value to first half sum (G)
        add     $s1, $s1, $t15

update_index:
        addi    $s0, $s0, 1      # Move to the next character index
        j       loop

finish_loop:
        # -------------------------------
        # After the loop: if no valid digits were found, print "N/A"
        # Otherwise, compute G - H and print the result.
        # -------------------------------
        beq     $s3, $zero, print_na

        # Compute the difference: result = G - H
        sub     $t17, $s1, $s2

        # Print the difference as a decimal integer.
        li      $v0, 1           # Syscall for print integer
        move    $a0, $t17
        syscall
        j       exit_program

print_na:
        # Print "N/A" if no valid digit was encountered.
        li      $v0, 4           # Syscall for print string
        la      $a0, outputNA
        syscall

exit_program:
        # Exit the program.
        li      $v0, 10          # Syscall for exit
        syscall

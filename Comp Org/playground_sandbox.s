    .data
SpaceInput: .space 1002         # Up to 1000 chars + newline + null term
null_msg:   .asciiz "NULL"
semicolon:  .asciiz ";"
.align 2
strint:     .space 4000         # Array for integer results

    .text
    .globl main

#------------------------------------------------------------
# main: read string, strip newline, call process_string, then
#       print strint[0..count-1] per spec.
#------------------------------------------------------------
main:
    # Hard‑code N=30, compute M=N-10 into $s7
    li   $t0, 30
    li   $t1, 10
    sub  $s7, $t0, $t1        # $s7 = 20

    # Read user string
    li   $v0, 8
    la   $a0, SpaceInput
    li   $a1, 1002
    syscall

    # Strip the newline
    la   $t7, SpaceInput
strip_nl:
    lb   $t0, 0($t7)
    beqz $t0, done_strip
    li   $t8, 0x0A
    beq  $t0, $t8, do_null
    addi $t7, $t7, 1
    j    strip_nl
do_null:
    sb   $zero, 0($t7)
done_strip:

    # Call process_string(strptr → $a0, arrptr → $a1)
    la   $a0, SpaceInput
    la   $a1, strint
    jal  process_string
    # returns: $v0 = substring count

    # Print them out
    move $t0, $v0        # count
    li   $t1, 0          # idx
    la   $t2, strint

print_loop:
    beq  $t1, $t0, exit_main

    lw   $t3, 0($t2)     # value
    li   $t4, 0x7FFFFFFF
    beq  $t3, $t4, print_null

    # print integer
    li   $v0, 1
    move $a0, $t3
    syscall
    j    do_semicolon

print_null:
    li   $v0, 4
    la   $a0, null_msg
    syscall

do_semicolon:
    addi $t1, $t1, 1
    beq  $t1, $t0, skip_sem
    li   $v0, 4
    la   $a0, semicolon
    syscall
skip_sem:
    addi $t2, $t2, 4
    j    print_loop

exit_main:
    li   $v0, 10
    syscall


#------------------------------------------------------------
# process_string(strptr in $a0, arrptr in $a1):
#   uses $s0=arrptr, $s1=strptr, $s2=count,
#   pushes each substring addr, calls get_substring_value,
#   pops its return and stores, advances pointers.
#   Returns count in $v0.
#------------------------------------------------------------
process_string:
    # save $ra and $s0–$s2
    addi $sp, $sp, -16
    sw   $ra,    0($sp)
    sw   $s0,    4($sp)
    sw   $s1,    8($sp)
    sw   $s2,   12($sp)

    move $s0, $a1        # arrptr
    move $s1, $a0        # strptr
    li   $s2, 0          # count = 0

ps_loop:
    lb   $t0, 0($s1)
    beqz $t0, ps_done

    # push substring address
    addi $sp, $sp, -4
    sw   $s1, 0($sp)

    jal  get_substring_value

    # pop return value into $t1
    lw   $t1, 0($sp)
    addi $sp, $sp, 4

    # store into array
    sw   $t1, 0($s0)
    addi $s0, $s0, 4
    addi $s2, $s2, 1
    addi $s1, $s1, 10
    j    ps_loop

ps_done:
    move $v0, $s2       # return count

    # restore $ra and $s0–$s2
    lw   $ra,    0($sp)
    lw   $s0,    4($sp)
    lw   $s1,    8($sp)
    lw   $s2,   12($sp)
    addi $sp, $sp, 16
    jr   $ra


#------------------------------------------------------------
# get_substring_value:
#   pops one word off stack → $a0,
#   processes exactly 10 chars, sums G,H base‑N digits,
#   pushes G–H or 0x7FFFFFFF back on stack.
#   preserves $s1–$s3.
#------------------------------------------------------------
get_substring_value:
    # pop substring address into $a0
    lw   $a0, 0($sp)
    addi $sp, $sp, 4

    # save $s1–$s3
    addi $sp, $sp, -12
    sw   $s1, 8($sp)
    sw   $s2, 4($sp)
    sw   $s3, 0($sp)

    # init
    li   $t5, 0      # char index
    li   $s1, 0      # G
    li   $s2, 0      # H
    li   $s3, 0      # valid-digit count

gsv_loop:
    bge  $t5, 10, gsv_done
    lb   $t6, 0($a0)
    beqz $t6, gsv_pad
    j    gsv_check

gsv_pad:
    li   $t6, 32      # space

gsv_check:
    # '0'–'9'?
    li   $t7, 48
    li   $t8, 57
    blt  $t6, $t7, gsv_low
    bgt  $t6, $t8, gsv_low
    sub  $t9, $t6, $t7
    j    gsv_valid

gsv_low:
    # 'a'–('a'+M‑1)?
    li   $t7, 97
    blt  $t6, $t7, gsv_up
    add  $t8, $t7, $s7
    bge  $t6, $t8, gsv_up
    sub  $t9, $t6, $t7
    addi $t9, $t9, 10
    j    gsv_valid

gsv_up:
    # 'A'–('A'+M‑1)?
    li   $t7, 65
    blt  $t6, $t7, gsv_next
    add  $t8, $t7, $s7
    bge  $t6, $t8, gsv_next
    sub  $t9, $t6, $t7
    addi $t9, $t9, 10

gsv_valid:
    addi $s3, $s3, 1
    li   $t7, 5
    blt  $t5, $t7, gsv_addG
    add  $s2, $s2, $t9
    j    gsv_next

gsv_addG:
    add  $s1, $s1, $t9

gsv_next:
    addi $t5, $t5, 1
    addi $a0, $a0, 1
    j    gsv_loop

gsv_done:
    # choose return in $t0
    beqz $s3, gsv_null
    sub  $t0, $s1, $s2
    j    gsv_store

gsv_null:
    li   $t0, 0x7FFFFFFF

gsv_store:
    # restore $s1–$s3
    lw   $s1, 8($sp)
    lw   $s2, 4($sp)
    lw   $s3, 0($sp)
    addi $sp, $sp, 12

    # push return
    addi $sp, $sp, -4
    sw   $t0, 0($sp)
    jr   $ra

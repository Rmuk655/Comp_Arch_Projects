# Assume x3 = 0x10000000
# Program 1: Multiply a value at 0x10000000 by 9 using only add and ld
addi     x3, x0, 3       # x3 = 3
addi     x5, x3,3        # x5 = x3+3
add     x6, x5, x5        # x6 = val * 2
add     x7, x6, x6        # x7 = val * 4
add     x8, x7, x7        # x8 = val * 8
add     x11, x8, x5       # x11 = val * 8 + val = val * 9

# verify
addi    x12, x0, 54
bne     x11, x12, fail


# Program 2: Sum 10 consecutive 64-bit integers starting from 10

    addi    x4, x0, 0       # sum = 0
    addi    x5, x0, 10      # i = 10 (start)
    addi    x7, x0, 20      # limit = 20 (stop before 20)

LOOP:
    add     x4, x4, x5      # sum += i
    addi    x5, x5, 1       # i++
    blt     x5, x7, LOOP    # loop while i < 20

    add     x10, x4, x0     # move sum to x10 (result)



    addi    x11, x0, 145
    bne     x10, x11, fail

pass:
addi x10,x0,0       #exit with error code 0
ecall

fail:
addi x10,x0,1       #exit with error code 1
ecall
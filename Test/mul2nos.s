main:
    addi x4, x0, 4
    addi x5, x0, 5
    addi x6, x0, 1
    addi x7, x4, 0
    bne x5, x6, Loop
Loop:
    add x4, x4, x7
    sub x5, x5, x6
    bne x5, x6, Loop
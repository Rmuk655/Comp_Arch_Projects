main:
    addi x4, x0, 4
    addi x5, x0, 5
    addi x6, x0, 3
    addi x7, x0, 4
    addi x8, x0, 1
    addi x9, x4, 0
    addi x10, x5, 0
    addi x11, x6, 0
    addi x12, x7, 0
    addi x13, x4, 0
    addi x14, x5, 0
    addi x15, x6, 0
    addi x16, x7, 0
    bne x9, x8, Loop1
    bne x10, x8, Loop2
    bne x11, x8, Loop3
    bne x12, x8, Loop4
Loop1:
    add x15, x15, x6
    sub x9, x9, x8
    bne x9, x8, Loop1
Loop2:
    add x16, x16, x7
    sub x10, x10, x8
    bne x10, x8, Loop2
Loop3:
    add x14, x14, x5
    sub x11, x11, x8
    bne x11, x8, Loop3
Loop4:
    add x13, x13, x4
    sub x12, x12, x8
    bne x12, x8, Loop4
end:
    sub x17, x15, x16
    add x18, x13, x14
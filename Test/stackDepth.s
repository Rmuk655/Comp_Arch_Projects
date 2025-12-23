    lui x2, 0x10000
    addi x5, x0, 1000          # Simulate 10 pushes (change to 1000 for deeper test)

stack_loop:
    sw x5, 0(x2)
    addi x2, x2, -4
    addi x5, x5, -1
    bne x5, x0, stack_loop

    addi x10, x0, 0
    ecall

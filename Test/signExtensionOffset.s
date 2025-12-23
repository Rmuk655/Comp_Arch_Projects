    lui x2, 0x10000
    addi x1, x0, -1          # x1 = 0xFFFFFFFF
    sw   x1, -4(x2)          # Store at (x2 - 4)
    addi x1, x0, 0           # Clear x1
    lw   x1, -4(x2)          # Load from (x2 - 4)

    addi x10, x0, -1
    bne x1, x10, fail_signext

    addi x10, x0, 0
    ecall

fail_signext:
    addi x10, x0, 2
    ecall

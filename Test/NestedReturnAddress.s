    lui x2, 0x10000
    addi x10, x0, 42
    jal x1, funcA

    addi x11, x0, 42
    bne x10, x11, fail_nested

    addi x10, x0, 0
    ecall

fail_nested:
    addi x10, x0, 4
    ecall

funcA:
    addi x2, x2, -16
    sw x1, 12(x2)
    jal x1, funcB
    lw x1, 12(x2)
    addi x2, x2, 16
    jalr x0, 0(x1)

funcB:
    addi x2, x2, -16
    sw x1, 12(x2)
    add x10, x10, x0        # Touch x10 to ensure preserved
    lw x1, 12(x2)
    addi x2, x2, 16
    jalr x0, 0(x1)

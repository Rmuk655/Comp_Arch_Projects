    lui x2, 0x10000
    lw x1, 0(x2)             # Load from memory we never wrote to

    addi x10, x0, 0
    bne x1, x10, fail_uninit

    addi x10, x0, 0
    ecall

fail_uninit:
    addi x10, x0, 3
    ecall

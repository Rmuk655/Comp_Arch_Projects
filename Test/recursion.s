    lui x2, 0x10000
    addi x5, x0, 3
    jal x1, 32

    addi x11, x0, 3
    bne  x10, x11, fail

    addi x10, x0, 0
    ecall

fail:
    addi x10, x0, 2
    ecall

# At PC+8: recurse function
    addi x2, x2, -16
    sw   x1, 12(x2)
    sw   x5, 8(x2)

    beq  x5, x0, base_case

    addi x5, x5, -1
    jal x1, -32
    
    lw   x5, 8(x2)
    addi x10, x10, 1

base_case:
    lw   x1, 12(x2)
    addi x2, x2, 16
    jalr x0, 0(x1)

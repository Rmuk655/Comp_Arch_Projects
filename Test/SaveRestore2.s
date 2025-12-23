    lui x2, 0x10000          # Stack pointer at 0x10000000
    addi x1, x0, 0x34        # x1 = 0x1234 (lower 12 bits first)
    slli x1, x1, 4           # x1 <<= 4 → x1 = 0x340
    addi x1, x1, 0x4         # x1 = 0x344
    sw   x1, 0(x2)           # Store x1 to memory

    addi x1, x0, 0           # Clear x1
    lw   x1, 0(x2)           # Reload from memory

    # x10 = expected = 0x344
    addi x10, x0, 0x344
    bne x1, x10, fail_ra

    addi x10, x0, 0          # Success
    ecall

fail_ra:
    addi x10, x0, 1
    ecall

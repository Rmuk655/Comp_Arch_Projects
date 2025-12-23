    lui x2, 0x10000
    jal x1, 12

    # If returned normally, error code 0
    addi x10, x0, 0
    ecall

# At PC+8: bad_func
    addi x2, x2, -16
    sw   x1, 0(x2)  # Save ra at wrong offset

    jalr x0, 0(x1)  # Return without restoring SP or ra

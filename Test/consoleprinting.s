main:
    addi x10, x0, 3      # a0 = 3
    addi x11, x0, 5      # a1 = 5
    jal x1, loopPrint    # call loopPrint
    jal x0, exit         # exit after done

loopPrint:
    addi x5, x10, 0       # x5 = a0 (t0 = a0)
    addi x6, x11, 0       # x6 = a1 (t1 = a1)

loop:
    addi x10, x5, 0       # x10 = x5 (a0 = t0)
    addi x17, x12, 0      # x17 = x12 (a7 = a2)
#    ecall                 # print value

    addi x17, x0, 4       # x17 = 4 (a7 = 4)
    lui  x10, 0x10010     # address of delimiter (you must define it to match 0x10010000 etc.)
    addi x10, x10, 0      # x10 = x10 + offset (adjust if needed)
#    ecall

    addi x5, x5, 1        # t0++

    bge  x6, x5, loop     # t1 >= t0 => loop (emulating ble)

    jalr x0, 0(x1)        # return (jr x1)

exit:
    addi x10, x0, 0      # x10 = 0 
    ecall                # exit with exit code in x10

# loop_memtest.s

    addi x1, x0, 0         # base = 0
    addi x2, x0, 16        # loop count = 16
    addi x3, x0, 100       # value = 100

loop1:
    sw x3, 0(x1)           # store 100 to mem[0] repeatedly
    lw x4, 0(x1)           # load from mem[0] repeatedly
    addi x2, x2, -1
    bne x2, x0, loop1      # loop until x2 == 0

    addi x5, x0, 20        # loop count = 20
    addi x6, x0, 0         # address offset
    addi x7, x0, 1         # value = 1

loop2:
    sw x7, 0(x6)           # write to mem[offset]
    lw x8, 0(x6)           # read back
    addi x6, x6, 4         # move to next block
    addi x5, x5, -1
    bne x5, x0, loop2      # touch 20 distinct blocks

    # Access reused locations again
    lw x9, 0(x0)
    lw x10, 16(x0)
    lw x11, 32(x0)

_start:
    # Assume inputs a in x10, b in x11
    # Loop start:
        addi x10, x0, 98       # a = 48
        addi x11, x0, 56       # b = 18 
loop_start:
    sub x5, x10, x11       # t0 = a - b
    beq x5, x0, done       # if a == b, done

    srai x6, x5, 31         # t1 = sign bit of (a-b), 0 if >=0, -1 if negative

    beq x6, x0, a_ge_b     # if sign == 0 (a >= b), branch to a_ge_b

    # else a < b
    sub x11, x11, x10      # b = b - a
    beq x0, x0, loop_start # unconditional branch simulated by always taken branch

a_ge_b:
    sub x10, x10, x11      # a = a - b
    beq x0, x0, loop_start # unconditional branch simulated by always taken branch

done:
    # gcd result in x10
    # prepare for exit ecall with 0 in a0 (x10)
    addi x17, x0, 14       # a7 = 14 (ecall exit)
    bne x17, x10, fail
pass:    
    addi x10, x0, 0        # a0 = 0
    ecall
fail:
    addi x10, x0, 1        # a0 = 1
    ecall
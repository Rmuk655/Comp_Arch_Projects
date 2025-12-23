    # Assume x10 = multiplicand (set to 3)
    # Assume x11 = multiplier   (set to 4)
    # Result will be stored in x9 = 0 initially
    # Loop runs 32 times (for each bit in multiplier)

    addi x10, x0, 3        # x10 = 3 (multiplicand)
    addi x11, x0, 4        # x11 = 4 (multiplier)
    addi x9, x0, 0         # x9 = 0 (result)
    addi x5, x0, 32        # x5 = 32 (loop counter)
    addi x6, x0, 0         # x6 = temp for checking LSB
    addi x7, x0, 0         # x7 = unused (optional temp)
    addi x8, x0, 0         # x8 = unused (optional temp)
    #ebreak
mult_loop:
    andi x6, x11, 1        # check LSB of multiplier
    beq x6, x0, skip_add   # if 0, skip addition
    add  x9, x9, x10       # else add multiplicand to result

skip_add:
    slli x10, x10, 1       # shift multiplicand left (multiply by 2)
    srli x11, x11, 1       # shift multiplier right (divide by 2)
    addi x5, x5, -1        # decrement loop counter
    bne x5, x0, mult_loop  # if not done, loop again

    # program ends here; result is in x9
    # use x9 as needed

end:
    addi x15,x0, 12
    bne x9,x15,fail
    addi x10,x0,0 #success
    ecall
fail:
    addi x10,x0,1 # exit code 1
    ecall

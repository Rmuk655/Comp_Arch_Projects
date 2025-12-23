    # Registers:
    # x30 = status (0 = pass, >0 fail code)
    # x31 = temp compare

    # Clear status (x30 = 0)
    addi x30, x0, 0

    # --- Test 1: ADD and ADDI ---
    addi x1, x0, 10        # x1 = 10
    addi x2, x0, 20        # x2 = 20
    add x3, x1, x2         # x3 = 30

    # set x31 = 30
    addi x31, x0, 10
    addi x31, x31, 10
    addi x31, x31, 10      # x31 = 30

    bne x3, x31, fail_add

    # --- Test 2: SUB ---
    sub x4, x2, x1         # x4 = 10

    addi x31, x0, 10       # x31 = 10

    bne x4, x31, fail_sub

    # --- Test 3: ANDI ---
    andi x5, x1, 15        # 0xF = 15 decimal

    addi x31, x0, 10       # x31 = 10

    bne x5, x31, fail_andi

    # --- Test 4: ORI ---
    ori x6, x1, 15         # 10 | 15 = 15

    addi x31, x0, 15       # x31 = 15

    bne x6, x31, fail_ori

    # --- Test 5: XORI ---
    xori x7, x1, 15        # 10 ^ 15 = 5

    addi x31, x0, 5        # x31 = 5

    bne x7, x31, fail_xori

    # --- Test 6: SLLI ---
    slli x8, x1, 1         # 10 << 1 = 20

    addi x31, x0, 20       # x31 = 20

    bne x8, x31, fail_slli

    # --- Test 7: SRLI ---
    srli x9, x2, 2         # 20 >> 2 = 5

    addi x31, x0, 5        # x31 = 5

    bne x9, x31, fail_srli

    # --- Test 8: SRAI ---
    addi x10, x0, -16      # x10 = -16
    srai x11, x10, 2       # x11 = -4

    addi x31, x0, -4       # x31 = -4

    bne x11, x31, fail_srai

    # --- Test 9: Memory store/load ---
    # base address simulation (just a number, no real mem)
    addi x12, x0, 100
    addi x13, x0, 0x12345678

    sw x13, 0(x12)
    lw x14, 0(x12)

    # Compare with 0x12345678 in x31:
    # Since we can't use li with large immediate, build x31 in pieces

    # 0x12345678 = 305419896 decimal
    # do it via multiple adds (not optimal but no li):

    addi x31, x0, 305000000    # too large immediate? No, addi is 12-bit signed immediate only

    # We must build it differently using lui + addi or use small constants in test sim


    lui x31, 0x12345           # x31 = 0x12345000
    addi x31, x31, 0x678      # x31 = 0x12345678

    bne x14, x31, fail_mem


    # All tests passed: set status 0 in x10
    addi x10, x0, 0
    ecall
  

    # End loop (simulate done)  

    # Some test failed: set status 1 in x10
    
    loop:
    addi x10, x0, 1
    ecall

fail_add:
    addi x30, x0, 1
    beq x0, x0, loop

fail_sub:
    addi x30, x0, 2
    beq x0, x0, loop

fail_andi:
    addi x30, x0, 3
    beq x0, x0, loop

fail_ori:
    addi x30, x0, 4
    beq x0, x0, loop

fail_xori:
    addi x30, x0, 5
    beq x0, x0, loop

fail_slli:
    addi x30, x0, 6
    beq x0, x0, loop

fail_srli:
    addi x30, x0, 7
    beq x0, x0, loop

fail_srai:
    addi x30, x0, 8
    beq x0, x0, loop

fail_mem:
    addi x30, x0, 9
    beq x0, x0, loop


# Load full 32-bit value into x5
lui x5, 0xAB     # x5 = 0xAB000
addi x5, x5, 0xCD # x5 = 0xABCD

# Or this version if 0xABCD = 0x0000ABCD fails:
lui x5, 0xA         # upper 20 bits = 0xA000
addi x5, x5, 0xBCD  # lower 12 bits = 0xBCD → 0xA000 + 0xBCD = 0xABCD

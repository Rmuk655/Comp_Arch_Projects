# Loop with multiple hazards
addi x1, x0, 10     # Line 1: loop counter
addi x2, x0, 0      # Line 2: accumulator
loop:
lw x3, 0(x4)        # Line 3: load data
add x2, x2, x3      # Line 4: accumulate (load-use hazard)
addi x4, x4, 4      # Line 5: increment pointer
addi x1, x1, -1     # Line 6: decrement counter
bne x1, x0, loop    # Line 7: branch (RAW hazard with counter)
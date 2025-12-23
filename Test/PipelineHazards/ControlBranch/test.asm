# Branch hazard example
add x1, x2, x3      # Line 1: normal instruction
beq x1, x4, target  # Line 2: branch instruction
sub x5, x6, x7      # Line 3: instruction after branch
mul x8, x9, x10     # Line 4: might be flushed
target:
or x11, x12, x13    # Line 5: branch target
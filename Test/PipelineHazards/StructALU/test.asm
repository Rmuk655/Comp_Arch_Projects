# Multiple ALU instructions in close succession
add x1, x2, x3      # Line 1: uses ALU
sub x4, x5, x6      # Line 2: needs ALU (potential structural stall)
and x7, x8, x9      # Line 3: needs ALU (potential structural stall)
or x10, x11, x12    # Line 4: needs ALU (potential structural stall)
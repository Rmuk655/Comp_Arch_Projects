# Complex RAW hazard chain
add x1, x2, x3      # Line 1: writes x1
sub x4, x1, x5      # Line 2: reads x1, writes x4 (RAW on x1)
mul x6, x4, x7      # Line 3: reads x4, writes x6 (RAW on x4)
div x8, x6, x1      # Line 4: reads x6 and x1, writes x8 (RAW on x6)
or x9, x8, x10      # Line 5: reads x8, writes x9 (RAW on x8)
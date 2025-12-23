# WAW hazard example
add x1, x2, x3      # Line 1: writes x1
sub x4, x5, x6      # Line 2: no hazard
mul x1, x7, x8      # Line 3: writes x1 (WAW hazard with Line 1)
or x9, x1, x10      # Line 4: reads x1
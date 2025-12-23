# Load-use hazard example
lw x1, 0(x2)        # Line 1: loads into x1 (takes extra cycle)
add x3, x1, x4      # Line 2: uses x1 immediately (load-use hazard)
sub x5, x6, x7      # Line 3: no hazard
mul x8, x1, x9      # Line 4: uses x1 (resolved by pipeline)
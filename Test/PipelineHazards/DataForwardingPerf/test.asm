# Performance comparison: with and without forwarding
add x1, x2, x3      # writes x1
sub x4, x1, x5      # reads x1 (RAW)
lw x6, 0(x7)        # load instruction  
add x8, x6, x9      # load-use hazard
mul x10, x1, x11    # reads x1 again
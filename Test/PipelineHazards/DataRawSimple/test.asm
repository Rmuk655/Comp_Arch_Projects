# Simple RAW hazard example
add x1, x2, x3    # Instruction 1: writes to x1
sub x4, x1, x5    # Instruction 2: reads from x1 (RAW hazard)
or x6, x7, x8     # Instruction 3: no hazard
and x9, x1, x10   # Instruction 4: reads from x1 (RAW hazard)
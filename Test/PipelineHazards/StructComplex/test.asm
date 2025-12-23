# Various multiplication and division instructions
mul x1, x2, x3     # Line 1: uses Mul/Div unit
mulw x4, x5, x6    # Line 2: structural stall (same unit)
div x7, x8, x9      # Line 3: structural stall (same unit)
divuw x10, x11, x12  # Line 4: structural stall (same unit)
add x13, x14, x15   # Line 5: no conflict (different unit)
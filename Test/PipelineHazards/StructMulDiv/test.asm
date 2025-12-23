# Two multiplication instructions competing for the same Mul/Div unit
mul x1, x2, x3      # Line 1: uses Mul/Div unit (cycles 1-4)
div x4, x5, x6      # Line 2: needs Mul/Div unit (structural stall)
add x7, x8, x9      # Line 3: can proceed (uses ALU)
mul x10, x11, x12  # Line 4: another Mul/Div conflict
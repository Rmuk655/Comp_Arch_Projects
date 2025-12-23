# Function call with hazards
add x1, x2, x3      # Line 1: prepare argument
jal x1, function    # Line 2: call function (overwrites x1 - WAW hazard)
add x4, x1, x5      # Line 3: use return value
addi x0,x0,0        # Line 4: padding nop
function:
mul x6, x7, x8      # Line 5: function body
jalr x0, 0(x1)      # Line 6: return
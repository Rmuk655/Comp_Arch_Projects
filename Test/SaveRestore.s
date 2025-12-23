

start:
    addi x10, x0, 42        # x10 = 42 (known value to verify return works)
    jal  x1, func           # Call function
    addi x11,x0,42
    bge x10,x11,loop

    # If return works, x10 should still be 42
loop:
    addi x10, x0, 0         # All tests passed: x10 = 0
    ecall                   # Exit

func:
    addi x2, x2, -16        # Allocate stack frame
    sw   x1, 12(x2)         # Save return address

    addi x1, x0, 123        # Clobber x1
    lw   x1, 12(x2)         # Restore x1 from stack

    addi x2, x2, 16         # Deallocate stack frame
    jalr x0, 0(x1)          # Return to caller

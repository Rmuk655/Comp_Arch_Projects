    jal x1, mult_loop   # Call mult_loop
end:
    ecall               # Exit the program

# Function: mult_loop
# Saves return address, calls inner_func, restores return address, and returns
mult_loop:
    addi sp, sp, -4     # Make space on stack
    sw x1, 0(sp)        # Save caller's return address (x1)

    jal x1, inner_func  # Call inner_func (overwrites x1)

    lw x1, 0(sp)        # Restore saved return address
    addi sp, sp, 4      # Clean up stack
    jalr x0, x1, 0      # Return to caller

# Function: inner_func
# Performs some work and returns
inner_func:
    addi x5, x0, 42     # Do something (store 42 in x5)
    jalr x0, x1, 0      # Return to caller

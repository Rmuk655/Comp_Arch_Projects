# Entry: x10 = n
# Output: x10 = n!

    addi x10, x0, 5         # Set n = 5
    addi x11, x0, 120       # Expected result = 5!

    jal x1, factorial       # Call factorial(n)

    bne x10, x11, fail      # Check result
    addi x10, x0, 0         # Success exit code
    ecall

fail:
    addi x10, x0, 1         # Failure exit code
    ecall

# -----------------------------------------
# Function: factorial
# Input:  x10 = n
# Output: x10 = n!
# -----------------------------------------
factorial:
    addi x5, x0, 1          # x5 = 1

    blt x10, x5, base_case  # if n < 1 → return 1
    beq x10, x5, base_case  # if n == 1 → return 1

    # Save return address and n
    addi x2, x2, -8         # Allocate 8 bytes on stack
    sw x1, 4(x2)            # Save return address
    sw x10, 0(x2)           # Save n

    addi x10, x10, -1       # x10 = n - 1
    jal x1, factorial       # Call factorial(n - 1)

    # Restore n and return address
    lw x5, 0(x2)            # x5 = original n
    lw x1, 4(x2)            # Restore return address
    addi x2, x2, 8          # Deallocate stack

    # Multiply x5 * x10 using loop (no mul)
    addi x6, x0, 0          # x6 = result = 0
    addi x7, x0, 0          # x7 = counter = 0

multiply_loop:
    beq x7, x5, multiply_done
    add x6, x6, x10         # x6 += x10
    addi x7, x7, 1
    jal x0, multiply_loop

multiply_done:
    addi x10, x6, 0         # Move result to x10
    jalr x0, 0(x1)          # Return

base_case:
    addi x10, x0, 1         # Return 1
    jalr x0, 0(x1)          # Return to caller

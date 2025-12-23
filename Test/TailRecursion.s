start:
    lui x2, 0x10000         # Set up stack pointer (x2 = 0x10000000)
    addi x5, x0, 3          # x5 = 3 (initial input to tail-recursive function)

    jal x1, tail_recurse    # Call tail-recursive function

    # After return, x10 should contain the expected result: 3
    addi x11, x0, 3
    bne  x10, x11, fail     # If not equal, test failed

    addi x10, x0, 0         # Test passed (exit code 0)
    ecall                   # Exit

fail:
    addi x10, x0, 4         # Test failed (exit code 4)
    ecall                   # Exit

# Tail-recursive function: decrements x5 to 0 and sets x10 to final result
tail_recurse:
    beq  x5, x0, tail_base  # Base case: if x5 == 0, return

    addi x10, x0, 0         # Dummy work: x10 = 0 (cleared each step)
    addi x5, x5, -1         # Decrement x5
    jal x0, tail_recurse    # Tail call: jump (no return address)

tail_base:
    addi x10, x5, 3         # Final result: x5 (which is 0) + 3 = 3
    jalr x0, 0(x1)          # Return to caller

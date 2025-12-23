    addi x5, x0, 1         # Set  a_real
    addi x6, x0, 2         # Set  a_imag
    addi x7, x0, 3         # Set  b_real
    addi x8, x0, 4         # Set  b_imag

# Initialize all product/temp registers
addi x9,  x0, 0   # a_real * b_real
addi x10, x0, 0   # a_imag * b_imag
addi x12, x0, 0   # a_real * b_imag
addi x13, x0, 0   # a_imag * b_real

# -----------------------------
# x9 = a_real * b_real
addi x15, x0, 0       # counter = 0
loop_rr:
beq  x15, x7, end_loop_rr
add  x9, x9, x5
addi x15, x15, 1
beq  x0, x0, loop_rr
end_loop_rr:

# -----------------------------
# x10 = a_imag * b_imag
addi x16, x0, 0
loop_ii:
beq  x16, x8, end_loop_ii
add  x10, x10, x6
addi x16, x16, 1
beq  x0, x0, loop_ii
end_loop_ii:

# -----------------------------
# x11 = x9 - x10  (real result)
sub x11, x9, x10

# -----------------------------
# x12 = a_real * b_imag
addi x17, x0, 0
loop_rb:
beq  x17, x8, end_loop_rb
add  x12, x12, x5
addi x17, x17, 1
beq  x0, x0, loop_rb
end_loop_rb:

# -----------------------------
# x13 = a_imag * b_real
addi x18, x0, 0
loop_ir:
beq  x18, x7, end_loop_ir
add  x13, x13, x6
addi x18, x18, 1
beq  x0, x0, loop_ir
end_loop_ir:

# -----------------------------
# x14 = x12 + x13 (imaginary result)
add x14, x12, x13

# x11 = real part
# x14 = imaginary part

    addi x15, x0,-5  
    bne x11, x15, fail     # Check real part result
    addi x15, x0, 10  
    bne x14, x15, fail      # Check imaginary part result
     
    addi x10, x0, 0         # Success exit code
    ecall

fail:
    addi x10, x0, 1         # Failure exit code
    ecall

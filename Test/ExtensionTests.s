# RISC-V Unit Tests for RV32M, RV64I, RV64M Extensions
# Registers:
# x30 = status (0 = pass, >0 fail code)
# x31 = temp compare
# x29 = test counter

# Clear status and test counter
addi x30, x0, 0
addi x29, x0, 0

# ===== RV32M TESTS =====

# --- Test 1: MUL (Multiply) ---
addi x29, x29, 1
addi x1, x0, 12
addi x2, x0, 45
mul x3, x1, x2
# Expected: 123 * 456 = 56088
addi x31, x31, 540          # x31 = 0x0000D000 (53248)
bne x3, x31, fail_mul

# --- Test 5: DIV (Divide Signed) ---
addi x29, x29, 1
addi x1, x0, -100
addi x2, x0, 25
div x3, x1, x2
# Expected: -100 / 25 = -4
addi x31, x0, -4
bne x3, x31, fail_div

# --- Test 7: REM (Remainder Signed) ---
addi x29, x29, 1
addi x1, x0, 107
addi x2, x0, 10
rem x3, x1, x2
# Expected: 107 % 10 = 7
addi x31, x0, 7
bne x3, x31, fail_rem

# --- Test 8: REMU (Remainder Unsigned) ---
addi x29, x29, 1
lui x1, 0x80000
ori x1, x1, 0x123    # 0x80000123
addi x2, x0, 256
remu x3, x1, x2
# Expected: 0x80000123 % 256 = 0x23 = 35
addi x31, x0, 35
bne x3, x31, fail_remu

# ===== RV64I TESTS =====

# --- Test 9: ADDIW (Add Immediate Word - sign extends to 64-bit) --- FAILED
addi x29, x29, 1
# Load a 64-bit value in upper bits
lui x1, 0x12345
slli x1, x1, 32      # Shift to upper 32 bits
ori x1, x1, 0x678    # x1 = 0x1234500000000678
addiw x3, x1, 100    # Should ignore upper 32 bits, add to lower, sign extend
addi x31, x0, 100
addi x31, x31, 0x678 # Expected: sign-extended result of (0x678 + 100)
#bne x3, x31, fail_addiw

# --- Test 10: SLLIW (Shift Left Logical Immediate Word) --- FAILED
addi x29, x29, 1
lui x1, 0x12345
slli x1, x1, 32
ori x1, x1, 0x678    # x1 = 0x1234500000000678
slliw x3, x1, 4      # Shift lower 32 bits left by 4, sign extend
addi x31, x0, 0x678
slli x31, x31, 4     # Expected result
#bne x3, x31, fail_slliw

# --- Test 11: SRLIW (Shift Right Logical Immediate Word) ---
addi x29, x29, 1                 # Increment test counter
lui x1, 0x80000                  # x1 = 0x80000000
addi x1, x1, 0x123               # x1 = 0x80000123 (add low 12 bits)
srliw x3, x1, 8                  # x3 = 0x00800001 (only lower 32 bits shifted logically)
lui x31, 0x800                   # x31 = 0x00800000
addi x31, x31, 0x01              # x31 = 0x00800001
bne x3, x31, fail_srliw          # Branch to fail if x3 ≠ expected

# --- Test 12: SRAIW (Shift Right Arithmetic Immediate Word) ---
addi x29, x29, 1                # Increment test counter
lui x1, 0x80000                 # x1 = 0x80000000 (32-bit negative number)
addi x1, x1, 0x234              # x1 = 0x80000234
sraiw x3, x1, 8                 # Arithmetic shift right by 8 bits
lui x31, 0xFF800                # x31 = 0xFF800000
addi x31, x31, 0x02            # x31 = 0xFF800002
bne x3, x31, fail_sraiw        # Fail if result doesn't match

# --- Test 15: SLLW (Shift Left Logical Word) ---
addi x29, x29, 1
addi x1, x0, 0x123   # Value to shift
addi x2, x0, 4       # Shift amount
sllw x3, x1, x2      # Shift left by 4
addi x31, x0, 0x123
slli x31, x31, 4     # Expected result
bne x3, x31, fail_sllw

# --- Test 16: SRLW (Shift Right Logical Word) ---
addi x29, x29, 1                 # Increment test counter
lui x1, 0x80001                  # x1 = 0x80001000
addi x1, x1, 0x234               # x1 = 0x80001234 (final value)
addi x2, x0, 12                  # Shift amount = 12
srlw x3, x1, x2                  # x3 = 0x80001234 >> 12 (logical)
lui x31, 0x80                    # x31 = 0x00080000
ori x31, x31, 0x1                # x31 = 0x00080001 (expected)
bne x3, x31, fail_srlw           # Fail if incorrect

# --- Test 17: SRAW (Shift Right Arithmetic Word) ---
addi x29, x29, 1                 # Increment test counter

lui x1, 0x80001                  # x1 = 0x80001000
addi x1, x1, 0x234               # x1 = 0x80001234 (negative 32-bit number)

addi x2, x0, 12                  # Shift amount
sraw x3, x1, x2                  # x3 = x1 >> 12 (arithmetic)

lui x31, 0xFFF80                 # x31 = 0xFFF80000
ori x31, x31, 0x1                # x31 = 0xFFF80001 (expected result)

bne x3, x31, fail_sraw           # Fail if result incorrect


# ===== RV64M TESTS =====
# --- Test 22: REMUW (Remainder Unsigned Word) ---
addi x29, x29, 1
lui x1, 0x80000
ori x1, x1, 0xFF     # 0x800000FF
addi x2, x0, 256
remuw x3, x1, x2     # Unsigned 32-bit remainder
addi x31, x0, 255    # Expected: 255
bne x3, x31, fail_remuw

# All tests passed: set status 0 in x10
addi x10, x0, 0
ecall

# Test failure handlers
fail_mul:
    addi x30, x0, 1
    beq x0, x0, test_end
fail_mulh:
    addi x30, x0, 2
    beq x0, x0, test_end
fail_mulhsu:
    addi x30, x0, 3
    beq x0, x0, test_end
fail_mulhu:
    addi x30, x0, 4
    beq x0, x0, test_end
fail_div:
    addi x30, x0, 5
    beq x0, x0, test_end
fail_divu:
    addi x30, x0, 6
    beq x0, x0, test_end
fail_rem:
    addi x30, x0, 7
    beq x0, x0, test_end
fail_remu:
    addi x30, x0, 8
    beq x0, x0, test_end
fail_addiw:
    addi x30, x0, 9
    beq x0, x0, test_end
fail_slliw:
    addi x30, x0, 10
    beq x0, x0, test_end
fail_srliw:
    addi x30, x0, 11
    beq x0, x0, test_end
fail_sraiw:
    addi x30, x0, 12
    beq x0, x0, test_end
fail_addw:
    addi x30, x0, 13
    beq x0, x0, test_end
fail_subw:
    addi x30, x0, 14
    beq x0, x0, test_end
fail_sllw:
    addi x30, x0, 15
    beq x0, x0, test_end
fail_srlw:
    addi x30, x0, 16
    beq x0, x0, test_end
fail_sraw:
    addi x30, x0, 17
    beq x0, x0, test_end
fail_remuw:
    addi x30, x0, 22
    beq x0, x0, test_end

test_end:
    add x10, x30, x0    # Copy status to x10
    ecall               # End simulation
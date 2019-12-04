#pragma once

#include <cstdint>

//----------------------------------------------------------------------------------------------------------------------
// Enum representing a type of CPU opcode.
//
// These operations should represent MOST (all?) of the available operations on the Playstation's R3000 MIPS I CPU plus
// some unsupported MIPS II TRAP instructions that were found in the DOOM code - presumably to mark unreachable code
// regions that should NOT be executed (would cause illegal instruction on MIPS I).
//
// Sources of info:
//      https://www.cs.cornell.edu/courses/cs3410/2008fa/MIPS_Vol2.pdf
//      https://problemkaputt.de/psx-spx.htm#cpuspecifications
//      https://opencores.org/projects/plasma/opcodes
//      https://github.com/JaCzekanski/Avocado
//
// Some notes:
//  (1) N.B: IMPORTANT - the so called 'branch delay' slot.
//      For all jumps and branches the instruction AFTER the jump/branch is executed BEFORE the jump is taken.
//      See 'Playstation Emulation Guide' for more on this under 'Branch Delay Slots':
//          https://svkt.org/~simias/guide.pdf
//  (2) N.B: IMPORTANT - the so called 'load delay' slot.
//      For loads the result of that load will not be visible until the 2nd instruction AFTER the load.
//      The behavior is actually even a little more complex than this.
//      See 'Playstation Emulation Guide' for more on this under 'Load Delay Slots':
//          https://svkt.org/~simias/guide.pdf
//----------------------------------------------------------------------------------------------------------------------
enum class CpuOpcode : uint8_t {
    //------------------------------------------------------------------------------------------------------------------
    // [ADD WORD]
    //      Add the SIGNED registers 'S' and 'T' and save in the output register 'D', I.E:
    //          D = S + T
    //      If an OVERFLOW occurs then an EXCEPTION will be raised and the destination register will be UNCHANGED.
    // 
    // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100000
    //------------------------------------------------------------------------------------------------------------------
    ADD,
    //------------------------------------------------------------------------------------------------------------------
    // [ADD IMMEDIATE WORD]
    //      Add the 16-bit SIGNED constant 'I' to 'S' and save in the output register 'T', I.E:
    //          T = S + I
    //      If an OVERFLOW occurs then an EXCEPTION will be raised and the destination register will be UNCHANGED.
    //
    // Encoding: 001000 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    ADDI,
    //------------------------------------------------------------------------------------------------------------------
    // [ADD IMMEDIATE WORD UNSIGNED]
    //      Add the 16-bit SIGNED constant 'I' to 'S' and save in the output register 'T', I.E:
    //          T = S + I
    //      OVERFLOWS are IGNORED by this particular operation.
    //
    // Encoding: 001001 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    ADDIU,
    //------------------------------------------------------------------------------------------------------------------
    // [ADD UNSIGNED WORD]
    //      Add the registers 'S' and 'T' and save in the output register 'D', I.E:
    //          D = S + T
    //      OVERFLOWS are IGNORED by this particular operation.
    // 
    // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100001
    //------------------------------------------------------------------------------------------------------------------
    ADDU,
    //------------------------------------------------------------------------------------------------------------------
    // [AND]
    //      Do a bitwise logical AND of registers 'S' and 'T' and store in register 'D', I.E:
    //          D = S & T
    //
    // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100100
    //------------------------------------------------------------------------------------------------------------------    
    AND,
    //------------------------------------------------------------------------------------------------------------------
    // [AND IMMEDIATE]
    //      Do a bitwise logical AND of register 'S' with 16-bit constant 'I' and store in register 'T', I.E:
    //          T = S & I
    //
    // Encoding: 001100 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    ANDI,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON EQUAL]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset 'I' if 'S' == 'T' where 'S', 'T' are registers.
    //
    // Encoding: 000100 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BEQ,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON GREATER THAN OR EQUAL TO ZERO]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset 'I' if register 'S' is >= 0.
    //
    // Encoding: 000001 SSSSS 00001 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BGEZ,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON GREATER THAN OR EQUAL TO ZERO AND LINK]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset if register 'S' is >= 0 and 'link'.
    //      The address of the 2nd (note: NOT 1st!) instruction following the branch is saved in register 'RA' (R31).
    //
    // Encoding: 000001 SSSSS 10001 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BGEZAL,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON GREATER THAN ZERO]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset 'I' if register 'S' > 0.
    //
    // Encoding: 000111 SSSSS ----- IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BGTZ,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON LESS THAN OR EQUAL TO ZERO]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset if register 'S' is <= 0.
    //
    // Encoding: 000110 SSSSS ----- IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BLEZ,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON LESS THAN ZERO]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset if register 'S' is < 0.
    //
    // Encoding: 000001 SSSSS 00000 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BLTZ,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON LESS THAN ZERO AND LINK]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset if register 'S' is < 0 and 'link'.
    //      The address of the 2nd (note: NOT 1st!) instruction following the branch is saved in register 'RA' (R31).
    //
    // Encoding: 000001 SSSSS 10000 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BLTZAL,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON NOT EQUAL]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset 'I' if 'S' != 'T' where 'S', 'T' are registers.
    //
    // Encoding: 000101 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BNE,
    //------------------------------------------------------------------------------------------------------------------
    // [BREAK]
    //      Triggers a breakpoint exception and transfers control to the exception handler.
    //      The 'I' (Code) field is used in the handling of the exception, as an argument.
    //
    // Encoding: 000000 IIIII IIIII IIIII IIIII 001101
    //------------------------------------------------------------------------------------------------------------------
    BREAK,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE CONTROL WORD FROM COPROCESSOR 2]
    //      Move a value from a coprocessor 2 control register 'D' and save the result in register 'T':
    //          T = COP2_C[D]
    //
    // Encoding: 010010 00010 TTTTT DDDDD ----- ------
    //------------------------------------------------------------------------------------------------------------------
    CFC2,
    //------------------------------------------------------------------------------------------------------------------
    // [COPROCESSOR OPERATION TO COPROCESSOR 2]
    //      Perform an operation on coprocessor 2, which is the 'Geometry Transformation Engine' (GTE) on the PSX.
    //      The immediate parameter 'I' is passed to the coprocessor so signal what type of operation is intended.
    //      To see what this means refer to documentation on GTE operations.
    //
    // Encoding: 010010 1IIII IIIII IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    COP2,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE CONTROL WORD TO COPROCESSOR 2]
    //      Move a value from register 'T' and save to coprocessor 2 control register 'D':
    //          COP2_C[D] = T
    //
    // Encoding: 010010 00110 TTTTT DDDDD ----- ------
    //------------------------------------------------------------------------------------------------------------------
    CTC2,
    //------------------------------------------------------------------------------------------------------------------
    // [DIVIDE WORD]
    //      Divide SIGNED register 'S' by SIGNED register 'T' and save the result in special register 'LO'.
    //      The remainder is stored in special register 'HI'.
    //      If dividing by zero then the result is UNPREDICTABLE but no error occurs.
    //
    // Encoding: 000000 SSSSS TTTTT ----- ----- 011010
    //------------------------------------------------------------------------------------------------------------------
    DIV,
    //------------------------------------------------------------------------------------------------------------------
    // [DIVIDE UNSIGNED WORD]
    //      Divide UNSIGNED register 'S' by UNSIGNED register 'T' and save the result in special register 'LO'.
    //      The remainder is stored in special register 'HI'.
    //      If dividing by zero then the result is UNPREDICTABLE but no error occurs.
    //
    // Encoding: 000000 SSSSS TTTTT ----- ----- 011011
    //------------------------------------------------------------------------------------------------------------------
    DIVU,
    //------------------------------------------------------------------------------------------------------------------
    // [JUMP]
    //      Branch to the given program 32-bit WORD index 'I' (note: NOT byte!) within the current 256 MB memory region.
    //
    // Encoding: 000010 IIIII IIIII IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    J,
    //------------------------------------------------------------------------------------------------------------------
    // [JUMP AND LINK]
    //      Branch to the given program 32-bit WORD index 'I' (note: NOT byte!) within the current 256 MB memory region.
    //      The address of the 2nd (note: NOT 1st!) instruction following the branch is also saved in register 'RA' (R31).
    //
    // Encoding: 000011 IIIII IIIII IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    JAL,
    //------------------------------------------------------------------------------------------------------------------
    // [JUMP AND LINK REGISTER]
    //      Branch to the location specified in register 'S' and place the return address in register 'D'.
    //      Note: the return address is the address of the 2nd (note: NOT 1st!) instruction following the branch.
    //
    // Encoding: 000000 SSSSS ----- DDDDD ----- 001001
    //------------------------------------------------------------------------------------------------------------------
    JALR,
    //------------------------------------------------------------------------------------------------------------------
    // [JUMP REGISTER]
    //      Branch to the location specified in register 'S'.
    //
    // Encoding: 000000 SSSSS ----- ----- ----- 001000
    //------------------------------------------------------------------------------------------------------------------
    JR,
    //------------------------------------------------------------------------------------------------------------------
    // [LOAD BYTE]
    //      Load the contents of the SIGNED byte pointed to by register 'S' plus the 16-bit SIGNED constant offset 
    //      'I' and store in register 'T'.
    //          T = S[I]
    //
    // Encoding: 100000 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    LB,
    //------------------------------------------------------------------------------------------------------------------
    // [LOAD BYTE UNSIGNED]
    //      Load the contents of the UNSIGNED byte pointed to by register 'S' plus the 16-bit SIGNED constant offset 
    //      'I' and store in register 'T'.
    //          T = S[I]
    //
    // Encoding: 100100 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    LBU,
    //------------------------------------------------------------------------------------------------------------------
    // [LOAD HALF WORD]
    //      Load the contents of the ALIGNED and SIGNED 16-bit half word pointed to by register 'S' plus the 16-bit
    //      SIGNED constant offset 'I' and store in register 'T'.
    //          T = S[I]
    //
    // Encoding: 100001 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    LH,
    //------------------------------------------------------------------------------------------------------------------
    // [LOAD HALF WORD UNSIGNED]
    //      Load the contents of the ALIGNED and UNSIGNED 16-bit half word pointed to by register 'S' plus the 16-bit
    //      SIGNED constant offset 'I' and store in register 'T'.
    //          T = S[I]
    //
    // Encoding: 100101 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    LHU,
    //------------------------------------------------------------------------------------------------------------------
    // [LOAD UPPER IMMEDIATE]
    //      Load the constant 'I' into the MOST SIGNIFICANT bits of register 'T' and zero all other bits.
    //          T = I << 16
    //
    // Encoding: 001111 ----- TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    LUI,
    //------------------------------------------------------------------------------------------------------------------
    // [LOAD WORD]
    //      Load the contents of the ALIGNED 32-bit word pointed to by register 'S' plus the 16-bit SIGNED constant
    //      offset 'I' and store in register 'T':
    //          T = S[I]
    //
    // Encoding: 100011 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    LW,
    //------------------------------------------------------------------------------------------------------------------
    // [LOAD WORD TO COPROCESSOR 2]
    //      Load the contents of the ALIGNED 32-bit word pointed to by register 'S' plus the 16-bit SIGNED constant
    //      offset 'I' and store in coprocessor 2 register 'T':
    //          COP2[T] = S[I]
    //
    // Encoding: 110010 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    LWC2,
    //------------------------------------------------------------------------------------------------------------------
    // [LOAD WORD LEFT]
    //      Makeup a POTENTIALLY UNALIGNED memory address 'A' by adding the address in register 'S' plus the 16-bit
    //      SIGNED constant offset 'I'. Then forcefully word align 'A' by truncating 2 bits to form address 'AL'.
    //      Replace the MOST SIGNIFICANT bytes in output register 'T' which are in the word pointed to by 'AL' and
    //      which are <= address 'A'. All other bytes remain unchanged.
    //      
    //      Pseudocode:
    //          A = S + I
    //          AL = A & FFFFFFFC
    //
    //          switch (A % 4)
    //              case 0:     T = (T & 00FFFFFF) | (AL[0] << 24)
    //              case 1:     T = (T & 0000FFFF) | (AL[0] << 16)
    //              case 2:     T = (T & 000000FF) | (AL[0] << 8)
    //              case 3:     T = (T & 00000000) | (AL[0] << 0)
    //
    // Encoding: 100010 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    LWL,
    //------------------------------------------------------------------------------------------------------------------
    // [LOAD WORD RIGHT]
    //      Makeup a POTENTIALLY UNALIGNED memory address 'A' by adding the address in register 'S' plus the 16-bit
    //      SIGNED constant offset 'I'. Then forcefully word align 'A' by truncating 2 bits to form address 'AL'.
    //      Replace the LEAST SIGNIFICANT bytes in output register 'T' which are in the word pointed to by 'AL' and
    //      which are >= address 'A'. All other bytes remain unchanged.
    //
    //      Pseudocode:
    //          A = S + I
    //          AL = A & FFFFFFFC
    //
    //          switch (A % 4)
    //              case 0:     T = (T & 00000000) | (AL[0] >> 0)
    //              case 1:     T = (T & FF000000) | (AL[0] >> 8)
    //              case 2:     T = (T & FFFF0000) | (AL[0] >> 16)
    //              case 3:     T = (T & FFFFFF00) | (AL[0] >> 24)
    //
    // Encoding: 100110 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    LWR,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE FROM COPROCESSOR 0]
    //      Move a value from a coprocessor 0 register 'D' and save the result in register 'T':
    //          T = COP0[D]
    //
    // Encoding: 010000 00000 TTTTT DDDDD ----- ------
    //------------------------------------------------------------------------------------------------------------------
    MFC0,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE FROM COPROCESSOR 2]
    //      Move a value from a coprocessor 2 register 'D' and save the result in register 'T':
    //          T = COP2[D]
    //
    // Encoding: 010010 00000 TTTTT DDDDD ----- ------
    //------------------------------------------------------------------------------------------------------------------
    MFC2,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE FROM HI REGISTER]
    //      Copy the value in the special purpose 'HI' register to the given register 'D':
    //          D = HI
    //
    //      IMPORTANT: there are MANY restrictions to when values can be moved TO and FROM this register and what
    //      causes unpredictable behavior. See the MIPS instruction set reference for more details.
    //
    // Encoding: 000000 ----- ----- DDDDD ----- 010000
    //------------------------------------------------------------------------------------------------------------------
    MFHI,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE FROM LO REGISTER]
    //      Copy the value in the special purpose 'LO' register to the given register 'D':
    //          D = LO
    //
    //      IMPORTANT: there are MANY restrictions to when values can be moved TO and FROM this register and what
    //      causes unpredictable behavior. See the MIPS instruction set reference for more details.
    //
    // Encoding: 000000 ----- ----- DDDDD ----- 010010
    //------------------------------------------------------------------------------------------------------------------
    MFLO,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE TO COPROCESSOR 0]
    //      Move a value to a coprocessor 0 register 'D' from register 'T':
    //          COP0[D] = T
    //
    // Encoding: 010000 00100 TTTTT DDDDD ----- ------
    //------------------------------------------------------------------------------------------------------------------
    MTC0,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE TO COPROCESSOR 2]
    //      Move a value to a coprocessor 2 register 'D' from register 'T':
    //          COP2[D] = T
    //
    // Encoding: 010010 00100 TTTTT DDDDD ----- ------
    //------------------------------------------------------------------------------------------------------------------
    MTC2,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE TO HI REGISTER]
    //      Move a value from register 'S' to the special purpose 'HI' register:
    //          HI = S
    //
    //      IMPORTANT: there are MANY restrictions to when values can be moved TO and FROM this register and what
    //      causes unpredictable behavior. See the MIPS instruction set reference for more details.
    //
    // Encoding: 000000 SSSSS ----- ----- ----- 010001
    //------------------------------------------------------------------------------------------------------------------
    MTHI,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE TO LO REGISTER]
    //      Move a value from register 'S' to the special purpose 'LO' register:
    //          LO = S
    //
    //      IMPORTANT: there are MANY restrictions to when values can be moved TO and FROM this register and what
    //      causes unpredictable behavior. See the MIPS instruction set reference for more details.
    //
    // Encoding: 000000 SSSSS ----- ----- ----- 010011
    //------------------------------------------------------------------------------------------------------------------
    MTLO,
    //------------------------------------------------------------------------------------------------------------------
    // [MULTIPLY WORD]
    //      Multiply SIGNED register 'S' by SIGNED register 'T' and save the results in special registers 'HI' & 'LO'.
    //      The low 32-bits of the result is stored in 'LO' and the high 32-bits stored in 'HI':
    //          HI, LO = S * T
    //
    // Encoding: 000000 SSSSS TTTTT ----- ----- 011000
    //------------------------------------------------------------------------------------------------------------------
    MULT,
    //------------------------------------------------------------------------------------------------------------------
    // [MULTIPLY UNSIGNED WORD]
    //      Multiply UNSIGNED register 'S' by UNSIGNED register 'T' and save the results in special registers 'HI' & 'LO'.
    //      The low 32-bits of the result is stored in 'LO' and the high 32-bits stored in 'HI':
    //          HI, LO = S * T
    //
    // Encoding: 000000 SSSSS TTTTT ----- ----- 011001
    //------------------------------------------------------------------------------------------------------------------
    MULTU,
    //------------------------------------------------------------------------------------------------------------------
    // [NOT OR]
    //      Does a bitwise logical OR of register 'S' with register 'T' and then inverts the bits of the result.
    //      The output is stored in register 'D'. This is equivalent to:
    //          D = ~(S | T)
    //
    // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100111
    //------------------------------------------------------------------------------------------------------------------
    NOR,
    //------------------------------------------------------------------------------------------------------------------
    // [OR]
    //      Do a bitwise logical OR of registers 'S' and 'T' and store in register 'D', I.E:
    //          D = S | T
    //
    // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100101
    //------------------------------------------------------------------------------------------------------------------
    OR,
    //------------------------------------------------------------------------------------------------------------------
    // [OR IMMEDIATE]
    //      Do a bitwise logical OR of register 'S' with 16-bit constant 'I' and store in register 'T', I.E:
    //          T = S | I
    //
    // Encoding: 001101 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    ORI,
    //------------------------------------------------------------------------------------------------------------------
    // [RESTORE FROM EXCEPTION]
    //      Unsure what this does exactly, something to do with interrupt handling and restoring previous interrupts.
    //
    // Encoding: 010000 10000 ----- ----- ----- 010000 
    //------------------------------------------------------------------------------------------------------------------
    RFE,
    //------------------------------------------------------------------------------------------------------------------
    // [STORE BYTE]
    //      Store the LEAST SIGNIFICANT BYTE of register 'T' to the address pointed to by register 'S' plus the 16-bit
    //      SIGNED constant offset 'I':
    //          S[I] = T & 000000FF
    //
    // Encoding: 101000 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    SB,
    //------------------------------------------------------------------------------------------------------------------
    // [STORE HALF WORD]
    //      Store the LEAST SIGNIFICANT HALF WORD of register 'T' to the ALIGNED address pointed to by register 'S' plus
    //      the 16-bit SIGNED constant offset 'I':
    //          S[I] = T & 0000FFFF
    //
    // Encoding: 101001 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    SH,
    //------------------------------------------------------------------------------------------------------------------
    // [SHIFT WORD LEFT LOGICAL]
    //      Do a logical left shift of register 'T' by 5 bit unsigned constant 'I' and store the result in 'D', I.E:
    //          D = T << I
    //
    // Encoding: 000000 ----- TTTTT DDDDD IIIII 000000
    //------------------------------------------------------------------------------------------------------------------
    SLL,
    //------------------------------------------------------------------------------------------------------------------
    // [SHIFT WORD LEFT LOGICAL VARIABLE]
    //      Do a logical left shift of register 'T' by UNSIGNED register 'S' and store in register 'D', I.E:
    //          D = T << S
    //
    // Encoding: 000000 SSSSS TTTTT DDDDD ----- 000100
    //------------------------------------------------------------------------------------------------------------------
    SLLV,
    //------------------------------------------------------------------------------------------------------------------
    // [SET ON LESS THAN]
    //      Compare SIGNED registers 'S' and 'T', checking the condition 'S < T'.
    //      If the condition is 'true' then store '1' in register 'D', otherwise '0'. This is equivalent to:
    //          D = (S < T) ? 1 : 0
    //
    // Encoding: 000000 SSSSS TTTTT DDDDD ----- 101010
    //------------------------------------------------------------------------------------------------------------------
    SLT,
    //------------------------------------------------------------------------------------------------------------------
    // [SET ON LESS THAN IMMEDIATE]
    //      Compare SIGNED register 'S' with SIGNED 16-bit constant 'I', checking the condition 'S < I'.
    //      If the condition is 'true' then store '1' in register 'T', otherwise '0'. This is equivalent to:
    //          T = (S < I) ? 1 : 0
    //
    // Encoding: 001010 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    SLTI,
    //------------------------------------------------------------------------------------------------------------------
    // [SET ON LESS THAN IMMEDIATE UNSIGNED]
    //      Compare UNSIGNED register 'S' with SIGNED 16-bit constant 'I', checking the condition 'S < I'.
    //      Before comparison, the constant 'I' is SIGN EXTENDED to 32-bits and then interpreted as an unsigned.
    //      If the condition is 'true' then store '1' in register 'T', otherwise '0'. This is equivalent to:
    //          T = (S < sign_extend(I)) ? 1 : 0
    //
    // Encoding: 001011 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    SLTIU,
    //------------------------------------------------------------------------------------------------------------------
    // [SET ON LESS THAN UNSIGNED]
    //      Compare UNSIGNED registers 'S' and 'T', checking the condition 'S < T'.
    //      If the condition is 'true' then store '1' in register 'D', otherwise '0'. This is equivalent to:
    //          D = (S < T) ? 1 : 0
    //
    // Encoding: 000000 SSSSS TTTTT DDDDD ----- 101011
    //------------------------------------------------------------------------------------------------------------------
    SLTU,
    //------------------------------------------------------------------------------------------------------------------
    // [SHIFT WORD RIGHT ARITHMETIC]
    //      Do an ARITHMETIC right shift of register 'T' by 5 bit unsigned constant 'I' and store the result in 'D', I.E:
    //          D = T >> I
    //
    // Encoding: 000000 ----- TTTTT DDDDD IIIII 000011
    //------------------------------------------------------------------------------------------------------------------
    SRA,
    //------------------------------------------------------------------------------------------------------------------
    // [SHIFT WORD RIGHT ARITHMETIC VARIABLE]
    //      Do an ARITHMETIC right shift of register 'T' by UNSIGNED register 'S' and store in register 'D', I.E:
    //          D = T >> S
    //
    // Encoding: 000000 SSSSS TTTTT DDDDD ----- 000111
    //------------------------------------------------------------------------------------------------------------------
    SRAV,
    //------------------------------------------------------------------------------------------------------------------
    // [SHIFT WORD RIGHT LOGICAL]
    //      Do a LOGICAL right shift of register 'T' by 5 bit unsigned constant 'I' and store the result in 'D', I.E:
    //          D = T >>> I
    //
    // Encoding: 000000 ----- TTTTT DDDDD IIIII 000010
    //------------------------------------------------------------------------------------------------------------------
    SRL,
    //------------------------------------------------------------------------------------------------------------------
    // [SHIFT WORD RIGHT LOGICAL VARIABLE]
    //      Do a LOGICAL right shift of register 'T' by UNSIGNED register 'S' and store in register 'D', I.E:
    //          D = T >>> S
    //
    // Encoding: 000000 SSSSS TTTTT DDDDD ----- 000110
    //------------------------------------------------------------------------------------------------------------------
    SRLV,
    //------------------------------------------------------------------------------------------------------------------
    // [SUBTRACT WORD]
    //      Do a SIGNED subtraction of register 'T' from register 'S' and store in register 'D', I.E:
    //          D = S - T
    //      If an OVERFLOW occurs then an EXCEPTION will be raised and the destination register will be UNCHANGED.
    //
    // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100010
    //------------------------------------------------------------------------------------------------------------------
    SUB,
    //------------------------------------------------------------------------------------------------------------------
    // [SUBTRACT UNSIGNED WORD]
    //      Do a subtraction of register 'T' from register 'S' and store in register 'D', I.E:
    //          D = S - T
    //      OVERFLOWS are IGNORED by this particular operation.
    //
    // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100011
    //------------------------------------------------------------------------------------------------------------------
    SUBU,
    //------------------------------------------------------------------------------------------------------------------
    // [STORE WORD]
    //      Store the contents of register 'T' to the ALIGNED 32-bit word pointed to by register 'S' plus the 16-bit
    //      SIGNED constant offset 'I':
    //          S[I] = T
    //
    // Encoding: 101011 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    SW,
    //------------------------------------------------------------------------------------------------------------------
    // [STORE WORD FROM COPROCESSOR 2]
    //      Store the contents of coprocessor 2 register 'T' to the ALIGNED 32-bit word pointed to by register 'S' plus
    //      the 16-bit SIGNED constant offset 'I':
    //          S[I] = COP2[T]
    //
    // Encoding: 111010 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    SWC2,
    //------------------------------------------------------------------------------------------------------------------
    // [STORE WORD LEFT]
    //      Makeup a POTENTIALLY UNALIGNED memory address 'A' by adding the address in register 'S' plus the 16-bit
    //      SIGNED constant offset 'I'. Then forcefully word align 'A' by truncating 2 bits to form address 'AL'.
    //      Store a varying number of the MOST SIGNIFICANT bytes in register 'T' to the LEAST SIGNIFICANT bytes of
    //      address 'AL' based on the alignment of address 'A'. Note: all other bytes remain unchanged.
    //
    //      The 4 possible cases are outlined below:
    //          A = S + I
    //          AL = A & FFFFFFFC
    //
    //          switch (A % 4)
    //              case 0:     AL[0] = (AL[0] & FFFFFF00) | (T >> 24)
    //              case 1:     AL[0] = (AL[0] & FFFF0000) | (T >> 16)
    //              case 2:     AL[0] = (AL[0] & FF000000) | (T >> 8)
    //              case 3:     AL[0] = (AL[0] & 00000000) | (T >> 0)
    //
    // Encoding: 101010 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    SWL,
    //------------------------------------------------------------------------------------------------------------------
    // [STORE WORD RIGHT]
    //      Makeup a POTENTIALLY UNALIGNED memory address 'A' by adding the address in register 'S' plus the 16-bit
    //      SIGNED constant offset 'I'. Then forcefully word align 'A' by truncating 2 bits to form address 'AL'.
    //      Store a varying number of the LEAST SIGNIFICANT bytes in register 'T' to the MOST SIGNIFICANT bytes of
    //      address 'AL' based on the alignment of address 'A'. Note: all other bytes remain unchanged.
    //
    //      The 4 possible cases are outlined below:
    //          A = S + I
    //          AL = A & FFFFFFFC
    //
    //          switch (A % 4)
    //              case 0:     AL[0] = (AL[0] & 00000000) | (T << 0)
    //              case 1:     AL[0] = (AL[0] & 000000FF) | (T << 8)
    //              case 2:     AL[0] = (AL[0] & 0000FFFF) | (T << 16)
    //              case 3:     AL[0] = (AL[0] & 00FFFFFF) | (T << 24)
    //
    // Encoding: 101110 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    SWR,
    //------------------------------------------------------------------------------------------------------------------
    // [SYSTEM CALL]
    //      Causes a system call exception and transfers control to the exception handler.
    //      Code 'I' is available as a parameter for the exception handler.
    //
    // Encoding: 000000 IIIII IIIII IIIII IIIII 001100
    //------------------------------------------------------------------------------------------------------------------
    SYSCALL,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF EQUAL]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          SIGNED registers 'S' == SIGNED register 'T' and uses code 'I' as an execption param.
    //
    // Encoding: 000000 SSSSS TTTTT IIIII IIIII 110100
    //------------------------------------------------------------------------------------------------------------------
    TEQ,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF EQUAL IMMEDIATE]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          SIGNED register 'S' == SIGNED constant 'I'
    //
    // Encoding: 000001 SSSSS 01100 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    TEQI,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF GREATER OR EQUAL]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          SIGNED register 'S' >= SIGNED register 'T' and uses code 'I' as an execption param.
    //
    // Encoding: 000000 SSSSS TTTTT IIIII IIIII 110000
    //------------------------------------------------------------------------------------------------------------------
    TGE,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF GREATER OR EQUAL IMMEDIATE]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          SIGNED Register 'S' >= SIGNED constant 'I'
    //
    // Encoding: 000001 SSSSS 01000 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    TGEI,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF GREATER OR EQUAL IMMEDIATE UNSIGNED]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          UNSIGNED Register 'S' >= UNSIGNED, SIGN EXTENDED constant 'I'
    //
    // Encoding: 000001 SSSSS 01001 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    TGEIU,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF GREATER OR EQUAL UNSIGNED]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          UNSIGNED register 'S' >= UNSIGNED register 'T' and uses code 'I' as an execption param.
    //
    // Encoding: 000000 SSSSS TTTTT IIIII IIIII 110001
    //------------------------------------------------------------------------------------------------------------------
    TGEU,
    //------------------------------------------------------------------------------------------------------------------
    // [PROBE TLB FOR MATCHING ENTRY]
    //      System instruction. See MIPS instruction reference for more details (shouldn't matter much for DOOM).
    //
    // Encoding: 010000 10000 ----- ----- ----- 001000
    //------------------------------------------------------------------------------------------------------------------
    TLBP,
    //------------------------------------------------------------------------------------------------------------------
    // [READ INDEXED TLB ENTRY]
    //      System instruction. See MIPS instruction reference for more details (shouldn't matter much for DOOM).
    //
    // Encoding: 010000 10000 ----- ----- ----- 000001
    //------------------------------------------------------------------------------------------------------------------
    TLBR,
    //------------------------------------------------------------------------------------------------------------------
    // [WRITE INDEXED TLB ENTRY]
    //      System instruction. See MIPS instruction reference for more details (shouldn't matter much for DOOM).
    //
    // Encoding: 010000 10000 ----- ----- ----- 000010
    //------------------------------------------------------------------------------------------------------------------
    TLBWI,
    //------------------------------------------------------------------------------------------------------------------
    // [WRITE RANDOM TLB ENTRY]
    //      System instruction. See MIPS instruction reference for more details (shouldn't matter much for DOOM).
    //
    // Encoding: 010000 10000 ----- ----- ----- 000110
    //------------------------------------------------------------------------------------------------------------------
    TLBWR,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF LESS THAN]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          SIGNED register 'S' < SIGNED register 'T' and uses code 'I' as an execption param.
    //
    // Encoding: 000000 SSSSS TTTTT IIIII IIIII 110010
    //------------------------------------------------------------------------------------------------------------------
    TLT,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF LESS THAN IMMEDIATE]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          SIGNED Register 'S' < SIGNED constant 'I'
    //
    // Encoding: 000001 SSSSS 01010 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    TLTI,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF LESS THAN IMMEDIATE UNSIGNED]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          UNSIGNED Register 'S' < UNSIGNED, SIGN EXTENDED constant 'I'
    //
    // Encoding: 000001 SSSSS 01011 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    TLTIU,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF LESS THAN UNSIGNED]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          UNSIGNED register 'S' < UNSIGNED register 'T' and uses code 'I' as an execption param.
    //
    // Encoding: 000000 SSSSS TTTTT IIIII IIIII 110011
    //------------------------------------------------------------------------------------------------------------------
    TLTU,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF NOT EQUAL]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          Register 'S' != register 'T' and uses code 'I' as an execption param.
    //
    // Encoding: 000000 SSSSS TTTTT IIIII IIIII 110110
    //------------------------------------------------------------------------------------------------------------------
    TNE,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF NOT EQUAL IMMEDIATE]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          SIGNED Register 'S' != SIGNED constant 'I'
    //
    // Encoding: 000001 SSSSS 01110 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    TNEI,
    //------------------------------------------------------------------------------------------------------------------
    // [EXCLUSIVE OR]
    //      Do a bitwise logical exclusive OR of registers 'S' and 'T' and store in register 'D', I.E:
    //          D = S ^ T
    //
    // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100110
    //------------------------------------------------------------------------------------------------------------------
    XOR,
    //------------------------------------------------------------------------------------------------------------------
    // [EXCLUSIVE OR IMMEDIATE]
    //      Do a bitwise logical exclusive OR of register 'S' with 16-bit constant 'I' and store in register 'T', I.E:
    //          T = S ^ I
    //
    // Encoding: 001110 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    XORI,
    //------------------------------------------------------------------------------------------------------------------
    // Used to represent an invalid opcode.
    // Anything at this opcode index or greater is INVALID.
    //------------------------------------------------------------------------------------------------------------------
    INVALID
};

static constexpr const uint32_t NUM_CPU_OPCODES = (uint32_t) CpuOpcode::INVALID;

namespace CpuOpcodeUtils {
    //------------------------------------------------------------------------------------------------------------------
    // Get the human readable name for an opcode
    //------------------------------------------------------------------------------------------------------------------
    inline constexpr const char* getMnemonic(const CpuOpcode opcode) noexcept {
        switch (opcode) {
            case CpuOpcode::ADD:        return "add";
            case CpuOpcode::ADDI:       return "addi";
            case CpuOpcode::ADDIU:      return "addiu";
            case CpuOpcode::ADDU:       return "addu";
            case CpuOpcode::AND:        return "and";
            case CpuOpcode::ANDI:       return "andi";
            case CpuOpcode::BEQ:        return "beq";
            case CpuOpcode::BGEZ:       return "bgez";
            case CpuOpcode::BGEZAL:     return "bgezal";
            case CpuOpcode::BGTZ:       return "bgtz";
            case CpuOpcode::BLEZ:       return "blez";
            case CpuOpcode::BLTZ:       return "bltz";
            case CpuOpcode::BLTZAL:     return "bltzal";
            case CpuOpcode::BNE:        return "bne";
            case CpuOpcode::BREAK:      return "break";
            case CpuOpcode::CFC2:       return "cfc2";
            case CpuOpcode::COP2:       return "cop2";
            case CpuOpcode::CTC2:       return "ctc2";
            case CpuOpcode::DIV:        return "div";
            case CpuOpcode::DIVU:       return "divu";
            case CpuOpcode::J:          return "j";
            case CpuOpcode::JAL:        return "jal";
            case CpuOpcode::JALR:       return "jalr";
            case CpuOpcode::JR:         return "jr";
            case CpuOpcode::LB:         return "lb";
            case CpuOpcode::LBU:        return "lbu";
            case CpuOpcode::LH:         return "lh";
            case CpuOpcode::LHU:        return "lhu";
            case CpuOpcode::LUI:        return "lui";
            case CpuOpcode::LW:         return "lw";
            case CpuOpcode::LWC2:       return "lwc2";
            case CpuOpcode::LWL:        return "lwl";
            case CpuOpcode::LWR:        return "lwr";
            case CpuOpcode::MFC0:       return "mfc0";
            case CpuOpcode::MFC2:       return "mfc2";
            case CpuOpcode::MFHI:       return "mfhi";
            case CpuOpcode::MFLO:       return "mflo";
            case CpuOpcode::MTC0:       return "mtc0";
            case CpuOpcode::MTC2:       return "mtc2";
            case CpuOpcode::MTHI:       return "mthi";
            case CpuOpcode::MTLO:       return "mtlo";
            case CpuOpcode::MULT:       return "mult";
            case CpuOpcode::MULTU:      return "multu";
            case CpuOpcode::NOR:        return "nor";
            case CpuOpcode::OR:         return "or";
            case CpuOpcode::ORI:        return "ori";
            case CpuOpcode::RFE:        return "rfe";
            case CpuOpcode::SB:         return "sb";
            case CpuOpcode::SH:         return "sh";
            case CpuOpcode::SLL:        return "sll";
            case CpuOpcode::SLLV:       return "sllv";
            case CpuOpcode::SLT:        return "slt";
            case CpuOpcode::SLTI:       return "slti";
            case CpuOpcode::SLTIU:      return "sltiu";
            case CpuOpcode::SLTU:       return "sltu";
            case CpuOpcode::SRA:        return "sra";
            case CpuOpcode::SRAV:       return "srav";
            case CpuOpcode::SRL:        return "srl";
            case CpuOpcode::SRLV:       return "srlv";
            case CpuOpcode::SUB:        return "sub";
            case CpuOpcode::SUBU:       return "subu";
            case CpuOpcode::SW:         return "sw";
            case CpuOpcode::SWC2:       return "swc2";
            case CpuOpcode::SWL:        return "swl";
            case CpuOpcode::SWR:        return "swr";
            case CpuOpcode::SYSCALL:    return "syscall";
            case CpuOpcode::TEQ:        return "teq";
            case CpuOpcode::TEQI:       return "teqi";
            case CpuOpcode::TGE:        return "tge";
            case CpuOpcode::TGEI:       return "tgei";
            case CpuOpcode::TGEIU:      return "tgeiu";
            case CpuOpcode::TGEU:       return "tgeu";
            case CpuOpcode::TLBP:       return "tlbp";
            case CpuOpcode::TLBR:       return "tlbr";
            case CpuOpcode::TLBWI:      return "tlbwi";
            case CpuOpcode::TLBWR:      return "tlbwr";
            case CpuOpcode::TLT:        return "tlt";
            case CpuOpcode::TLTI:       return "tlti";
            case CpuOpcode::TLTIU:      return "tltiu";
            case CpuOpcode::TLTU:       return "tltu";
            case CpuOpcode::TNE:        return "tne";
            case CpuOpcode::TNEI:       return "tnei";
            case CpuOpcode::XOR:        return "xor";
            case CpuOpcode::XORI:       return "xori";
            case CpuOpcode::INVALID:
                break;
        }

        return "<INVALID OPCODE>";
    }

    inline constexpr bool isBranchOpcode(const CpuOpcode opcode) noexcept {
        switch (opcode) {
            case CpuOpcode::BEQ:
            case CpuOpcode::BGEZ:
            case CpuOpcode::BGEZAL:
            case CpuOpcode::BGTZ:
            case CpuOpcode::BLEZ:
            case CpuOpcode::BLTZ:
            case CpuOpcode::BLTZAL:
            case CpuOpcode::BNE:
                return true;

            default:
                return false;
        }
    }

    inline constexpr bool isJumpOpcode(const CpuOpcode opcode) noexcept {
        switch (opcode) {
            case CpuOpcode::J:
            case CpuOpcode::JAL:
            case CpuOpcode::JALR:
            case CpuOpcode::JR:
                return true;
            
            default:
                return false;
        }
    }

    inline constexpr bool isBranchOrJumpOpcode(const CpuOpcode opcode) noexcept {
        return (isBranchOpcode(opcode) || isJumpOpcode(opcode));
    }

    inline constexpr bool isFixedJumpOpcode(const CpuOpcode opcode) noexcept {
        switch (opcode) {
            case CpuOpcode::J:
            case CpuOpcode::JAL:
                return true;
            
            default:
                return false;
        }
    }

    inline constexpr bool isReturningJumpOpcode(const CpuOpcode opcode) noexcept {
        switch (opcode) {
            case CpuOpcode::JAL:
            case CpuOpcode::JALR:
                return true;
                
            default:
                return false;
        }
    }

    inline constexpr bool isTrapOpcode(const CpuOpcode opcode) noexcept {
        switch (opcode) {
            case CpuOpcode::TEQ:
            case CpuOpcode::TEQI:
            case CpuOpcode::TGE:
            case CpuOpcode::TGEI:
            case CpuOpcode::TGEIU:
            case CpuOpcode::TGEU:
            case CpuOpcode::TLT:
            case CpuOpcode::TLTI:
            case CpuOpcode::TLTIU:
            case CpuOpcode::TLTU:
            case CpuOpcode::TNE:
            case CpuOpcode::TNEI:
                return true;
            
            default:
                return false;
        }
    }

    // Tells if the CPU operation is one with a load delay slot
    inline constexpr bool isLoadDelaySlotOpcode(const CpuOpcode opcode) noexcept {
        switch (opcode) {
            case CpuOpcode::LB:
            case CpuOpcode::LBU:
            case CpuOpcode::LH:
            case CpuOpcode::LHU:
            case CpuOpcode::LW:
            case CpuOpcode::LWC2:
            case CpuOpcode::LWL:
            case CpuOpcode::LWR:
                return true;
        }

        return false;
    }

    inline constexpr bool isIllegalOpcode(const CpuOpcode opcode) noexcept {
        return ((uint32_t) opcode >= NUM_CPU_OPCODES);
    }
}

#pragma once

#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Shorthand type aliases
//------------------------------------------------------------------------------------------------------------------------------------------
typedef uint32_t    u32;
typedef uint16_t    u16;
typedef uint8_t     u8;
typedef int32_t     i32;
typedef int16_t     i16;
typedef int8_t      i8;

//------------------------------------------------------------------------------------------------------------------------------------------
// Mips R3000 registers and macros aliasing them.
// Note: not allowing modification of the 'zero' register.
//------------------------------------------------------------------------------------------------------------------------------------------
const uint32_t* gpReg_zero;
uint32_t* gpReg_at;
uint32_t* gpReg_v0;
uint32_t* gpReg_v1;
uint32_t* gpReg_a0;
uint32_t* gpReg_a1;
uint32_t* gpReg_a2;
uint32_t* gpReg_a3;
uint32_t* gpReg_t0;
uint32_t* gpReg_t1;
uint32_t* gpReg_t2;
uint32_t* gpReg_t3;
uint32_t* gpReg_t4;
uint32_t* gpReg_t5;
uint32_t* gpReg_t6;
uint32_t* gpReg_t7;
uint32_t* gpReg_s0;
uint32_t* gpReg_s1;
uint32_t* gpReg_s2;
uint32_t* gpReg_s3;
uint32_t* gpReg_s4;
uint32_t* gpReg_s5;
uint32_t* gpReg_s6;
uint32_t* gpReg_s7;
uint32_t* gpReg_t8;
uint32_t* gpReg_t9;
uint32_t* gpReg_k0;
uint32_t* gpReg_k1;
uint32_t* gpReg_gp;
uint32_t* gpReg_sp;
uint32_t* gpReg_fp;
uint32_t* gpReg_ra;
uint32_t* gpReg_hi;
uint32_t* gpReg_lo;

#define zero    (*gpReg_zero)
#define at      (*gpReg_at)
#define v0      (*gpReg_v0)
#define v1      (*gpReg_v1)
#define a0      (*gpReg_a0)
#define a1      (*gpReg_a1)
#define a2      (*gpReg_a2)
#define a3      (*gpReg_a3)
#define t0      (*gpReg_t0)
#define t1      (*gpReg_t1)
#define t2      (*gpReg_t2)
#define t3      (*gpReg_t3)
#define t4      (*gpReg_t4)
#define t5      (*gpReg_t5)
#define t6      (*gpReg_t6)
#define t7      (*gpReg_t7)
#define s0      (*gpReg_s0)
#define s1      (*gpReg_s1)
#define s2      (*gpReg_s2)
#define s3      (*gpReg_s3)
#define s4      (*gpReg_s4)
#define s5      (*gpReg_s5)
#define s6      (*gpReg_s6)
#define s7      (*gpReg_s7)
#define t8      (*gpReg_t8)
#define t9      (*gpReg_t9)
#define k0      (*gpReg_k0)
#define k1      (*gpReg_k1)
#define gp      (*gpReg_gp)
#define sp      (*gpReg_sp)
#define fp      (*gpReg_fp)
#define ra      (*gpReg_ra)
#define hi      (*gpReg_hi)
#define lo      (*gpReg_lo)

//------------------------------------------------------------------------------------------------------------------------------------------
// Mips instructions that must be emulated.
// These instructions were not so easy to convert directly to C++ so they are handled via functions.
//------------------------------------------------------------------------------------------------------------------------------------------

// Trap instructions
void teq(const uint32_t r1, const uint32_t r2, const uint16_t i) noexcept;
void teqi(const uint32_t r1, const int32_t i) noexcept;
void tge(const uint32_t r1, const uint32_t r2, const uint16_t i) noexcept;
void tgei(const uint32_t r1, const int32_t i) noexcept;
void tgeiu(const uint32_t r1, const uint32_t i) noexcept;
void tgeu(const uint32_t r1, const uint32_t r2, const uint16_t i) noexcept;
void tlt(const uint32_t r1, const uint32_t r2, const uint16_t i) noexcept;
void tlti(const uint32_t r1, const int32_t i) noexcept;
void tltiu(const uint32_t r1, const uint32_t i) noexcept;
void tltu(const uint32_t r1, const uint32_t r2, const uint16_t i) noexcept;
void tne(const uint32_t r1, const uint32_t r2, const uint16_t i) noexcept;
void tnei(const uint32_t r1, const int32_t i) noexcept;

// Arithmetic
uint32_t add(const uint32_t r1, const uint32_t r2) noexcept;
uint32_t addi(const uint32_t r1, const int16_t i) noexcept;
void div(const uint32_t r1, const uint32_t r2) noexcept;
void divu(const uint32_t r1, const uint32_t r2) noexcept;
void mult(const uint32_t r1, const uint32_t r2) noexcept;
void multu(const uint32_t r1, const uint32_t r2) noexcept;
uint32_t sub(const uint32_t r1, const uint32_t r2) noexcept;

// RAM to CPU loads
uint32_t lb(const uint32_t addr) noexcept;
uint32_t lbu(const uint32_t addr) noexcept;
uint32_t lh(const uint32_t addr) noexcept;
uint32_t lhu(const uint32_t addr) noexcept;
uint32_t lw(const uint32_t addr) noexcept;
uint32_t lwl(const uint32_t r1, const uint32_t addr) noexcept;
uint32_t lwr(const uint32_t r1, const uint32_t addr) noexcept;

// CPU to RAM stores
void sb(const uint32_t r1, const uint32_t addr) noexcept;
void sh(const uint32_t r1, const uint32_t addr) noexcept;
void sw(const uint32_t r1, const uint32_t addr) noexcept;
void swl(const uint32_t r1, const uint32_t addr) noexcept;
void swr(const uint32_t r1, const uint32_t addr) noexcept;

// Coprocessor 0 instructions
uint32_t mfc0(const uint8_t s) noexcept;
void mtc0(const uint32_t r1, const uint8_t d) noexcept;

// Coprocessor 2 (GTE) instructions
void cop2(const uint32_t i) noexcept;
uint32_t cfc2(const uint8_t s) noexcept;
void ctc2(const uint32_t r1, const uint8_t d) noexcept;
uint32_t mfc2(const uint8_t s) noexcept;
void mtc2(const uint32_t r1, const uint8_t d) noexcept;
void lwc2(const uint8_t d, const uint32_t addr) noexcept;
void swc2(const uint8_t s, const uint32_t addr) noexcept;

// Misc
void _break(const uint32_t i) noexcept;
void syscall(const uint32_t i) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Utilities
//------------------------------------------------------------------------------------------------------------------------------------------

// Call a function at the given address.
// The function passes args and return values through the VM interface.
void pcall(const uint32_t addr) noexcept;

// Call a bios function.
// This will transfer control over to the emulator.
void bios_call(const uint32_t func) noexcept;

// Called when the code is trying to jump to an unexpected location for a jump table.
// If this happens then something has seriously gone wrong.
void jump_table_err() noexcept;

#include <iostream>
#include <gtest/gtest.h>
#include "../../lib/src/M6502Lib.h"

class MOS6502Test1 : public testing::Test {

public:
    Mem mem;
    CPU cpu;

    virtual void SetUp()
    {
        cpu.Reset(mem);
    }

    virtual void TearDown() {}
};

static void VerifyUnmodifiedFlags(CPU& cpu, CPU& CPUCopy)
{
    EXPECT_EQ(cpu.C, CPUCopy.C);
    EXPECT_EQ(cpu.I, CPUCopy.I);
    EXPECT_EQ(cpu.D, CPUCopy.D);
    EXPECT_EQ(cpu.B, CPUCopy.B);
    EXPECT_EQ(cpu.V, CPUCopy.V);
}

static void NegativeFlagIsChanged(CPU& cpu)
{
    if ((cpu.A & 0b10000000) > 0)
        EXPECT_TRUE(cpu.N);
    else
        EXPECT_FALSE(cpu.N);
};

/* Registers tests */
TEST_F(MOS6502Test1, LDAImmediateCanLoadAValueIntoTheRegister)
{
    // Start - Inline a little program
    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0xFFFD] = 0x42;
    // End - Inline a little program

    CPU CPUCopy = cpu; // Copy for test flags
    u32 Cycles = cpu.Execute(2, mem);
    EXPECT_EQ(Cycles, 2);

    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_FALSE(cpu.Z);

    NegativeFlagIsChanged(cpu);
    VerifyUnmodifiedFlags(cpu, CPUCopy);
};

TEST_F(MOS6502Test1, LDAImmediateCanAffectTheZeroFlag)
{
    // Start - Inline a little program
    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0xFFFD] = 0x0;
    // End - Inline a little program

    CPU CPUCopy = cpu; // Copy for test flags
    cpu.Execute(2, mem);

    EXPECT_EQ(cpu.A, 0x0);

    EXPECT_TRUE(cpu.Z);
    NegativeFlagIsChanged(cpu);
    VerifyUnmodifiedFlags(cpu, CPUCopy);
};

TEST_F(MOS6502Test1, LDAZeroPageCanLoadAValueIntoTheRegister)
{
    // Start - Inline a little program
    mem[0xFFFC] = CPU::INS_LDA_ZP;
    mem[0xFFFD] = 0x40;
    mem[0x0040] = 0x38;
    // End - Inline a little program

    CPU CPUCopy = cpu; // Copy for test flags
    u32 Cycles = cpu.Execute(3, mem);
    EXPECT_EQ(Cycles, 3);

    EXPECT_EQ(cpu.A, 0x38);
    EXPECT_FALSE(cpu.Z);
    NegativeFlagIsChanged(cpu);
    VerifyUnmodifiedFlags(cpu, CPUCopy);
};

TEST_F(MOS6502Test1, LDAZeroPageXCanLoadAValueIntoTheRegister)
{

    // Set X
    cpu.X = 0xF6;

    // Start - Inline a little program
    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x40;

    Byte hexSum = (0x40 + cpu.X);
    mem[hexSum] = 0x38;
    // End - Inline a little program

    CPU CPUCopy = cpu; // Copy for test flags
    u32 Cycles = cpu.Execute(4, mem);
    EXPECT_EQ(Cycles, 4);

    EXPECT_EQ(cpu.A, 0x38);
    EXPECT_FALSE(cpu.Z);

    NegativeFlagIsChanged(cpu);
    VerifyUnmodifiedFlags(cpu, CPUCopy);
};

TEST_F(MOS6502Test1, LDAZeroPageXCanOverload)
{

    // Set X
    cpu.X = 0xFF;

    // Start - Inline a little program
    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x80;

    Byte hexSum = (0x80 + cpu.X);
    mem[hexSum] = 0x38;
    // End - Inline a little program

    CPU CPUCopy = cpu; // Copy for test flags
    u32 Cycles = cpu.Execute(4, mem);
    EXPECT_EQ(Cycles, 4);

    EXPECT_EQ(hexSum, 0x7F);
    EXPECT_FALSE(cpu.Z);
    NegativeFlagIsChanged(cpu);
    VerifyUnmodifiedFlags(cpu, CPUCopy);
};

TEST_F(MOS6502Test1, LDAAbsoluteCanLoadAValueIntoTheRegister)
{

    // Start - Inline a little program
    mem[0xFFFC] = CPU::INS_LDA_ABS;
    mem[0xFFFD] = 0x80;
    mem[0xFFFE] = 0x44;
    mem[0x4480] = 0x37;
    // End - Inline a little program

    CPU CPUCopy = cpu; // Copy for test flags
    u32 Cycles = cpu.Execute(4, mem);

    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(Cycles, 4);
    EXPECT_FALSE(cpu.Z);
    NegativeFlagIsChanged(cpu);
    VerifyUnmodifiedFlags(cpu, CPUCopy);
};

TEST_F(MOS6502Test1, LDAAbsoluteXCanLoadAValueIntoTheRegister)
{

    cpu.X = 1;
    // Start - Inline a little program
    mem[0xFFFC] = CPU::INS_LDA_ABSX;
    mem[0xFFFD] = 0x80;
    mem[0xFFFE] = 0x44;

    Word sum = 0x4480 + cpu.X;
    mem[sum] = 0x40;
    // End - Inline a little program

    CPU CPUCopy = cpu; // Copy for test flags
    u32 Cycles = cpu.Execute(4, mem);
    EXPECT_EQ(Cycles, 4);
    EXPECT_EQ(cpu.A, 0x40);
    EXPECT_FALSE(cpu.Z);
    NegativeFlagIsChanged(cpu);
    VerifyUnmodifiedFlags(cpu, CPUCopy);
};

TEST_F(MOS6502Test1, LDAAbsoluteXCanLoadAValueIntoTheRegisterWhenItCrossesAPageBoundary)
{

    cpu.X = 0xFF; // Limit of a page
    // Start - Inline a little program
    mem[0xFFFC] = CPU::INS_LDA_ABSX;
    mem[0xFFFD] = 0x01;
    mem[0xFFFE] = 0x44;
    Word sum = 0x4401 + cpu.X; // Crosses a page
    mem[sum] = 0x40;
    // End - Inline a little program

    CPU CPUCopy = cpu; // Copy for test flags
    u32 Cycles = cpu.Execute(5, mem);
    EXPECT_EQ(Cycles, 5);
    EXPECT_EQ(cpu.A, 0x40);
    EXPECT_FALSE(cpu.Z);
    NegativeFlagIsChanged(cpu);
    VerifyUnmodifiedFlags(cpu, CPUCopy);
};

TEST_F(MOS6502Test1, LDAAbsoluteYCanLoadAValueIntoTheRegister)
{

    cpu.Y = 0x20;
    // Start - Inline a little program
    mem[0xFFFC] = CPU::INS_LDA_ABSY;
    mem[0xFFFD] = 0x80;
    mem[0xFFFE] = 0x44;
    Word sum = 0x4480 + cpu.Y;
    mem[sum] = 0x40;
    // End - Inline a little program

    CPU CPUCopy = cpu; // Copy for test flags
    u32 Cycles = cpu.Execute(4, mem);
    EXPECT_EQ(Cycles, 4);
    EXPECT_EQ(cpu.A, 0x40);
    EXPECT_FALSE(cpu.Z);
    NegativeFlagIsChanged(cpu);
    VerifyUnmodifiedFlags(cpu, CPUCopy);
};

TEST_F(MOS6502Test1, LDAAbsoluteYCanLoadAValueIntoTheRegisterWhenItCrossesAPageBoundary)
{

    cpu.Y = 0xFF; // Limit of a page
    // Start - Inline a little program
    mem[0xFFFC] = CPU::INS_LDA_ABSY;
    mem[0xFFFD] = 0x01;
    mem[0xFFFE] = 0x44;
    Word sum = 0x4401 + cpu.Y; // Crosses a page
    mem[sum] = 0x40;
    // End - Inline a little program

    CPU CPUCopy = cpu; // Copy for test flags
    u32 Cycles = cpu.Execute(5, mem);
    EXPECT_EQ(Cycles, 5);
    EXPECT_EQ(cpu.A, 0x40);
    EXPECT_FALSE(cpu.Z);
    NegativeFlagIsChanged(cpu);
    VerifyUnmodifiedFlags(cpu, CPUCopy);
};

TEST_F(MOS6502Test1, LDAIndirectXCanLoadAValueIntoTheRegister)
{

    cpu.X = 0x04;
    // Start - Inline a little program
    mem[0xFFFC] = CPU::INS_LDA_INDX;
    mem[0xFFFD] = 0x02;

    Byte sum = mem[0xFFFD] + cpu.X;

    mem[sum] = 0x00;
    mem[0x0007] = 0x80;
    mem[0x8000] = 0x37;

    // End - Inline a little program

    CPU CPUCopy = cpu; // Copy for test flags
    u32 Cycles = cpu.Execute(6, mem);
    EXPECT_EQ(Cycles, 6);
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_FALSE(cpu.Z);
    NegativeFlagIsChanged(cpu);
    VerifyUnmodifiedFlags(cpu, CPUCopy);
};

TEST_F(MOS6502Test1, LDAIndirectYCanLoadAValueIntoTheRegister)
{

    cpu.Y = 0x04; // Limit of a page
    // Start - Inline a little program
    mem[0xFFFC] = CPU::INS_LDA_INDY;
    mem[0xFFFD] = 0x02;
    mem[0x0002] = 0x02;
    mem[0x0003] = 0x80;

    Word sum = 0x8002 + cpu.Y; // (mem[0x0002]+mem[0x0003]) + cpu.Y;
    mem[sum] = 0x50;
    // End - Inline a little program

    CPU CPUCopy = cpu; // Copy for test flags
    u32 Cycles = cpu.Execute(5, mem);
    EXPECT_EQ(Cycles, 5);
    EXPECT_EQ(cpu.A, 0x50);
    EXPECT_FALSE(cpu.Z);
    NegativeFlagIsChanged(cpu);
    VerifyUnmodifiedFlags(cpu, CPUCopy);
};

TEST_F(MOS6502Test1, LDAIndirectYCanLoadAValueIntoTheRegisterWhenItCrossesAPageBoundary)
{

    cpu.Y = 0xFF; // Limit of a page
    // Start - Inline a little program
    mem[0xFFFC] = CPU::INS_LDA_INDY;
    mem[0xFFFD] = 0x01;
    mem[0x0001] = 0x02;
    mem[0x0002] = 0x80;

    Word sum = 0x8002 + cpu.Y; // (mem[0x0002]+mem[0x0003]) + cpu.Y;
    mem[sum] = 0x50;
    // End - Inline a little program

    CPU CPUCopy = cpu; // Copy for test flags
    u32 Cycles = cpu.Execute(6, mem);
    EXPECT_EQ(Cycles, 6);
    EXPECT_EQ(cpu.A, 0x50);
    EXPECT_FALSE(cpu.Z);
    NegativeFlagIsChanged(cpu);
    VerifyUnmodifiedFlags(cpu, CPUCopy);
};

/* CPU tests */
TEST_F(MOS6502Test1, CPUDoesNothingWhenPassed0Cycles)
{

    u32 Cycles = cpu.Execute(1, mem);
    EXPECT_EQ(Cycles, 1);
};

TEST_F(MOS6502Test1, ExecutingABadInstructionDoesNotPutUsInAnInfiniteLoop)
{

    mem[0xFFFC] = 0x0; // Invalid instruction
    mem[0xFFFD] = 0x0;

    u32 Cycles = cpu.Execute(1, mem);
    EXPECT_EQ(Cycles, 1);
}

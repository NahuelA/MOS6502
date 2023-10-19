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


    if ((cpu.A & 0b10000000) > 0)
        EXPECT_TRUE(cpu.N);
    else
        EXPECT_FALSE(cpu.N);

    EXPECT_EQ(cpu.C, CPUCopy.C);
    EXPECT_EQ(cpu.I, CPUCopy.I);
    EXPECT_EQ(cpu.D, CPUCopy.D);
    EXPECT_EQ(cpu.B, CPUCopy.B);
    EXPECT_EQ(cpu.V, CPUCopy.V);
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
    EXPECT_FALSE(cpu.N);
    EXPECT_EQ(cpu.C, CPUCopy.C);
    EXPECT_EQ(cpu.I, CPUCopy.I);
    EXPECT_EQ(cpu.D, CPUCopy.D);
    EXPECT_EQ(cpu.B, CPUCopy.B);
    EXPECT_EQ(cpu.V, CPUCopy.V);
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
    EXPECT_FALSE(cpu.N);
    EXPECT_EQ(cpu.C, CPUCopy.C);
    EXPECT_EQ(cpu.I, CPUCopy.I);
    EXPECT_EQ(cpu.D, CPUCopy.D);
    EXPECT_EQ(cpu.B, CPUCopy.B);
    EXPECT_EQ(cpu.V, CPUCopy.V);
}

TEST_F(MOS6502Test1, LDAZeroPageXCanLoadAValueIntoTheRegisterWhenWraps)
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
    EXPECT_FALSE(cpu.N);
    EXPECT_EQ(cpu.C, CPUCopy.C);
    EXPECT_EQ(cpu.I, CPUCopy.I);
    EXPECT_EQ(cpu.D, CPUCopy.D);
    EXPECT_EQ(cpu.B, CPUCopy.B);
    EXPECT_EQ(cpu.V, CPUCopy.V);
}

#if 0
int main(int argc, char** argv) {

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif
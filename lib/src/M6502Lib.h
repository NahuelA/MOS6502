#include <stdio.h>
#include <stdlib.h>

using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;

struct Mem
{
    static constexpr u32 MAX_MEM = 1024 * 64;
    Byte Data[MAX_MEM];

    void Initialize()
    {
        for (u32 i{ 0 }; i < MAX_MEM; i++)
            Data[i] = 0;
    };

    // Read 1 byte
    Byte operator[](u32 Address) const
    {
        // Assert here address is < MAX_MEM
        return Data[Address];
    }

    // Write 1 byte
    Byte& operator[](u32 Address)
    {
        // Assert here address is < MAX_MEM
        return Data[Address];
    }

    // Write 2 bytes
    void WriteWord(Word Value, u32 Address, u32& Cycles)
    {
        Data[Address] = Value & 0xFF;
        Data[Address + 1] = (Value >> 8);
        Cycles -= 2;
    }
};

struct CPU
{

    Word PC; // Program counter
    Byte SP; // Stack pointer

    Byte A, X, Y; // Registers (Accumulator, Index X and Index Y)

    // Flags
    Byte C; // Carry
    Byte Z; // Zero
    Byte I; // Interrupt disable
    Byte D; // Decimal mode
    Byte B; // Break
    Byte V; // Overflow set
    Byte N; // Negative sign

    void Reset(Mem& memory)
    {
        PC = 0xFFFC; // Start address of program counter
        SP = 0x0100; // Start address of Stack pointer
        C = Z = I = D = B = V = N = 0;
        A = X = Y = 0;
        memory.Initialize();
    }

    Byte FetchByte(u32& Cycles, Mem& memory)
    {
        Byte Data = memory[PC];

        PC++;
        Cycles--;

        return Data;
    }

    Word FetchWord(u32& Cycles, Mem& memory)
    {
        // 6502 is little endian
        Word Data = memory[PC];
        PC++;

        Data |= (memory[PC] << 8);
        PC++;

        Cycles -= 2;

        // If you wanted to handle endianness
        // you would have to swap bytes here
        // if(PLATFORM_BIG_ENDIANN)
        // SwapBytesInWord(Data)

        return Data;
    }

    Byte ReadByte(
        u32& Cycles,
        Byte Address,
        Mem& memory)
    {
        Byte Data = memory[Address];
        Cycles--;
        return Data;
    }

    static constexpr Byte
        INS_LDA_IM = 0xA9,
        INS_LDA_ZP = 0xA5,
        INS_LDA_ZPX = 0xB5,
        INS_JSR = 0x20;

    void LDASetStatus()
    {
        Z = (A == 0);
        N = (A & 0b10000000) > 0;
    }

    u32 Execute(u32 Cycles, Mem& memory)
    {
        u32 CyclesRequested = Cycles;

        while (Cycles > 0)
        {
            Byte Ins = FetchByte(Cycles, memory);
            switch (Ins)
            {
            case INS_LDA_IM: // Load Accumulator - Immediate
            {
                Byte Value = FetchByte(Cycles, memory);
                A = Value;

                LDASetStatus();
            }
            break;
            case INS_LDA_ZP: // Load Accumulator - Zero Page
            {
                Byte ZeroPageAddr = FetchByte(Cycles, memory);
                A = ReadByte(Cycles, ZeroPageAddr, memory);
                LDASetStatus();
            }
            break;
            case INS_LDA_ZPX: // Load Accumulator - Zero Page X
            {
                Byte ZeroPageAddr = FetchByte(Cycles, memory);
                ZeroPageAddr += X;
                Cycles--;

                A = ReadByte(Cycles, ZeroPageAddr, memory);
                LDASetStatus();
            }
            break;
            case INS_JSR: // Jump to subroutine
            {
                Word SubAddr = FetchWord(Cycles, memory);
                memory.WriteWord(PC - 1, SP, Cycles);

                SP += 2;
                PC = SubAddr;
                Cycles--;
            }
            break;
            default:
                printf("\nInstruction no handled: %d", Ins);
                break;
            }
        }

        u32 TotalCycles = CyclesRequested - Cycles;
        return TotalCycles;
    }
};

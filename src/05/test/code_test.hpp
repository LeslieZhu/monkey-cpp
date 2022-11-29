#include <gtest/gtest.h>

#include <vector>
#include <memory>

#include "code/code.hpp"

TEST(TestMaker, BasicAssertions)
{
    struct Input{
        bytecode::OpcodeType op;
        std::vector<int> operands;
        std::vector<bytecode::Opcode> expected;
    };

    bytecode::Opcode opTest1, opTest2;
    
    /* if(bytecode::BinaryEndian() == bytecode::BinaryEndianType::SMALLENDIAN)
    {
        opTest1 = 254; //0xFE
        opTest2 = 255; //0xFF
    } else {
        opTest1 = 255; //0xFF
        opTest2 = 254; //0xFE
    } */

    // BIG ENDIAN
    opTest1 = 255; //0xFF
    opTest2 = 254; //0xFE

    struct Input inputs[]
    {
        {
            bytecode::OpcodeType::OpConstant,
                {65534},
            {
                static_cast<bytecode::Opcode>(bytecode::OpcodeType::OpConstant), opTest1, opTest2
            }
        }
    };

    for(auto& item: inputs)
    {
        std::vector<bytecode::Opcode> instruction = bytecode::Make(item.op, item.operands);
        EXPECT_EQ(instruction.size(), item.expected.size());

        for(unsigned long i=0; i < instruction.size(); i++)
        {
            EXPECT_EQ(instruction[i], item.expected[i]);
        }
    }
}

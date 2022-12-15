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
        },
        {
            bytecode::OpcodeType::OpAdd,
            {},
            {
                static_cast<bytecode::Opcode>(bytecode::OpcodeType::OpAdd)
            }
        },
        {
            bytecode::OpcodeType::OpGetLocal,
            {255},
            {
                static_cast<bytecode::Opcode>(bytecode::OpcodeType::OpGetLocal),
                255
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

TEST(TestInstructionsString, BasicTest)
{
    std::vector<std::vector<bytecode::Instructions>> vinstructions{
        {
            {bytecode::Make(bytecode::OpcodeType::OpAdd, {})},
            {bytecode::Make(bytecode::OpcodeType::OpConstant, {2})},
            {bytecode::Make(bytecode::OpcodeType::OpConstant, {65535})}
        },
        {
            {bytecode::Make(bytecode::OpcodeType::OpAdd, {})},
            {bytecode::Make(bytecode::OpcodeType::OpGetLocal, {1})},
            {bytecode::Make(bytecode::OpcodeType::OpConstant, {2})},
            {bytecode::Make(bytecode::OpcodeType::OpConstant, {65535})}
        }
    };

    std::vector<std::string> expected{
        R""(0000 OpAdd
0001 OpConstant 2
0004 OpConstant 65535
)"",
        R""(0000 OpAdd
0001 OpGetLocal 1
0003 OpConstant 2
0006 OpConstant 65535
)"",
    };

    int i = -1;
    for(auto &instructions: vinstructions)
    {
        i += 1;
        bytecode::Instructions concated = bytecode::Instructions{};
        for (auto &vins : instructions)
        {
            concated.insert(concated.end(), vins.begin(), vins.end());
        }

        std::string concatedStr = bytecode::InstructionsString(concated);

        EXPECT_STREQ(concatedStr.c_str(), expected[i].c_str());
    }
}

TEST(TestReadOperands, BasicTest)
{
    struct Input {
        bytecode::OpcodeType op;
        std::vector<int> Operands;
        int byteRead;
    };

    struct Input inputs[]
    {
        {
            bytecode::OpcodeType::OpConstant, {65535}, 2
        },
        {
            bytecode::OpcodeType::OpGetLocal, {255}, 1
        }
    };

    for(auto &test: inputs)
    {
        std::vector<bytecode::Opcode> instruction = bytecode::Make(test.op, test.Operands);
        std::shared_ptr<bytecode::Definition> def = bytecode::Lookup(test.op);
        std::pair<std::vector<int>, int> operandRead = ReadOperands(def, instruction, 1);

        EXPECT_EQ(operandRead.second, test.byteRead);

        for(int i=0, size = test.Operands.size(); i < size; i++)
        {
           EXPECT_EQ(operandRead.first[i], test.Operands[i]); 
        }
    }
}

#ifndef H_CODE_H
#define H_CODE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace bytecode
{
    enum class BinaryEndianType
    {
        SMALLENDIAN = 1,
        BIGENDIAN,
    };

    BinaryEndianType BinaryEndian()
    {
        int iVal = 0xFFFE; // 65534
        unsigned char *p = (unsigned char *)(&iVal);
        if (p[0] == 0xFF)
        {
            return BinaryEndianType::BIGENDIAN;
        }
        return BinaryEndianType::SMALLENDIAN;
    }

    using Opcode = uint8_t;
    using Instructions = std::vector<Opcode>;

    enum class OpcodeType : Opcode
    {
        OpConstant = 1,
        OpPop,

        OpAdd, // '+'
        OpSub, // '-'
        OpMul, // '*'
        OpDiv, // '/'

        OpTrue, // true
        OpFalse, // false

        OpEqual, // ==
        OpNotEqual, // !=
        OpGreaterThan, // >

        OpMinus, // -
        OpBang, // !

        OpJumpNotTruthy,
        OpJump,

        OpNull, // null
    };

    std::string OpcodeTypeStr(OpcodeType op)
    {
        std::stringstream oss;
        oss << static_cast<int>(op);
        return oss.str();
    }

    //using Instructions = std::vector<OpcodeType>;

    struct Definition
    {
        std::string Name;
        std::vector<int> OperandWidths;

        Definition(const std::string &name) : Name(name) {}
        Definition(const std::string &name, const int &width) : Name(name) { OperandWidths.push_back(width); }
        ~Definition() { OperandWidths.clear(); }
    };

    static const std::map<OpcodeType, std::shared_ptr<Definition>> definitions{
        {OpcodeType::OpConstant, std::make_shared<Definition>("OpConstant", 2)},
        {OpcodeType::OpPop, std::make_shared<Definition>("OpPop")},

        {OpcodeType::OpAdd, std::make_shared<Definition>("OpAdd")},
        {OpcodeType::OpSub, std::make_shared<Definition>("OpSub")},
        {OpcodeType::OpMul, std::make_shared<Definition>("OpMul")},
        {OpcodeType::OpDiv, std::make_shared<Definition>("OpDiv")},

        {OpcodeType::OpTrue, std::make_shared<Definition>("OpTrue")},
        {OpcodeType::OpFalse, std::make_shared<Definition>("OpFalse")},

        {OpcodeType::OpEqual, std::make_shared<Definition>("OpEqual")},
        {OpcodeType::OpNotEqual, std::make_shared<Definition>("OpNotEqual")},
        {OpcodeType::OpGreaterThan, std::make_shared<Definition>("OpGreaterThan")},

        {OpcodeType::OpMinus, std::make_shared<Definition>("OpMinus")},
        {OpcodeType::OpBang, std::make_shared<Definition>("OpBang")},

        {OpcodeType::OpJumpNotTruthy, std::make_shared<Definition>("OpJumpNotTruthy", 2)},
        {OpcodeType::OpJump, std::make_shared<Definition>("OpJump", 2)},

        {OpcodeType::OpNull, std::make_shared<Definition>("OpNull")},
    };

    std::shared_ptr<Definition> Lookup(OpcodeType op){
        auto fit = definitions.find(op);
        if(fit == definitions.end())
        {
            return nullptr;
        }
        return fit->second;
    }

    void ReadUint16(Instructions &ins, int offset, uint16_t& uint16Value)
    {
        memcpy(&uint16Value, (unsigned char*)(&ins[offset]), sizeof(uint16Value));

        if(bytecode::BinaryEndian() == bytecode::BinaryEndianType::SMALLENDIAN) // from BIGENDIAN
        {
            unsigned char *p = (unsigned char *)&uint16Value;
            unsigned char tmp = *(&p[0]);
            p[0] = p[1];
            p[1] = tmp;
        }
    }

    void WriteUint16(Instructions &ins, int offset, uint16_t& uint16Value)
    {
        if(bytecode::BinaryEndian() == bytecode::BinaryEndianType::SMALLENDIAN) // to BIGENDIAN
        {
            unsigned char *p = (unsigned char *)&uint16Value;
            unsigned char tmp = *(&p[0]);
            p[0] = p[1];
            p[1] = tmp;
        }
        memcpy(&ins[offset], (unsigned char *)(&uint16Value), sizeof(uint16Value));
    }

    std::vector<Opcode> Make(OpcodeType op, std::vector<int> operands)
    {
        auto def = Lookup(op);
        if(def == nullptr)
        {
            return std::vector<Opcode>{};
        }

        int instructionLen = 1;
        for(auto &w: def->OperandWidths)
        {
            instructionLen += w;
        }

        std::vector<Opcode> instruction = std::vector<Opcode>(instructionLen);
        instruction[0] = static_cast<Opcode>(op);

        int offset = 1;
        for(unsigned long i=0; i < operands.size(); i++)
        {
            auto width = def->OperandWidths[i];
            switch(width)
            {
                case 2:
                    uint16_t uint16Value = static_cast<uint16_t>(operands[i]);

                    /* if(bytecode::BinaryEndian() == bytecode::BinaryEndianType::SMALLENDIAN) // must use BIGENDIAN
                    {
                        unsigned char *p = (unsigned char*)&uint16Value;
                        unsigned char tmp = *(&p[0]);
                        p[0] = p[1];
                        p[1] = tmp;
                    }

                    memcpy(&instruction[offset], (unsigned char *)(&uint16Value), sizeof(uint16Value)); */
                    WriteUint16(instruction, offset, uint16Value);
                    break;
            }
            offset += width;
        }

        return instruction;
    }

    std::pair<std::vector<int>, int> ReadOperands(std::shared_ptr<Definition> def, Instructions &ins, int pos)
    {
        int size = def->OperandWidths.size();
        std::vector<int> operands(size);
        int offset = 0;

        for (int i = 0; i < size; i++)
        {
            auto width = def->OperandWidths[i];
            switch(width)
            {
                case 2:
                    {
                        uint16_t uint16Value;
                        ReadUint16(ins, pos, uint16Value);
                        operands[i] = static_cast<int>(uint16Value);
                    }
                    break;
            }

            offset += width;
        }

        return std::make_pair(operands, offset);
    }

    std::string fmtInstruction(std::shared_ptr<Definition> def, std::vector<int> operands)
    {
        std::stringstream oss;

        unsigned long operandCount = def->OperandWidths.size();

        if(operands.size() != operandCount)
        {
            oss << "ERROR: operand len " << operands.size() << " dose not match defined " << operandCount << "\n";
            return oss.str();
        }

        switch(operandCount)
        {
            case 0:
                return def->Name;
            case 1:
                {
                    oss << def->Name << " " << operands[0];
                    return oss.str();
                }
                break;
        }

        oss << "ERROR: unhandled operandCount for " << def->Name << "\n";
        return oss.str();
    }

    std::string InstructionsString(Instructions& ins)
    {
        std::stringstream oss;

        int i = 0, size = ins.size();
        while(i < size)
        {
            auto def = Lookup(static_cast<OpcodeType>(ins[i]));
            if(def == nullptr)
            {
                std::cout << "ERROR: can not Lookup this: " << unsigned(ins[i]) << std::endl;
                i += 1;
                continue;
            }

            auto operands = ReadOperands(def, ins, i+1);

            oss << std::setw(4) << std::setfill('0') << i << " ";
            oss << fmtInstruction(def, operands.first) << "\n";

            i += (1 + operands.second);
        }

        return oss.str();
    }
}

#endif // H_CODE_H
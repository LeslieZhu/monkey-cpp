#ifndef H_CODE_H
#define H_CODE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <arpa/inet.h>

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

    std::string InstructionsString(Instructions& instructions)
    {
        return "";
    }

    enum class OpcodeType : Opcode
    {
        OpConstant = 1,
    };

    struct Definition
    {
        std::string Name;
        std::vector<int> OperandWidths;

        Definition(const std::string &name, const int &width) : Name(name) { OperandWidths.push_back(width); }
        ~Definition() { OperandWidths.clear(); }
    };

    static const std::map<OpcodeType, std::shared_ptr<Definition>> definitions{
        {OpcodeType::OpConstant, std::make_shared<Definition>("OpConstant", 2)},
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
    }

    void WriteUint16(Instructions &ins, int offset, uint16_t& uint16Value)
    {
        if(bytecode::BinaryEndian() == bytecode::BinaryEndianType::SMALLENDIAN) // must use BIGENDIAN
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

    std::pair<std::vector<Opcode>, int> ReadOperands(std::shared_ptr<Definition> def, Instructions &ins)
    {
        int size = def->OperandWidths.size();
        std::vector<Opcode> operands = Make({}, {size});
        int offset = 1;

        for (int i = 0; i < size; i++)
        {
            auto width = def->OperandWidths[i];
            switch(width)
            {
                case 2:
                    {
                        uint16_t uint16Value;
                        //memcpy(&uint16Value, (unsigned char*)(&ins[offset]), sizeof(uint16Value));
                        ReadUint16(ins, offset, uint16Value);
                        operands[i] = static_cast<int>(uint16Value);
                    }
                    break;
            }

            offset += width;
        }

        return std::make_pair(operands, offset);
    }
}

#endif // H_CODE_H
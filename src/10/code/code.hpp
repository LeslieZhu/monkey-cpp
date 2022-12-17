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
        OpConstant = 0,
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

        OpGetGlobal,
        OpSetGlobal,

        OpGetLocal,
        OpSetLocal,

        OpArray,
        OpHash,
        OpIndex,

        OpCall,
        OpReturnValue,
        OpReturn,   // return null

        OpGetBuiltin,
        OpClosure,
        OpGetFree,
    };

    std::string OpcodeTypeStr(OpcodeType op)
    {
        switch(op)
        {
            case OpcodeType::OpConstant:
                return "OpConstant";
            case OpcodeType::OpPop:
                return "OpPop";
            case OpcodeType::OpAdd:
                return "+";
            case OpcodeType::OpSub:
                return "-";
            case OpcodeType::OpMul:
                return "*";
            case OpcodeType::OpDiv:
                return "/";
            case OpcodeType::OpTrue:
                return "OpTrue";
            case OpcodeType::OpFalse:
                return "OpFalse";
            case OpcodeType::OpEqual:
                return "==";
            case OpcodeType::OpNotEqual:
                return "!=";
            case OpcodeType::OpGreaterThan:
                return ">";
            case OpcodeType::OpMinus:
                return "-";
            case OpcodeType::OpBang:
                return "!";
            case OpcodeType::OpJumpNotTruthy:
                return "OpJumpNotTruthy";
            case OpcodeType::OpJump:
                return "OpJump";
            case OpcodeType::OpNull:
                return "OpNull";
            case OpcodeType::OpGetGlobal:
                return "OpGetGlobal";
            case OpcodeType::OpSetGlobal:
                return "OpSetGlobal";
            case OpcodeType::OpGetLocal:
                return "OpGetLocal";
            case OpcodeType::OpSetLocal:
                return "OpSetLocal";
            case OpcodeType::OpArray:
                return "OpArray";
            case OpcodeType::OpHash:
                return "OpHash";
            case OpcodeType::OpIndex:
                return "OpIndex";
            case OpcodeType::OpCall:
                return "OpCall";
            case OpcodeType::OpReturnValue:
                return "OpReturnValue";
            case OpcodeType::OpReturn:
                return "OpReturn";
            case OpcodeType::OpGetBuiltin:
                return "OpGetBuiltin";
            case OpcodeType::OpClosure:
                return "OpClosure";
            case OpcodeType::OpGetFree:
                return "OpGetFree";
            default:
                return std::to_string(static_cast<int>(op));
        }
    }

    //using Instructions = std::vector<OpcodeType>;

    struct Definition
    {
        std::string Name;
        std::vector<int> OperandWidths;

        Definition(const std::string &name) : Name(name) {}
        Definition(const std::string &name, const int &width) : Name(name) { OperandWidths.push_back(width); }
        Definition(const std::string &name, std::vector<int> widths) : Name(name)
        {
                OperandWidths.insert(OperandWidths.end(), widths.begin(), widths.end());
        }

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

        {OpcodeType::OpGetGlobal, std::make_shared<Definition>("OpGetGlobal", 2)},
        {OpcodeType::OpSetGlobal, std::make_shared<Definition>("OpSetGlobal", 2)},

        {OpcodeType::OpGetLocal, std::make_shared<Definition>("OpGetLocal", 1)},
        {OpcodeType::OpSetLocal, std::make_shared<Definition>("OpSetLocal", 1)},

        {OpcodeType::OpArray, std::make_shared<Definition>("OpArray", 2)},
        {OpcodeType::OpHash, std::make_shared<Definition>("OpHash", 2)},

        {OpcodeType::OpIndex, std::make_shared<Definition>("OpIndex")},

        {OpcodeType::OpCall, std::make_shared<Definition>("OpCall", 1)},
        {OpcodeType::OpReturnValue, std::make_shared<Definition>("OpReturnValue")},
        {OpcodeType::OpReturn, std::make_shared<Definition>("OpReturn")},

        {OpcodeType::OpGetBuiltin, std::make_shared<Definition>("OpGetBuiltin", 1)},

        {OpcodeType::OpClosure, std::make_shared<Definition>("OpClosure", std::vector<int>{2, 1})},
        {OpcodeType::OpGetFree, std::make_shared<Definition>("OpGetFree", 1)},
    };

    std::shared_ptr<Definition> Lookup(OpcodeType op){
        auto fit = definitions.find(op);
        if(fit == definitions.end())
        {
            return nullptr;
        }
        return fit->second;
    }

    void ReadUint8(Instructions &ins, int offset, uint8_t& uint8Value)
    {
        memcpy(&uint8Value, (unsigned char*)(&ins[offset]), sizeof(uint8Value));
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
                    {
                        uint16_t uint16Value = static_cast<uint16_t>(operands[i]);
                        WriteUint16(instruction, offset, uint16Value);
                    }
                    break;
                case 1:
                    {
                        instruction[offset] = static_cast<Opcode>(operands[i]);
                    }
                    break;
            }
            offset += width;
        }

        return instruction;
    }

    std::vector<Opcode> Make(OpcodeType op)
    {
        return Make(op, {});
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
                case 1:
                    {
                        uint8_t uint8Value;
                        ReadUint8(ins, pos, uint8Value);
                        operands[i] = static_cast<int>(uint8Value);
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
            case 2:
                {
                    oss << def->Name << " " << operands[0] << " " << operands[1];
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
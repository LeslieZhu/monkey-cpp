#ifndef H_VM_H
#define H_VM_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "objects/objects.hpp"
#include "compiler/compiler.hpp"
#include "code/code.hpp"
#include "evaluator/evaluator.hpp"

namespace vm
{
    const int StackSize = 2048;

    struct VM{
        std::vector<std::shared_ptr<objects::Object>> constants;
        bytecode::Instructions instructions;

        std::vector<std::shared_ptr<objects::Object>> stack;
        int sp; // 始终指向调用栈的下一个空闲位置，栈顶的值是stack[sp-1]

        VM(std::vector<std::shared_ptr<objects::Object>>& objs, bytecode::Instructions& ins):
        constants(objs),
        instructions(ins)
        {
            stack.resize(StackSize);
            sp = 0;
        }

        std::shared_ptr<objects::Object> LastPoppedStackElem()
        {
            return stack[sp];
        }

        std::shared_ptr<objects::Object> StackTop()
        {
            if(sp == 0)
            {
                return nullptr;
            }

            return stack[sp - 1];
        }

        std::shared_ptr<objects::Object> Push(std::shared_ptr<objects::Object> obj)
        {
            if(sp > StackSize)
            {
                return evaluator::newError("stack overflow");
            }

            stack[sp] = obj;
            sp += 1;

            return nullptr;
        }

        std::shared_ptr<objects::Object> Pop()
        {
            auto obj = stack[sp - 1];
            sp -= 1;

            return obj;
        }

        std::shared_ptr<objects::Object> Run()
        {
            int size = instructions.size();
            for(int ip=0; ip < size; ip++)
            {
                bytecode::OpcodeType op = static_cast<bytecode::OpcodeType>(instructions[ip]);

                switch(op)
                {
                    case bytecode::OpcodeType::OpConstant:
                        {
                            uint16_t constIndex;
                            bytecode::ReadUint16(instructions, ip+1, constIndex);
                            ip += 2;
                            auto result = Push(constants[constIndex]);
                            if(evaluator::isError(result))
                            {
                                return result;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpAdd:
                        {
                            auto right = Pop();
                            auto left = Pop();
                            auto rightObj = std::dynamic_pointer_cast<objects::Integer>(right);
                            auto leftObj = std::dynamic_pointer_cast<objects::Integer>(left);
                            auto result = leftObj->Value + rightObj->Value;
                            Push(std::make_shared<objects::Integer>(result));
                        }
                        break;
                    case bytecode::OpcodeType::OpPop:
                        {
                            Pop();
                        }
                        break;
                }
            }

            return nullptr;
        }
    };

    std::shared_ptr<VM> New(std::shared_ptr<compiler::ByteCode> bytecode)
    {
        return std::make_shared<VM>(bytecode->Constants, bytecode->Instructions);
    }
}


#endif // H_VM_H
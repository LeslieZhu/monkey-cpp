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
    const int GlobalsSize = 65536;

    struct VM{
        std::vector<std::shared_ptr<objects::Object>> constants;
        std::vector<std::shared_ptr<objects::Object>> globals;
        bytecode::Instructions instructions;

        std::vector<std::shared_ptr<objects::Object>> stack;
        int sp; // 始终指向调用栈的下一个空闲位置，栈顶的值是stack[sp-1]

        VM(std::vector<std::shared_ptr<objects::Object>>& objs, bytecode::Instructions& ins):
        constants(objs),
        instructions(ins)
        {
            globals.resize(GlobalsSize);
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
                    case bytecode::OpcodeType::OpSub:
                    case bytecode::OpcodeType::OpMul:
                    case bytecode::OpcodeType::OpDiv:
                        {
                            auto result = executeBinaryOperaction(op);
                            if(evaluator::isError(result))
                            {
                                return result;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpPop:
                        {
                            Pop();
                        }
                        break;
                    case bytecode::OpcodeType::OpTrue:
                        {
                            auto result = Push(objects::TRUE_OBJ);
                            if(evaluator::isError(result))
                            {
                                return result;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpFalse:
                        {
                            auto result = Push(objects::FALSE_OBJ);
                            if(evaluator::isError(result))
                            {
                                return result;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpEqual:
                    case bytecode::OpcodeType::OpNotEqual:
                    case bytecode::OpcodeType::OpGreaterThan:
                        {
                            auto result = executeComparison(op);
                            if(evaluator::isError(result))
                            {
                                return result;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpBang:
                        {
                            auto result = executeBangOperator();
                            if(evaluator::isError(result))
                            {
                                return result;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpMinus:
                        {
                            auto result = executeMinusOperator();
                            if(evaluator::isError(result))
                            {
                                return result;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpJump:
                        {
                            uint16_t pos;
                            bytecode::ReadUint16(instructions, ip+1, pos);
                            ip = pos - 1;
                        }
                        break;
                    case bytecode::OpcodeType::OpJumpNotTruthy:
                        {
                            uint16_t pos;
                            bytecode::ReadUint16(instructions, ip+1, pos);
                            ip += 2;
                            
                            auto condition = Pop();
                            if(!evaluator::isTruthy(condition))
                            {
                                ip = pos - 1;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpNull:
                        {
                            auto result = Push(objects::NULL_OBJ);
                            if(evaluator::isError(result))
                            {
                                return result;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpSetGlobal:
                        {
                            uint16_t globalIndex;
                            bytecode::ReadUint16(instructions, ip+1, globalIndex);
                            ip += 2;
                            globals[globalIndex] = Pop();
                        }
                        break;
                    case bytecode::OpcodeType::OpGetGlobal:
                        {
                            uint16_t globalIndex;
                            bytecode::ReadUint16(instructions, ip+1, globalIndex);
                            ip += 2;
                            auto result = Push(globals[globalIndex]);
                            if(evaluator::isError(result))
                            {
                               return result;
                            }
                        }
                        break;
                }
            }

            return nullptr;
        }

        std::shared_ptr<objects::Object> executeBinaryOperaction(bytecode::OpcodeType op)
        {
            auto right = Pop();
            auto left = Pop();

            if(left->Type() == objects::ObjectType::INTEGER && right->Type() == objects::ObjectType::INTEGER)
            {
                return executeBInaryIntegerOperaction(op, left, right);
            } else {
                return evaluator::newError("unsupported types for binary operaction: " + left->TypeStr() + " " + right->TypeStr());
            }
        }

        std::shared_ptr<objects::Object> executeBangOperator()
        {
            auto operand = Pop();

            if(operand == objects::TRUE_OBJ)
            {
                return Push(objects::FALSE_OBJ);
            }
            else if(operand == objects::FALSE_OBJ)
            {
                return Push(objects::TRUE_OBJ);
            }
            else
            {
                return Push(objects::FALSE_OBJ);
            }
        }

        std::shared_ptr<objects::Object> executeMinusOperator()
        {
            auto operand = Pop();

            if(operand->Type() != objects::ObjectType::INTEGER)
            {
                return evaluator::newError("unsupported type for negation: " + operand->TypeStr());
            }
            auto integerObj = std::dynamic_pointer_cast<objects::Integer>(operand);
            return Push(std::make_shared<objects::Integer>(-1 * integerObj->Value));
        }

        std::shared_ptr<objects::Object> executeBInaryIntegerOperaction(bytecode::OpcodeType op,
                                                                        std::shared_ptr<objects::Object> left,
                                                                        std::shared_ptr<objects::Object> right)
        {
            auto rightObj = std::dynamic_pointer_cast<objects::Integer>(right);
            auto leftObj = std::dynamic_pointer_cast<objects::Integer>(left);

            int64_t result = 0;

            switch (op)
            {
            case bytecode::OpcodeType::OpAdd:
                result = leftObj->Value + rightObj->Value;
                break;
            case bytecode::OpcodeType::OpSub:
                result = leftObj->Value - rightObj->Value;
                break;
            case bytecode::OpcodeType::OpMul:
                result = leftObj->Value * rightObj->Value;
                break;
            case bytecode::OpcodeType::OpDiv:
                result = leftObj->Value / rightObj->Value;
                break;
            
            default:
                return evaluator::newError("unknow integer operator: " + bytecode::OpcodeTypeStr(op));
                break;
            }

            return Push(std::make_shared<objects::Integer>(result));
        }

        std::shared_ptr<objects::Object> executeComparison(bytecode::OpcodeType op)
        {
            auto right = Pop();
            auto left = Pop();

            if(left->Type() == objects::ObjectType::INTEGER && right->Type() == objects::ObjectType::INTEGER)
            {
                return executeIntegerComparison(op, left, right);
            } 

            switch (op)
            {
            case bytecode::OpcodeType::OpEqual:
                return Push(evaluator::nativeBoolToBooleanObject(right == left));
                break;
            case bytecode::OpcodeType::OpNotEqual:
                return Push(evaluator::nativeBoolToBooleanObject(right != left));
                break;
            
            default:
                return evaluator::newError("unknow operator: " + bytecode::OpcodeTypeStr(op) + " (" + left->TypeStr() + " " + right->TypeStr() + ")");
            }
        }

        std::shared_ptr<objects::Object> executeIntegerComparison(bytecode::OpcodeType op,
                                                                        std::shared_ptr<objects::Object> left,
                                                                        std::shared_ptr<objects::Object> right)
        {
            auto rightObj = std::dynamic_pointer_cast<objects::Integer>(right);
            auto leftObj = std::dynamic_pointer_cast<objects::Integer>(left);

            switch (op)
            {
            case bytecode::OpcodeType::OpEqual:
                return Push(evaluator::nativeBoolToBooleanObject(rightObj->Value == leftObj->Value));
                break;
            case bytecode::OpcodeType::OpNotEqual:
                return Push(evaluator::nativeBoolToBooleanObject(rightObj->Value != leftObj->Value));
                break;
            case bytecode::OpcodeType::OpGreaterThan:
                return Push(evaluator::nativeBoolToBooleanObject(leftObj->Value > rightObj->Value));
                break;

            default:
                return evaluator::newError("unknow operator: " + bytecode::OpcodeTypeStr(op));
            }
        }
    };

    std::shared_ptr<VM> New(std::shared_ptr<compiler::ByteCode> bytecode)
    {
        return std::make_shared<VM>(bytecode->Constants, bytecode->Instructions);
    }
}


#endif // H_VM_H
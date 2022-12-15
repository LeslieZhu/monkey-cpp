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
#include "vm/frame.hpp"

namespace vm
{
    const int FrameSize = 1024;
    const int StackSize = 2048;
    const int GlobalsSize = 65536;

    struct VM{
        std::vector<std::shared_ptr<objects::Object>> constants;
        std::vector<std::shared_ptr<objects::Object>> globals;

        std::vector<std::shared_ptr<objects::Object>> stack;
        int sp; // 始终指向调用栈的下一个空闲位置，栈顶的值是stack[sp-1]

        std::vector<std::shared_ptr<Frame>> frames;
        int frameIndex;

        VM(std::vector<std::shared_ptr<objects::Object>>& objs, std::vector<std::shared_ptr<Frame>>& f):
        constants(objs),
        frames(f)
        {
            globals.resize(GlobalsSize);
            stack.resize(StackSize);
            sp = 0;
            frameIndex = 1;
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
            auto frame = currentFrame();

            int ip;
            bytecode::OpcodeType op;
            bytecode::Instructions instructions = currentFrame()->Instruction();
            int ins_size = instructions.size();

            while(frame->ip < ins_size - 1) // frame->ip start with -1
            {
                frame->ip += 1;

                ip = frame->ip;
                op = static_cast<bytecode::OpcodeType>(instructions[ip]);

                switch(op)
                {
                    case bytecode::OpcodeType::OpConstant:
                        {
                            uint16_t constIndex;
                            bytecode::ReadUint16(instructions, ip+1, constIndex);
                            frame->ip += 2;
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
                            frame->ip = pos - 1;
                        }
                        break;
                    case bytecode::OpcodeType::OpJumpNotTruthy:
                        {
                            uint16_t pos;
                            bytecode::ReadUint16(instructions, ip+1, pos);
                            frame->ip += 2;
                            
                            auto condition = Pop();
                            if(!evaluator::isTruthy(condition))
                            {
                                frame->ip = pos - 1;
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
                            frame->ip += 2;
                            globals[globalIndex] = Pop();
                        }
                        break;
                    case bytecode::OpcodeType::OpGetGlobal:
                        {
                            uint16_t globalIndex;
                            bytecode::ReadUint16(instructions, ip+1, globalIndex);
                            frame->ip += 2;
                            auto result = Push(globals[globalIndex]);
                            if(evaluator::isError(result))
                            {
                               return result;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpSetLocal:
                        {
                            uint8_t localIndex;
                            bytecode::ReadUint8(instructions, ip+1, localIndex);
                            frame->ip += 1;

                            stack[frame->basePointer + int(localIndex)] = Pop();
                        }
                        break;
                    case bytecode::OpcodeType::OpGetLocal:
                        {
                            uint8_t localIndex;
                            bytecode::ReadUint8(instructions, ip+1, localIndex);
                            frame->ip += 1;

                            auto result = Push(stack[frame->basePointer + int(localIndex)]);
                            if(evaluator::isError(result))
                            {
                               return result;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpArray:
                        {
                            uint16_t numElements;
                            bytecode::ReadUint16(instructions, ip+1, numElements);
                            frame->ip += 2;

                            auto arrayObj = buildArray(sp - numElements, sp);
                            if(evaluator::isError(arrayObj))
                            {
                               return arrayObj;
                            }

                            sp -= numElements; // 移出

                            auto result = Push(arrayObj);
                            if(evaluator::isError(result))
                            {
                               return result;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpHash:
                        {
                            uint16_t numElements;
                            bytecode::ReadUint16(instructions, ip+1, numElements);
                            frame->ip += 2;

                            auto hashObj = buildHash(sp - numElements, sp);
                            if(evaluator::isError(hashObj))
                            {
                               return hashObj;
                            }

                            sp -= numElements;

                            auto result = Push(hashObj);
                            if(evaluator::isError(result))
                            {
                               return result;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpIndex:
                        {
                            auto index = Pop();
                            auto left = Pop();

                            auto result = executeIndexExpression(left, index);
                            if(evaluator::isError(result))
                            {
                               return result;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpCall:
                        {
                            auto fnObj = stack[sp - 1];
                            if(fnObj->Type() != objects::ObjectType::COMPILED_FUNCTION)
                            {
                                return evaluator::newError("calling non-function");
                            }

                            auto compiledFnObj = std::dynamic_pointer_cast<objects::CompiledFunction>(fnObj);
                            auto funcFrame = NewFrame(compiledFnObj, sp);

                            pushFrame(funcFrame);

                            frame = currentFrame();
                            instructions = frame->Instruction();
                            ins_size = instructions.size();

                            sp = frame->basePointer + compiledFnObj->NumLocals;
                        }
                        break;
                    case bytecode::OpcodeType::OpReturnValue:
                        {
                            auto returnValue = Pop();

                            auto callFrame = popFrame();
                            sp = callFrame->basePointer - 1;

                            frame = currentFrame();
                            instructions = frame->Instruction();
                            ins_size = instructions.size();

                            //Pop(); // 函数本体出栈

                            auto result = Push(returnValue);
                            if(evaluator::isError(result))
                            {
                               return result;
                            }
                        }
                        break;
                    case bytecode::OpcodeType::OpReturn:
                        {
                            auto callFrame = popFrame();
                            sp = callFrame->basePointer - 1;

                            frame = currentFrame();
                            instructions = frame->Instruction();
                            ins_size = instructions.size();

                            //Pop(); // 函数本体出栈

                            auto result = Push(objects::NULL_OBJ);
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
                return executeBinaryIntegerOperaction(op, left, right);
            } 
            else if(left->Type() == objects::ObjectType::STRING && right->Type() == objects::ObjectType::STRING)
            {
                return executeBinaryStringOperaction(op, left, right);
            }
            else {
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

        std::shared_ptr<objects::Object> executeBinaryIntegerOperaction(bytecode::OpcodeType op,
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

        std::shared_ptr<objects::Object> executeBinaryStringOperaction(bytecode::OpcodeType op,
                                                                        std::shared_ptr<objects::Object> left,
                                                                        std::shared_ptr<objects::Object> right)
        {
            auto rightObj = std::dynamic_pointer_cast<objects::String>(right);
            auto leftObj = std::dynamic_pointer_cast<objects::String>(left);

            std::string result;

            switch (op)
            {
            case bytecode::OpcodeType::OpAdd:
                result = leftObj->Value + rightObj->Value;
                break;
            
            default:
                return evaluator::newError("unknow string operator: " + bytecode::OpcodeTypeStr(op));
                break;
            }

            return Push(std::make_shared<objects::String>(result));
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

        std::shared_ptr<objects::Object> executeIndexExpression(std::shared_ptr<objects::Object> left,
                                                                std::shared_ptr<objects::Object> index)
        {
            if(left->Type() == objects::ObjectType::ARRAY && index->Type() == objects::ObjectType::INTEGER)
            {
                auto result = evaluator::evalArrayIndexExpression(left, index);
                if (evaluator::isError(result))
                {
                    return result;
                }

                return Push(result);
                
            }
            else if(left->Type() == objects::ObjectType::HASH)
            {
                auto result = evaluator::evalHashIndexExpression(left, index);
                if (evaluator::isError(result))
                {
                    return result;
                }

                return Push(result);
            }
            else 
            {
                return evaluator::newError("index operator not supported: " + left->TypeStr());
            }
        }

        std::shared_ptr<objects::Object> buildArray(const int& startIndex, const int& endIndex)
        {
            std::vector<std::shared_ptr<objects::Object>> elements(endIndex - startIndex);
            for(int i=startIndex; i < endIndex; i++)
            {
                elements[i - startIndex] = stack[i];
            }

            return std::make_shared<objects::Array>(elements);
        }

        std::shared_ptr<objects::Object> buildHash(const int& startIndex, const int& endIndex)
        {
            std::map<objects::HashKey, std::shared_ptr<objects::HashPair>> hashPairs;
            
            for(int i=startIndex; i < endIndex; i += 2)
            {
                auto key = stack[i];
                auto value = stack[i+1];

                auto pair = std::make_shared<objects::HashPair>(key, value);

                if(!key->Hashable())
                {
                    return evaluator::newError("unusable as hash type: " + key->TypeStr());
                }

                hashPairs[key->GetHashKey()] = pair;
            }

            return std::make_shared<objects::Hash>(hashPairs);
        }

        std::shared_ptr<Frame> currentFrame()
        {
            return frames[frameIndex - 1];
        }

        void pushFrame(std::shared_ptr<Frame> f)
        {
            frames[frameIndex] = f;
            frameIndex += 1;
        }

        std::shared_ptr<Frame> popFrame()
        {
            frameIndex -= 1;
            return frames[frameIndex];
        }
    };

    std::shared_ptr<VM> New(std::shared_ptr<compiler::ByteCode> bytecode)
    {
        auto mainFn = std::make_shared<objects::CompiledFunction>(bytecode->Instructions, 0);
        auto mainFrame = NewFrame(mainFn, 0);

        std::vector<std::shared_ptr<Frame>> frames(FrameSize);
        frames[0] = mainFrame;

        return std::make_shared<VM>(bytecode->Constants, frames);
    }

    std::shared_ptr<VM> NewWithGlobalsStore(std::shared_ptr<compiler::ByteCode> bytecode,
                                            std::vector<std::shared_ptr<objects::Object>>& s)
    {
        std::shared_ptr<VM> vm = New(bytecode);
        vm->globals = s;
        return vm;
    }
}


#endif // H_VM_H
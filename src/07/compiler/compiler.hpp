#ifndef H_COMPILER_H
#define H_COMPILER_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>

#include "ast/ast.hpp"
#include "objects/objects.hpp"
#include "code/code.hpp"
#include "evaluator/evaluator.hpp"

namespace compiler
{
    struct ByteCode {
        bytecode::Instructions Instructions;
        std::vector<std::shared_ptr<objects::Object>> Constants;

        ByteCode(bytecode::Instructions &instructions,
                 std::vector<std::shared_ptr<objects::Object>> &constants) : Instructions(instructions),
                                                                             Constants(constants)
        {
        }
    };

    struct EmittedInstruction{
        bytecode::OpcodeType Opcode;
        int Position;

        EmittedInstruction(){}
        EmittedInstruction(bytecode::OpcodeType op, int pos): Opcode(op), Position(pos){}
    };

    struct Compiler
    {
        bytecode::Instructions instructions;
        std::vector<std::shared_ptr<objects::Object>> constants;

        EmittedInstruction lastInstruction;
        EmittedInstruction prevInstruction;

        Compiler(){}

        std::shared_ptr<objects::Error> Compile([[maybe_unused]] std::shared_ptr<ast::Node> node)
        {
            if(node->GetNodeType() == ast::NodeType::Program)
            {
                std::shared_ptr<ast::Program> program = std::dynamic_pointer_cast<ast::Program>(node);
                for(auto &stmt: program->v_pStatements)
                {
                    auto resultObj = Compile(stmt);
                    if(evaluator::isError(resultObj))
                    {
                        return resultObj;
                    }
                }
            }
            else if(node->GetNodeType() == ast::NodeType::BlockStatement)
            {
                std::shared_ptr<ast::BlockStatement> blockObj = std::dynamic_pointer_cast<ast::BlockStatement>(node);
                for(auto &stmt: blockObj->v_pStatements)
                {
                    auto resultObj = Compile(stmt);
                    if(evaluator::isError(resultObj))
                    {
                        return resultObj;
                    }
                }
            }
            else if(node->GetNodeType() == ast::NodeType::ExpressionStatement)
            {
                std::shared_ptr<ast::ExpressionStatement> exprStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(node);

                auto resultObj = Compile(exprStmt->pExpression);
                if (evaluator::isError(resultObj))
                {
                    return resultObj;
                }
                emit(bytecode::OpcodeType::OpPop, {});
            }
            else if(node->GetNodeType() == ast::NodeType::InfixExpression)
            {
                std::shared_ptr<ast::InfixExpression> infixObj = std::dynamic_pointer_cast<ast::InfixExpression>(node);

                if (infixObj->Operator == "<")
                {
                    auto resultObj = Compile(infixObj->pRight);
                    if (evaluator::isError(resultObj))
                    {
                        return resultObj;
                    }

                    resultObj = Compile(infixObj->pLeft);
                    if (evaluator::isError(resultObj))
                    {
                        return resultObj;
                    }

                    
                    emit(bytecode::OpcodeType::OpGreaterThan, {});

                    return nullptr;
                }

                auto resultObj = Compile(infixObj->pLeft);
                if (evaluator::isError(resultObj))
                {
                    return resultObj;
                }

                resultObj = Compile(infixObj->pRight);
                if (evaluator::isError(resultObj))
                {
                    return resultObj;
                }

                if (infixObj->Operator == "+")
                {
                    emit(bytecode::OpcodeType::OpAdd, {});
                } 
                else if (infixObj->Operator == "-")
                {
                    emit(bytecode::OpcodeType::OpSub, {});
                }
                else if (infixObj->Operator == "*")
                {
                    emit(bytecode::OpcodeType::OpMul, {});
                }
                else if (infixObj->Operator == "/")
                {
                    emit(bytecode::OpcodeType::OpDiv, {});
                }
                else if (infixObj->Operator == ">")
                {
                    emit(bytecode::OpcodeType::OpGreaterThan, {});
                }
                else if (infixObj->Operator == "==")
                {
                    emit(bytecode::OpcodeType::OpEqual, {});
                }
                else if (infixObj->Operator == "!=")
                {
                    emit(bytecode::OpcodeType::OpNotEqual, {});
                }
                else {
                    return evaluator::newError("unknow operator: " + infixObj->Operator);
                }
            }
            else if(node->GetNodeType() == ast::NodeType::PrefixExpression)
            {
                std::shared_ptr<ast::PrefixExpression> prefixObj = std::dynamic_pointer_cast<ast::PrefixExpression>(node);
                auto resultObj = Compile(prefixObj->pRight);
                if (evaluator::isError(resultObj))
                {
                    return resultObj;
                }

                if(prefixObj->Operator == "!")
                {
                    emit(bytecode::OpcodeType::OpBang, {});
                }
                else if(prefixObj->Operator == "-")
                {
                    emit(bytecode::OpcodeType::OpMinus, {});
                }
                else{
                    return evaluator::newError("unknow operator: " + prefixObj->Operator);
                }
            }
            else if(node->GetNodeType() == ast::NodeType::IfExpression)
            {
                std::shared_ptr<ast::IfExpression> ifObj = std::dynamic_pointer_cast<ast::IfExpression>(node);

                auto resultObj = Compile(ifObj->pCondition);
                if (evaluator::isError(resultObj))
                {
                    return resultObj;
                }

                // 预设一个偏移量方便后续回填
                auto jumpNotTruthyPos =  emit(bytecode::OpcodeType::OpJumpNotTruthy, {9999});

                resultObj = Compile(ifObj->pConsequence);
                if (evaluator::isError(resultObj))
                {
                    return resultObj;
                }

                if(lastInstructionIsPop())
                {
                    removeLastPop();
                }

                auto jumpPos = emit(bytecode::OpcodeType::OpJump, {9999});

                auto afterConsequencePos = instructions.size();
                changeOperand(jumpNotTruthyPos, afterConsequencePos);

                if(ifObj->pAlternative == nullptr)
                {
                    emit(bytecode::OpcodeType::OpNull, {});
                }
                else
                {
                    resultObj = Compile(ifObj->pAlternative);
                    if (evaluator::isError(resultObj))
                    {
                        return resultObj;
                    }

                    if (lastInstructionIsPop())
                    {
                        removeLastPop();
                    }
                }

                afterConsequencePos = instructions.size();
                changeOperand(jumpPos, afterConsequencePos);
            }
            else if(node->GetNodeType() == ast::NodeType::IntegerLiteral)
            {
                std::shared_ptr<ast::IntegerLiteral> integerLiteral = std::dynamic_pointer_cast<ast::IntegerLiteral>(node);
			    auto integerObj = std::make_shared<objects::Integer>(integerLiteral->Value);
                auto pos = addConstant(integerObj);
                emit(bytecode::OpcodeType::OpConstant, {pos});
            }
            else if(node->GetNodeType() == ast::NodeType::Boolean)
            {
                auto boolAst = std::dynamic_pointer_cast<ast::Boolean>(node);
                if(boolAst->Value)
                {
                    emit(bytecode::OpcodeType::OpTrue,{});
                } else {
                    emit(bytecode::OpcodeType::OpFalse,{});
                }
            }


            return nullptr;
        }

        int addConstant(std::shared_ptr<objects::Object> obj)
        {
            constants.push_back(obj);
            return (constants.size() - 1);
        }

        int emit(bytecode::OpcodeType op, std::vector<int> operands)
        {
            auto ins = bytecode::Make(op, operands);
            auto pos = addInstruction(ins);

            setLastInstruction(op, pos);        
            return pos;
        }

        int addInstruction(bytecode::Instructions ins)
        {
            auto posNewInstruction = instructions.size();
            
            instructions.insert(instructions.end(), ins.begin(), ins.end());
            return posNewInstruction;
        }

        void setLastInstruction(bytecode::OpcodeType op, int pos)
        {
            prevInstruction = lastInstruction;
            lastInstruction = EmittedInstruction(op, pos);
        }

        bool lastInstructionIsPop()
        {
            return (lastInstruction.Opcode == bytecode::OpcodeType::OpPop);
        }

        void removeLastPop()
        {
            instructions.assign(instructions.begin(), instructions.begin() + lastInstruction.Position);
            lastInstruction = prevInstruction;
        }

        void replaceInstruction(int pos, const bytecode::Instructions& newInstruction)
        {
            for (int i = 0, size = newInstruction.size(); i < size; i++)
            {
                instructions[pos + i] = newInstruction[i];
            }
        }

        void changeOperand(int opPos, int operand)
        {
            bytecode::OpcodeType op = static_cast<bytecode::OpcodeType>(instructions[opPos]);
            auto newInstruction = bytecode::Make(op, {operand});

            replaceInstruction(opPos, newInstruction);
        }

        std::shared_ptr<ByteCode> Bytecode()
        {
            return std::make_shared<ByteCode>(instructions, constants);
        }
    };

    std::shared_ptr<Compiler> New()
    {
        return std::make_shared<Compiler>();
    }
}

#endif // H_COMPILER_H
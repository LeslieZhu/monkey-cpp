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

    struct Compiler
    {
        bytecode::Instructions instructions;
        std::vector<std::shared_ptr<objects::Object>> constants;

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
            else if(node->GetNodeType() == ast::NodeType::ExpressionStatement)
            {
                std::shared_ptr<ast::ExpressionStatement> exprStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(node);

                auto resultObj = Compile(exprStmt->pExpression);
                if (evaluator::isError(resultObj))
                {
                    return resultObj;
                }
            }
            else if(node->GetNodeType() == ast::NodeType::InfixExpression)
            {
                std::shared_ptr<ast::InfixExpression> infixObj = std::dynamic_pointer_cast<ast::InfixExpression>(node);

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
                else {
                    return evaluator::newError("unknow operator: " + infixObj->Operator);
                }
            }
            else if(node->GetNodeType() == ast::NodeType::IntegerLiteral)
            {
                std::shared_ptr<ast::IntegerLiteral> integerLiteral = std::dynamic_pointer_cast<ast::IntegerLiteral>(node);
			    auto integerObj = std::make_shared<objects::Integer>(integerLiteral->Value);
                auto pos = addConstant(integerObj);
                emit(bytecode::OpcodeType::OpConstant, {pos});
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
            return pos;
        }

        int addInstruction(bytecode::Instructions ins)
        {
            auto posNewInstruction = instructions.size();

            for(auto &item: ins){
                instructions.push_back(item);
            }
            return posNewInstruction;
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
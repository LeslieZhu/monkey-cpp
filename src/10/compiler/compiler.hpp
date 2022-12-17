#ifndef H_COMPILER_H
#define H_COMPILER_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>

#include "ast/ast.hpp"
#include "objects/objects.hpp"
#include "code/code.hpp"
//#include "evaluator/evaluator.hpp"
//#include "objects/environment.hpp"
#include "compiler/symbol_table.hpp"
#include "objects/builtins.hpp"

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

    struct CompilationScope{
        bytecode::Instructions instructions;
        EmittedInstruction lastInstruction;
        EmittedInstruction prevInstruction;
    };

    struct Compiler
    {
        std::vector<std::shared_ptr<objects::Object>> constants;
        std::shared_ptr<compiler::SymbolTable> symbolTable;

        std::vector<std::shared_ptr<CompilationScope>> scopes;
        int scopeIndex;

        Compiler(){
            symbolTable = compiler::NewSymbolTable();

            auto mainScope = std::make_shared<CompilationScope>();
            scopes.push_back(mainScope);
            scopeIndex = 0;
        }

        std::shared_ptr<objects::Error> Compile([[maybe_unused]] std::shared_ptr<ast::Node> node)
        {
            if(node->GetNodeType() == ast::NodeType::Program)
            {
                std::shared_ptr<ast::Program> program = std::dynamic_pointer_cast<ast::Program>(node);
                for(auto &stmt: program->v_pStatements)
                {
                    auto resultObj = Compile(stmt);
                    if(objects::isError(resultObj))
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
                    if(objects::isError(resultObj))
                    {
                        return resultObj;
                    }
                }
            }
            else if(node->GetNodeType() == ast::NodeType::ExpressionStatement)
            {
                std::shared_ptr<ast::ExpressionStatement> exprStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(node);

                auto resultObj = Compile(exprStmt->pExpression);
                if (objects::isError(resultObj))
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
                    if (objects::isError(resultObj))
                    {
                        return resultObj;
                    }

                    resultObj = Compile(infixObj->pLeft);
                    if (objects::isError(resultObj))
                    {
                        return resultObj;
                    }

                    
                    emit(bytecode::OpcodeType::OpGreaterThan, {});

                    return nullptr;
                }

                auto resultObj = Compile(infixObj->pLeft);
                if (objects::isError(resultObj))
                {
                    return resultObj;
                }

                resultObj = Compile(infixObj->pRight);
                if (objects::isError(resultObj))
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
                    return objects::newError("unknow operator: " + infixObj->Operator);
                }
            }
            else if(node->GetNodeType() == ast::NodeType::PrefixExpression)
            {
                std::shared_ptr<ast::PrefixExpression> prefixObj = std::dynamic_pointer_cast<ast::PrefixExpression>(node);
                auto resultObj = Compile(prefixObj->pRight);
                if (objects::isError(resultObj))
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
                    return objects::newError("unknow operator: " + prefixObj->Operator);
                }
            }
            else if(node->GetNodeType() == ast::NodeType::IfExpression)
            {
                std::shared_ptr<ast::IfExpression> ifObj = std::dynamic_pointer_cast<ast::IfExpression>(node);

                auto resultObj = Compile(ifObj->pCondition);
                if (objects::isError(resultObj))
                {
                    return resultObj;
                }

                // 预设一个偏移量方便后续回填
                auto jumpNotTruthyPos =  emit(bytecode::OpcodeType::OpJumpNotTruthy, {9999});

                resultObj = Compile(ifObj->pConsequence);
                if (objects::isError(resultObj))
                {
                    return resultObj;
                }

                if(lastInstructionIsPop())
                {
                    removeLastPop();
                }

                auto jumpPos = emit(bytecode::OpcodeType::OpJump, {9999});

                auto afterConsequencePos = scopes[scopeIndex]->instructions.size();
                changeOperand(jumpNotTruthyPos, afterConsequencePos);

                if(ifObj->pAlternative == nullptr)
                {
                    emit(bytecode::OpcodeType::OpNull, {});
                }
                else
                {
                    resultObj = Compile(ifObj->pAlternative);
                    if (objects::isError(resultObj))
                    {
                        return resultObj;
                    }

                    if (lastInstructionIsPop())
                    {
                        removeLastPop();
                    }
                }

                afterConsequencePos = scopes[scopeIndex]->instructions.size();
                changeOperand(jumpPos, afterConsequencePos);
            }
            else if(node->GetNodeType() == ast::NodeType::LetStatement)
            {
                std::shared_ptr<ast::LetStatement> letObj = std::dynamic_pointer_cast<ast::LetStatement>(node);

                auto symbol = symbolTable->Define(letObj->pName->Value);

                auto resultObj = Compile(letObj->pValue);
                if (objects::isError(resultObj))
                {
                    return resultObj;
                }

                // auto symbol = symbolTable->Define(letObj->pName->Value);

                if(symbol->Scope == compiler::SymbolScopeType::GlobalScope)
                {
                    emit(bytecode::OpcodeType::OpSetGlobal, {symbol->Index});
                } else {
                    emit(bytecode::OpcodeType::OpSetLocal, {symbol->Index});
                }
            }
            else if(node->GetNodeType() == ast::NodeType::Identifier)
            {
                std::shared_ptr<ast::Identifier> identObj = std::dynamic_pointer_cast<ast::Identifier>(node);
                auto symbol = symbolTable->Resolve(identObj->Value);
                if(symbol == nullptr)
                {
                    return objects::newError("undefined variable " + identObj->Value);
                }

                loadSymbol(symbol);
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
            else if(node->GetNodeType() == ast::NodeType::StringLiteral)
            {
                std::shared_ptr<ast::StringLiteral> stringLiteral = std::dynamic_pointer_cast<ast::StringLiteral>(node);
                auto strObj = std::make_shared<objects::String>(stringLiteral->Value);
                auto pos = addConstant(strObj);
                emit(bytecode::OpcodeType::OpConstant, {pos});
            }
            else if(node->GetNodeType() == ast::NodeType::ArrayLiteral)
            {
                std::shared_ptr<ast::ArrayLiteral> arrayLiteral = std::dynamic_pointer_cast<ast::ArrayLiteral>(node);
                for(auto &stmt: arrayLiteral->Elements)
                {
                    auto resultObj = Compile(stmt);
                    if (objects::isError(resultObj))
                    {
                        return resultObj;
                    }
                }

                emit(bytecode::OpcodeType::OpArray, {static_cast<int>(arrayLiteral->Elements.size())});
            }
            else if(node->GetNodeType() == ast::NodeType::HashLiteral)
            {
                std::shared_ptr<ast::HashLiteral> hashLiteral = std::dynamic_pointer_cast<ast::HashLiteral>(node);

                std::vector<std::shared_ptr<ast::Expression>> keys{};
                for(auto &pair: hashLiteral->Pairs)
                {
                    keys.push_back(pair.first);
                }

                std::sort(keys.begin(), keys.end(), [](const auto &lhs, const auto& rhs){ return lhs->String() < rhs->String(); });

                for(auto &key: keys)
                {
                    auto resultObj = Compile(key);
                    if (objects::isError(resultObj))
                    {
                        return resultObj;
                    }

                    resultObj = Compile(hashLiteral->Pairs[key]);
                    if (objects::isError(resultObj))
                    {
                        return resultObj;
                    }
                }  

                emit(bytecode::OpcodeType::OpHash, {2 * static_cast<int>(hashLiteral->Pairs.size())});              
            }
            else if(node->GetNodeType() == ast::NodeType::IndexExpression)
            {
                std::shared_ptr<ast::IndexExpression> indexObj = std::dynamic_pointer_cast<ast::IndexExpression>(node);

                auto resultObj = Compile(indexObj->Left);
                if (objects::isError(resultObj))
                {
                    return resultObj;
                }

                resultObj = Compile(indexObj->Index);
                if (objects::isError(resultObj))
                {
                    return resultObj;
                }

                emit(bytecode::OpcodeType::OpIndex);
            }
            else if(node->GetNodeType() == ast::NodeType::FunctionLiteral)
            {
                std::shared_ptr<ast::FunctionLiteral> funcObj = std::dynamic_pointer_cast<ast::FunctionLiteral>(node);

                enterScope();

                if(funcObj->Name != "")
                {
                    symbolTable->DefineFunctionName(funcObj->Name);
                }

                for(auto &args: funcObj->v_pParameters)
                {
                    symbolTable->Define(args->Value);
                }

                auto resultObj = Compile(funcObj->pBody);
                if (objects::isError(resultObj))
                {
                    return resultObj;
                }

                if (lastInstructionIs(bytecode::OpcodeType::OpPop))
                {
                    removeLastPopWithReturn();
                }

                if(!lastInstructionIs(bytecode::OpcodeType::OpReturnValue))
                {
                    emit(bytecode::OpcodeType::OpReturn);
                }

                auto freeSymbols = symbolTable->FreeSymbols;
                auto numLocals = symbolTable->numDefinitions;
                auto numParameters = funcObj->v_pParameters.size();
                auto ins = leaveScope();

                for(auto &sym: freeSymbols)
                {
                    loadSymbol(sym);
                }

                auto compiledFn = std::make_shared<objects::CompiledFunction>(ins, numLocals, numParameters);
                auto pos = addConstant(compiledFn);

                //emit(bytecode::OpcodeType::OpConstant, {pos});
                emit(bytecode::OpcodeType::OpClosure, {pos, static_cast<int>(freeSymbols.size())});
            }
            else if(node->GetNodeType() == ast::NodeType::ReturnStatement)
            {
                std::shared_ptr<ast::ReturnStatement> returnObj = std::dynamic_pointer_cast<ast::ReturnStatement>(node);

                auto resultObj = Compile(returnObj->pReturnValue);
                if (objects::isError(resultObj))
                {
                    return resultObj;
                }

                emit(bytecode::OpcodeType::OpReturnValue);
            }
            else if(node->GetNodeType() == ast::NodeType::CallExpression)
            {
                std::shared_ptr<ast::CallExpression> callObj = std::dynamic_pointer_cast<ast::CallExpression>(node);

                auto resultObj = Compile(callObj->pFunction);
                if (objects::isError(resultObj))
                {
                    return resultObj;
                }

                for(auto &args: callObj->pArguments)
                {
                    resultObj = Compile(args);
                    if (objects::isError(resultObj))
                    {
                        return resultObj;
                    }
                }

                int argsNum = callObj->pArguments.size();
                emit(bytecode::OpcodeType::OpCall, {argsNum});
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

        int emit(bytecode::OpcodeType op)
        {
            return emit(op, {});
        }

        void loadSymbol(std::shared_ptr<compiler::Symbol> symbol)
        {
            if (symbol->Scope == compiler::SymbolScopeType::GlobalScope)
            {
                emit(bytecode::OpcodeType::OpGetGlobal, {symbol->Index});
            }
            else if(symbol->Scope == compiler::SymbolScopeType::LocalScope)
            {
                emit(bytecode::OpcodeType::OpGetLocal, {symbol->Index});
            }
            else if(symbol->Scope == compiler::SymbolScopeType::BuiltinScope)
            {
                emit(bytecode::OpcodeType::OpGetBuiltin, {symbol->Index});
            }
            else if(symbol->Scope == compiler::SymbolScopeType::FreeScope)
            {
                emit(bytecode::OpcodeType::OpGetFree, {symbol->Index});
            }
            else if(symbol->Scope == compiler::SymbolScopeType::FunctionScope)
            {
                emit(bytecode::OpcodeType::OpCurrentClosure);
            }
        }

        int addInstruction(bytecode::Instructions ins)
        {
            auto instructions = currentInstructions();
            auto posNewInstruction = instructions.size();
            
            instructions.insert(instructions.end(), ins.begin(), ins.end());

            scopes[scopeIndex]->instructions = instructions;

            return posNewInstruction;
        }

        void setLastInstruction(bytecode::OpcodeType op, int pos)
        {
            scopes[scopeIndex]->prevInstruction = scopes[scopeIndex]->lastInstruction;
            scopes[scopeIndex]->lastInstruction = EmittedInstruction(op, pos);
        }

        bool lastInstructionIsPop()
        {
            return lastInstructionIs(bytecode::OpcodeType::OpPop);
        }

        bool lastInstructionIs(bytecode::OpcodeType op)
        {
            if(scopes[scopeIndex]->instructions.size() == 0)
            {
                return false;
            }

            return (scopes[scopeIndex]->lastInstruction.Opcode == op);
        }

        void removeLastPop()
        {
            auto instructions = currentInstructions();
            auto lastInstruction = scopes[scopeIndex]->lastInstruction;

            instructions.assign(instructions.begin(), instructions.begin() + lastInstruction.Position);

            scopes[scopeIndex]->instructions = instructions;
            scopes[scopeIndex]->lastInstruction = scopes[scopeIndex]->prevInstruction;
        }

        void removeLastPopWithReturn()
        {
            auto lastPos = scopes[scopeIndex]->lastInstruction.Position;
            replaceInstruction(lastPos, bytecode::Make(bytecode::OpcodeType::OpReturnValue));
            scopes[scopeIndex]->lastInstruction.Opcode = bytecode::OpcodeType::OpReturnValue;
        }

        void replaceInstruction(int pos, const bytecode::Instructions& newInstruction)
        {
            for (int i = 0, size = newInstruction.size(); i < size; i++)
            {
                scopes[scopeIndex]->instructions[pos + i] = newInstruction[i];
            }
        }

        void changeOperand(int opPos, int operand)
        {
            auto instructions = currentInstructions();
            bytecode::OpcodeType op = static_cast<bytecode::OpcodeType>(instructions[opPos]);
            auto newInstruction = bytecode::Make(op, {operand});

            replaceInstruction(opPos, newInstruction);
        }

        std::shared_ptr<ByteCode> Bytecode()
        {
            return std::make_shared<ByteCode>(scopes[scopeIndex]->instructions, constants);
        }

        bytecode::Instructions currentInstructions()
        {
            return scopes[scopeIndex]->instructions;
        }

        void enterScope()
        {
            auto scope = std::make_shared<CompilationScope>();
            scopes.push_back(scope);
            scopeIndex += 1;
            symbolTable = NewEnclosedSymbolTable(symbolTable);
        }

        bytecode::Instructions leaveScope()
        {
            auto ins = currentInstructions();
            scopes.pop_back();
            scopeIndex -= 1;
            symbolTable = symbolTable->Outer;

            return ins;
        }
    };

    std::shared_ptr<Compiler> New()
    {
        auto symbolTable = NewSymbolTable();

        int i = -1;
        for(auto &fn: objects::Builtins)
        {
            i += 1;
            symbolTable->DefineBuiltin(i, fn->Name);
        }

        auto compiler = std::make_shared<Compiler>();
        compiler->symbolTable = symbolTable;

        return compiler;
    }

    std::shared_ptr<Compiler> NewWithState(std::shared_ptr<compiler::SymbolTable> symbolTable,
                                           std::vector<std::shared_ptr<objects::Object>>& constants)
    {
        std::shared_ptr<Compiler> compiler = New();
        compiler->symbolTable = symbolTable;
        compiler->constants = constants;
        return compiler;
    }
}

#endif // H_COMPILER_H
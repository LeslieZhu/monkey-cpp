#ifndef H_COMPILER_H
#define H_COMPILER_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>

#include "ast/ast.hpp"
#include "objects/objects.hpp"
#include "code/code.hpp"

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

        std::shared_ptr<objects::Error> Compile([[maybe_unused]] std::unique_ptr<ast::Node> node)
        {
            return nullptr;
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
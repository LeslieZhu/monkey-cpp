#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <variant>

#include "code/code.hpp"
#include "compiler/compiler.hpp"
#include "compiler/symbol_table.hpp"

extern void printParserErrors(std::vector<std::string> errors);
extern void testIntegerObject(std::shared_ptr<objects::Object> obj, int64_t expected);
extern std::unique_ptr<ast::Node> TestHelper(const std::string& input);


TEST(testSymbolDefine, Basic)
{
    std::map<std::string, std::shared_ptr<compiler::Symbol>> expected{
        {"a", std::make_shared<compiler::Symbol>("a", compiler::SymbolScopeType::GlobalScope, 0)},
        {"b", std::make_shared<compiler::Symbol>("b", compiler::SymbolScopeType::GlobalScope, 1)},
    };

    auto global = compiler::NewSymbolTable();

    auto a = global->Define("a");
    EXPECT_EQ(*a, *(expected["a"]));

    auto b = global->Define("b");
    EXPECT_EQ(*b, *(expected["b"]));
}

TEST(testResolveGlobal, Basic)
{
    auto global = compiler::NewSymbolTable();
    global->Define("a");
    global->Define("b");

    std::vector<std::shared_ptr<compiler::Symbol>> expected{
        std::make_shared<compiler::Symbol>("a", compiler::SymbolScopeType::GlobalScope, 0),
        std::make_shared<compiler::Symbol>("b", compiler::SymbolScopeType::GlobalScope, 1),
    };

    for(auto &sym: expected)
    {
        auto result = global->Resolve(sym->Name);

        EXPECT_NE(result, nullptr);
        EXPECT_EQ(*result, *sym);
    }
}
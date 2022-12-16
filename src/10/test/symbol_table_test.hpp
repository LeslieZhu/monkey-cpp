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
        {"c", std::make_shared<compiler::Symbol>("c", compiler::SymbolScopeType::LocalScope, 0)},
        {"d", std::make_shared<compiler::Symbol>("d", compiler::SymbolScopeType::LocalScope, 1)},
        {"e", std::make_shared<compiler::Symbol>("e", compiler::SymbolScopeType::LocalScope, 0)},
        {"f", std::make_shared<compiler::Symbol>("f", compiler::SymbolScopeType::LocalScope, 1)},
    };

    auto global = compiler::NewSymbolTable();

    auto a = global->Define("a");
    EXPECT_EQ(*a, *(expected["a"]));

    auto b = global->Define("b");
    EXPECT_EQ(*b, *(expected["b"]));

    auto firstLocal = compiler::NewEnclosedSymbolTable(global);
    auto c = firstLocal->Define("c");
    auto d = firstLocal->Define("d");

    EXPECT_EQ(*c, *(expected["c"]));
    EXPECT_EQ(*d, *(expected["d"]));

    auto secondLocal = compiler::NewEnclosedSymbolTable(firstLocal);
    auto e = secondLocal->Define("e");
    auto f = secondLocal->Define("f");

    EXPECT_EQ(*e, *(expected["e"]));
    EXPECT_EQ(*f, *(expected["f"]));
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

TEST(testResolveLocal, Basic)
{
    auto global = compiler::NewSymbolTable();
    global->Define("a");
    global->Define("b");

    auto local = compiler::NewEnclosedSymbolTable(global);
    local->Define("c");
    local->Define("d");

    std::vector<std::shared_ptr<compiler::Symbol>> expected{
        std::make_shared<compiler::Symbol>("a", compiler::SymbolScopeType::GlobalScope, 0),
        std::make_shared<compiler::Symbol>("b", compiler::SymbolScopeType::GlobalScope, 1),
        std::make_shared<compiler::Symbol>("c", compiler::SymbolScopeType::LocalScope, 0),
        std::make_shared<compiler::Symbol>("d", compiler::SymbolScopeType::LocalScope, 1),
    };

    for(auto &sym: expected)
    {
        auto result = local->Resolve(sym->Name);

        EXPECT_NE(result, nullptr);
        EXPECT_EQ(*result, *sym);
    }
}

TEST(testResolveNestedLocal, Basic)
{
    auto global = compiler::NewSymbolTable();
    global->Define("a");
    global->Define("b");

    auto firstLocal = compiler::NewEnclosedSymbolTable(global);
    firstLocal->Define("c");
    firstLocal->Define("d");

    auto secondLocal = compiler::NewEnclosedSymbolTable(firstLocal);
    secondLocal->Define("e");
    secondLocal->Define("f");

    std::vector<std::shared_ptr<compiler::SymbolTable>> tests{
        firstLocal, 
        secondLocal
    };

    std::vector<std::vector<std::shared_ptr<compiler::Symbol>>> expected{
        {
            std::make_shared<compiler::Symbol>("a", compiler::SymbolScopeType::GlobalScope, 0),
            std::make_shared<compiler::Symbol>("b", compiler::SymbolScopeType::GlobalScope, 1),
            std::make_shared<compiler::Symbol>("c", compiler::SymbolScopeType::LocalScope, 0),
            std::make_shared<compiler::Symbol>("d", compiler::SymbolScopeType::LocalScope, 1),
        },
        {
            std::make_shared<compiler::Symbol>("a", compiler::SymbolScopeType::GlobalScope, 0),
            std::make_shared<compiler::Symbol>("b", compiler::SymbolScopeType::GlobalScope, 1),
            std::make_shared<compiler::Symbol>("e", compiler::SymbolScopeType::LocalScope, 0),
            std::make_shared<compiler::Symbol>("f", compiler::SymbolScopeType::LocalScope, 1),
        }
    };

    for (unsigned long i = 0; i < tests.size(); i++)
    {
        for (auto &sym : expected[i])
        {
            auto result = tests[i]->Resolve(sym->Name);

            EXPECT_NE(result, nullptr);
            EXPECT_EQ(*result, *sym);
        }
    }
}

TEST(testResolveBuiltins, Basic)
{
    auto global = compiler::NewSymbolTable();
    auto firstLocal = compiler::NewEnclosedSymbolTable(global);
    auto secondLocal = compiler::NewEnclosedSymbolTable(firstLocal);

    std::vector<std::shared_ptr<compiler::SymbolTable>> tests{
        global,
        firstLocal, 
        secondLocal
    };

    std::vector<std::shared_ptr<compiler::Symbol>> expected{
        std::make_shared<compiler::Symbol>("a", compiler::SymbolScopeType::BuiltinScope, 0),
        std::make_shared<compiler::Symbol>("c", compiler::SymbolScopeType::BuiltinScope, 1),
        std::make_shared<compiler::Symbol>("e", compiler::SymbolScopeType::BuiltinScope, 2),
        std::make_shared<compiler::Symbol>("f", compiler::SymbolScopeType::BuiltinScope, 3),
    };

    int i = -1;
    for(auto &sym: expected)
    {
        i += 1;
        global->DefineBuiltin(i, sym->Name);
    }

    for (unsigned long i = 0; i < tests.size(); i++)
    {
        for (auto &sym : expected)
        {
            auto result = tests[i]->Resolve(sym->Name);

            EXPECT_NE(result, nullptr);
            EXPECT_EQ(*result, *sym);
        }
    }
}
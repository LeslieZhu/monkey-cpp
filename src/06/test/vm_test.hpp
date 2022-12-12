#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <variant>

#include "lexer/lexer.hpp"
#include "ast/ast.hpp"
#include "objects/objects.hpp"
#include "parser/parser.hpp"
#include "vm/vm.hpp"

extern void printParserErrors(std::vector<std::string> errors);
extern void testIntegerObject(std::shared_ptr<objects::Object> obj, int64_t expected);
extern std::unique_ptr<ast::Node> TestHelper(const std::string& input);
extern void testBooleanObject(std::shared_ptr<objects::Object> obj, bool expected);

struct vmTestCases{
    std::string input;
    std::variant<int, bool> expected;
};

void testExpectedObject(std::variant<int, bool> expected, std::shared_ptr<objects::Object> actual)
{
    if(std::holds_alternative<int>(expected))
    {
        int val = std::get<int>(expected);
        testIntegerObject(actual, static_cast<int64_t>(val));
    }
    else if(std::holds_alternative<bool>(expected))
    {
        bool val = std::get<bool>(expected);
        testBooleanObject(actual, val);
    }
}

void runVmTests(std::vector<vmTestCases>& tests)
{
    for(auto &test: tests)
    {
        std::unique_ptr<ast::Node> astNode = TestHelper(test.input);
        std::shared_ptr<compiler::Compiler> compiler = compiler::New();
        
        auto resultObj = compiler->Compile(std::move(astNode));
        EXPECT_EQ(resultObj, nullptr);

        std::shared_ptr<compiler::ByteCode> bytecodeObj = compiler->Bytecode();

        auto vm = vm::New(bytecodeObj);
        auto vmresult = vm->Run();
        EXPECT_EQ(vmresult, nullptr);

        // auto stackElem = vm->StackTop();
        auto stackElem = vm->LastPoppedStackElem();
        testExpectedObject(test.expected, stackElem);
    }
}


TEST(testVMIntegerArithmetic, basicTest)
{
    std::vector<vmTestCases> tests{
        {"1", 1},
        {"2", 2},
        {"1 + 2", 3},
        {"1 - 2", -1},
        {"1 * 2", 2},
        {"4 / 2", 2},
        {"50 / 2 * 2 + 10 - 5", 55},
        {"5 + 5 + 5 + 5 - 10", 10},
        {"2 * 2 *2 * 2 * 2", 32},
        {"5 * 2 + 10", 20},
        {"5 + 2 * 10", 25},
        {"5 * (2 + 10)", 60}
        };

    runVmTests(tests);  
}

TEST(testVMBooleanExpression, basicTest)
{
    std::vector<vmTestCases> tests{
        {"true", true},
        {"false", false},
        };

    runVmTests(tests);  
}


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

struct vmTestCases{
    std::string input;
    std::variant<int> expected;
};

void testExpectedObject(std::variant<int> expected, std::shared_ptr<objects::Object> actual)
{
    if(std::holds_alternative<int>(expected))
    {
        int val = std::get<int>(expected);
        testIntegerObject(actual, static_cast<int64_t>(val));
    }
}


TEST(testVMIntegerArithmetic, basicTest)
{
    struct vmTestCases tests[]
    {
        {"1", 1},
        {"2", 2},
        {"1 + 2", 3}
    };

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

        auto stackElem = vm->StackTop();
        testExpectedObject(test.expected, stackElem);
    }
}


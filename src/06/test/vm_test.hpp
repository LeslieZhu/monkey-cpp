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
extern void testNullObject(std::shared_ptr<objects::Object> obj);

struct vmTestCases{
    std::string input;
    std::variant<int, bool, void*> expected;
};

void testExpectedObject(std::variant<int, bool, void*> expected, std::shared_ptr<objects::Object> actual)
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
    else
    {
        testNullObject(actual);
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
        {"5 * (2 + 10)", 60},
        {"-5", -5},
        {"-10", -10},
        {"-50 + 100 + -50", 0},
        {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
        };

    runVmTests(tests);  
}

TEST(testVMBooleanExpression, basicTest)
{
    std::vector<vmTestCases> tests{
        {"true", true},
        {"false", false},
        {"1 < 2", true},
        {"1 > 2", false},
        {"1 < 1", false},
        {"1 > 1", false},
        {"1 == 1", true},
        {"1 != 1", false},
        {"1 == 2", false},
        {"1 != 2", true},
        {"true == true", true},
        {"false == false", true},
        {"true == false", false},
        {"true != false", true},
        {"false != true", true},
        {"(1 < 2) == true", true},
        {"(1 < 2) == false", false},
        {"(1 > 2) == true", false},
        {"(1 > 2) == false", true},
        {"!true", false},
        {"!false", true},
        {"!5", false},
        {"!!true", true},
        {"!!false", false},
        {"!!5", true}
        };

    runVmTests(tests);  
}


TEST(testVMConditionals, basicTest)
{
    std::vector<vmTestCases> tests{
        {"if(true) { 10 }", 10},
        {"if(true) { 10 } else { 20 }", 10},
        {"if(false) { 10 } else { 20 }", 20},
        {"if(1) { 10 }", 10},
        {"if(1 < 2){ 10}", 10},
        {"if(1 < 2){ 10 } else { 20 }", 10},
        {"if(1 > 2){ 10 } else { 20 }", 20},
        {"if( 1 > 2) { 10 }", nullptr},
        {"if( false ){ 10 }", nullptr},
        {"if((if (false) { 10 })){ 10 } else { 20 }", 20}
        };

    runVmTests(tests);  
}

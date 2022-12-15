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
extern void testStringObject(std::shared_ptr<objects::Object> obj, std::string expected);

struct vmTestCases{
    std::string input;
    std::variant<int, bool, std::string, void*> expected;
};

void testExpectedObject(std::variant<int, bool, std::string, void*> expected, std::shared_ptr<objects::Object> actual)
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
    else if(std::holds_alternative<std::string>(expected))
    {
        std::string val = std::get<std::string>(expected);
        if(std::shared_ptr<objects::Array> arrObj = std::dynamic_pointer_cast<objects::Array>(actual); arrObj != nullptr)
        {
            EXPECT_NE(arrObj, nullptr);
            EXPECT_STREQ(arrObj->Inspect().c_str(), val.c_str());
        } 
        else if(std::shared_ptr<objects::Hash> hashObj = std::dynamic_pointer_cast<objects::Hash>(actual); hashObj != nullptr)
        {
            EXPECT_NE(hashObj, nullptr);
            EXPECT_STREQ(hashObj->Inspect().c_str(), val.c_str());
        }
        else {
            testStringObject(actual, val);
        }
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

TEST(testVMGlobalLetStatements, basicTest)
{
    std::vector<vmTestCases> tests{
        {"let one = 1; one", 1},
        {"let one = 1; let two = 2; one + two;", 3},
        {"let one = 1; let two = one + one; one + two;", 3}
        };

    runVmTests(tests);  
}

TEST(testVMStringExpression, basicTest)
{
    std::vector<vmTestCases> tests{
        {"\"monkey\"", "monkey"s},
        {"\"mon\" + \"key\";", "monkey"s},
        {"\"mon\" + \"key\" + \"banana\";", "monkeybanana"s}
        };

    runVmTests(tests);  
}

TEST(testVMArrayLiterals, basicTest)
{
    std::vector<vmTestCases> tests{
        {"[]", "[]"s},
        {"[1, 2, 3]", "[1, 2, 3]"s},
        {"[1+2, 3*4, 5+6]", "[3, 12, 11]"s}
        };

    runVmTests(tests);  
}

TEST(testVMHashLiterals, basicTest)
{
    std::vector<vmTestCases> tests{
        {"{}", "{}"s},
        {"{1: 2, 2: 3}", "{1: 2, 2: 3}"s},
        {"{1 + 1: 2 * 2, 3 + 3: 4 * 4}", "{2: 4, 6: 16}"s}
        };

    runVmTests(tests);  
}

TEST(testVMIndexExpressions, basicTest)
{
    std::vector<vmTestCases> tests{
        {"[1,2,3][1]", 2},
        {"[1,2,3][0 + 2]", 3},
        {"[[1,1,1]][0][0]", 1},
        {"[][0]", nullptr},
        {"[1,2,3][99]", nullptr},
        {"[1][-1]", nullptr},
        {"{1: 1, 2:2}[1]", 1},
        {"{1: 1, 2:2}[2]", 2},
        {"{1: 1}[0]", nullptr},
        {"{}[0]", nullptr},
        };

    runVmTests(tests);  
}

TEST(testVMCallingFunctionWIthoutArguments, basicTest)
{
    std::vector<vmTestCases> tests{
        {
            R""(
                let fivePlusTen = fn(){ 5 + 10; }
                fivePlusTen();
            )"",
            15
        },
        {
            R""(
                let one = fn(){ 1; };
                let two = fn(){ 2; };
                one() + two();
            )"",
            3
        },
        {
            R""(
                let a = fn(){ 1; };
                let b = fn(){ a() + 1 };
                let c = fn(){ b() + 1 };
                c();
            )"",
            3
        },
        {
            R""(
                let earlyExit = fn(){ return 99; 100; };
                earlyExit();
            )"",
            99
        },
        {
            R""(
                let earlyExit = fn(){ return 99; return 100; }
                earlyExit();
            )"",
            99
        },
        {
            R""(
                let noReturn = fn() { };
                noReturn();
            )"",
            nullptr
        },
        {
            R""(
                let noReturn = fn() { };
                let noReturnTwo = fn() { noReturn(); };
                noReturn();
                noReturnTwo();
            )"",
            nullptr
        },
        {
            R""(
                let returnOne = fn(){ 1; };
                let returnOneReturner = fn(){ returnOne; };
                returnOneReturner()();
            )"",
            1
        }
    };

    runVmTests(tests);  
}

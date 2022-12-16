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
    std::variant<int, bool, std::string, std::shared_ptr<objects::Object>, void*> expected;
};

void testExpectedObject(std::variant<int, bool, std::string, std::shared_ptr<objects::Object>, void*> expected, std::shared_ptr<objects::Object> actual)
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
        else if(std::shared_ptr<objects::Error> errorObj = std::dynamic_pointer_cast<objects::Error>(actual); errorObj != nullptr)
        {
            EXPECT_NE(errorObj, nullptr);
            EXPECT_STREQ(errorObj->Message.c_str(), val.c_str());
        }
        else {
            testStringObject(actual, val);
        }
    }
    else if(std::holds_alternative<std::shared_ptr<objects::Object>>(expected))
    {
        auto obj = std::get<std::shared_ptr<objects::Object>>(expected);
        if(std::shared_ptr<objects::Error> errorObj = std::dynamic_pointer_cast<objects::Error>(obj); errorObj != nullptr)
        {
            testExpectedObject(errorObj->Message, actual);
        } 
        else if(std::shared_ptr<objects::Array> arrayObj = std::dynamic_pointer_cast<objects::Array>(obj); arrayObj != nullptr)
        {
            std::shared_ptr<objects::Array> actualArray = std::dynamic_pointer_cast<objects::Array>(actual);
            EXPECT_NE(actualArray, nullptr);
            EXPECT_STREQ(actualArray->Inspect().c_str(), arrayObj->Inspect().c_str());
        }
        else {
            testNullObject(actual);
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

TEST(testVMCallingFunctionWithoutArguments, basicTest)
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
        },
        {
            R""(
                let one = fn(){ let one = 1; one }
                one();
            )"",
            1
        },
        {
            R""(
                let oneAndTwo = fn(){ let one = 1; let two = 2; one + two; }
                oneAndTwo();
            )"",
            3
        },
        {
            R""(
                let oneAndTwo = fn(){ let one=1; let two=2; one + two; }
                let threeAndFour = fn(){ let three = 3; let four = 4; three + four; }
                oneAndTwo() + threeAndFour();
            )"",
            10
        },
        {
            R""(
                let firstFoobar = fn(){ let foobar = 50; foobar; }
                let secondFoobar = fn(){ let foobar = 100; foobar; }
                firstFoobar() + secondFoobar();
            )"",
            150
        },
        {
            R""(
                let globalSeed = 50;
                let minusOne = fn(){
                    let num = 1;
                    globalSeed - num;
                }

                let minusTwo = fn(){
                    let num = 2;
                    globalSeed - num;
                }

                minusOne() + minusTwo();
            )"",
            97
        }
    };

    runVmTests(tests);  
}


TEST(testVMCallingFirstFunctions, basicTest)
{
    std::vector<vmTestCases> tests{
        {
            R""(
                let returnsOneReturner = fn(){
                    let returnsOne = fn(){ 1 ; };
                    returnsOne;
                }

                returnsOneReturner()();
            )"",
            1
        },
    };

    runVmTests(tests);  
}

TEST(testVMCallingFunctionWithArguments, basicTest)
{
    std::vector<vmTestCases> tests{
        {
            R""(
                let identify = fn(a) { a; }
                identify(4);
            )"",
            4
        },
        {
            R""(
                let sum = fn(a, b){ a + b; }
                sum(1,2);
            )"",
            3
        },
        {
            R""(
                let sum = fn(a, b){
                    let c = a + b;
                    c;
                }

                sum(1,2);
            )"",
            3
        },
        {
            R""(
                let sum = fn(a, b) {
                    let c = a + b;
                    c;
                }

                sum(1, 2) + sum(3, 4)
            )"",
            10
        },
        {
            R""(
                let sum = fn(a, b){
                    let c = a + b;
                    c;
                }

                let outer = fn(){
                    sum(1, 2) + sum(3, 4);
                }

                outer();
            )"",
            10
        }
    };

    runVmTests(tests);  
}


TEST(testVMCallingFunctionWithWrongArguments, basicTest)
{
    struct testInput
    {
        std::string input;
        std::string expected;
    };

    std::vector<testInput> tests{
        {
            R""(
                fn(){ 1; }(1)
            )"",
            "wrong number of arguments: want=0, got=1"
        },
        {
            R""(
                fn(a){ a; }()
            )"",
            "wrong number of arguments: want=1, got=0"
        },
        {
            R""(
                fn(a, b){ a + b; }(1)
            )"",
            "wrong number of arguments: want=2, got=1"
        }
    };

    for (const auto &test : tests)
    {
        std::unique_ptr<ast::Node> astNode = TestHelper(test.input);
        std::shared_ptr<compiler::Compiler> compiler = compiler::New();
        
        auto resultObj = compiler->Compile(std::move(astNode));
        EXPECT_EQ(resultObj, nullptr);

        std::shared_ptr<compiler::ByteCode> bytecodeObj = compiler->Bytecode();

        auto vm = vm::New(bytecodeObj);
        auto vmresult = vm->Run();
        EXPECT_NE(vmresult, nullptr);

        std::shared_ptr<objects::Error> errorObj = std::dynamic_pointer_cast<objects::Error>(vmresult);

        EXPECT_NE(errorObj, nullptr);
        EXPECT_STREQ(errorObj->Message.c_str(), test.expected.c_str());
    }
} 


TEST(testVMCallingBuiltinFunction, basicTest)
{
    auto int1 = std::make_shared<objects::Integer>(1);
    auto int2 = std::make_shared<objects::Integer>(2);
    auto int3 = std::make_shared<objects::Integer>(3);

    std::vector<std::shared_ptr<objects::Object>> Elements1 = {int1};
    std::vector<std::shared_ptr<objects::Object>> Elements2 = {int2, int3};

    auto array1 = std::make_shared<objects::Array>(Elements1);
    auto array2 = std::make_shared<objects::Array>(Elements2);

    std::vector<vmTestCases> tests{
        {
            R""(
                len("")
            )"",
            0
        },
        {
            R""(
                len("four")
            )"",
            4
        },
        {
            R""(
                len("hello world")
            )"",
            11
        },
        {
            R""(
                last([1, 2, 3])
            )"",
            3
        },
        {
            R""(
                len([1, 2, 3])
            )"",
            3
        },
        {
            R""(
                first([1, 2, 3])
            )"",
            1
        },
        {
            R""(
                len([])
            )"",
            0
        },
        {
            "len(1)",
            objects::newError("argument to `len` not supported, got INTEGER")
        },
        {
            R""(
                len("one", "two")
            )"",
            objects::newError("wrong number of arguments. got=2, want=1")
        },
        {
            R""(
                first(1)
            )"",
            objects::newError("argument to `first` must be ARRAY, got INTEGER")
        },
        {
            R""(
                last(1)
            )"",
            objects::newError("argument to `last` must be ARRAY, got INTEGER")
        },
        {
            R""(
                push(1, 1)
            )"",
            objects::newError("argument to `push` must be ARRAY, got INTEGER")
        },
        {
            R""(
                puts("hello", "world!")
            )"",
            objects::NULL_OBJ
        },
        {
            R""(
                first([])
            )"",
            objects::NULL_OBJ
        },
        {
            R""(
                last([])
            )"",
            objects::NULL_OBJ
        },
        {
            R""(
                rest([])
            )"",
            objects::NULL_OBJ
        },
        {
            R""(
                rest([1, 2, 3])
            )"",
            array2
        },
        {
            R""(
                rest([])
            )"",
            objects::NULL_OBJ
        },
        {
            R""(
                push([], 1)
            )"",
            array1
        }
    };

    runVmTests(tests);
} 

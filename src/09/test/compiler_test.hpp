#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <variant>

#include "code/code.hpp"
#include "compiler/compiler.hpp"

extern void printParserErrors(std::vector<std::string> errors);
extern void testIntegerObject(std::shared_ptr<objects::Object> obj, int64_t expected);
extern void testStringObject(std::shared_ptr<objects::Object> obj, std::string expected);

std::unique_ptr<ast::Node> TestHelper(const std::string& input)
{
    std::unique_ptr<lexer::Lexer> pLexer = lexer::New(input);
    std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
    std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
    printParserErrors(pParser->Errors());

    std::unique_ptr<ast::Node> astNode(reinterpret_cast<ast::Node *>(pProgram.release()));

    return astNode;
}

bytecode::Instructions concatInstructions(std::vector<bytecode::Instructions>& s)
{
    bytecode::Instructions out{};

    for(auto &vins: s)
    {
        out.insert(out.end(), vins.begin(), vins.end());
    }

    return out;
}

void testInstructions(std::vector<bytecode::Instructions>& expected, bytecode::Instructions& actual)
{
    auto concated = concatInstructions(expected);
    EXPECT_EQ(actual.size(), concated.size());

    for(unsigned long i=0; i < concated.size(); i++)
    {
        EXPECT_EQ(actual[i], concated[i]);
    }
}

void testConstans(std::vector<std::variant<int, std::string, std::vector<bytecode::Instructions>>> expected,
                  std::vector<std::shared_ptr<objects::Object>> actual)
{
    EXPECT_EQ(expected.size(), actual.size());

    int i = 0;
    for(auto &constant: expected)
    {
        if(std::holds_alternative<int>(constant))
        {
            int64_t val = static_cast<int64_t>(std::get<int>(constant));
            testIntegerObject(actual[i], val);
        }
        else if(std::holds_alternative<std::string>(constant))
        {
            std::string val = std::get<std::string>(constant);
            testStringObject(actual[i], val);
        }
        else if(std::holds_alternative<std::vector<bytecode::Instructions>>(constant))
        {
            std::vector<bytecode::Instructions> ins = std::get<std::vector<bytecode::Instructions>>(constant);

            std::shared_ptr<objects::CompiledFunction> funcObj = std::dynamic_pointer_cast<objects::CompiledFunction>(actual[i]);
            EXPECT_NE(funcObj, nullptr);

            testInstructions(ins, funcObj->Instructions);
        }

        i += 1;
    }
}

struct CompilerTestCase{
    std::string input;
    std::vector<std::variant<int, std::string, std::vector<bytecode::Instructions>>> expectedConstants;
    std::vector<bytecode::Instructions> expectedInstructions;
};

void runCompilerTests(std::vector<CompilerTestCase>& tests)
{
    for(auto &test: tests)
    {
        std::unique_ptr<ast::Node> astNode = TestHelper(test.input);
        std::shared_ptr<compiler::Compiler> compiler = compiler::New();
        
        auto resultObj = compiler->Compile(std::move(astNode));

        EXPECT_EQ(resultObj, nullptr);

        std::shared_ptr<compiler::ByteCode> bytecodeObj = compiler->Bytecode();
        
        testInstructions(test.expectedInstructions, bytecodeObj->Instructions);
        testConstans(test.expectedConstants, bytecodeObj->Constants);
    }
}


TEST(TestIntegerArithmetic, BasicAssertions)
{
    struct CompilerTestCase  tests[]
    {
        {
            "1+2",
            {1,2},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpAdd, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
        {
            "1;2",
            {1,2},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
        {
            "1-2",
            {1,2},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpSub, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
        {
            "1*2",
            {1,2},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpMul, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
        {
            "1 / 2",
            {1,2},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpDiv, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
        {
            "2 / 1",
            {2,1},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpDiv, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
        {
            "-1",
            {1},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpMinus, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
    };

    for(auto &test: tests)
    {
        std::unique_ptr<ast::Node> astNode = TestHelper(test.input);
        std::shared_ptr<compiler::Compiler> compiler = compiler::New();
        
        auto resultObj = compiler->Compile(std::move(astNode));

        EXPECT_EQ(resultObj, nullptr);

        std::shared_ptr<compiler::ByteCode> bytecodeObj = compiler->Bytecode();
        
        testInstructions(test.expectedInstructions, bytecodeObj->Instructions);
        testConstans(test.expectedConstants, bytecodeObj->Constants);
    }
}

TEST(TestCompileBooleanExpression, BasicAssertions)
{
    struct CompilerTestCase  tests[]
    {
        {
            "true",
            {},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpTrue, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
        {
            "false",
            {},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpFalse, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
        {
            "1 > 2",
            {1, 2},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpGreaterThan, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
        {
            "1 < 2",
            {2, 1},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpGreaterThan, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
        {
            "1 == 2",
            {1, 2},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpEqual, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
        {
            "1 != 2",
            {1, 2},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpNotEqual, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
        {
            "true == false",
            {},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpTrue, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpFalse, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpEqual, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
        {
            "true != false",
            {},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpTrue, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpFalse, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpNotEqual, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
        {
            "!true",
            {},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpTrue, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpBang, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                }
            }
        },
    };

    for(auto &test: tests)
    {
        std::unique_ptr<ast::Node> astNode = TestHelper(test.input);
        std::shared_ptr<compiler::Compiler> compiler = compiler::New();
        
        auto resultObj = compiler->Compile(std::move(astNode));

        EXPECT_EQ(resultObj, nullptr);

        std::shared_ptr<compiler::ByteCode> bytecodeObj = compiler->Bytecode();
        
        testInstructions(test.expectedInstructions, bytecodeObj->Instructions);
        testConstans(test.expectedConstants, bytecodeObj->Constants);
    }
}


TEST(TestCompileIfExpression, BasicAssertions)
{
    struct CompilerTestCase  tests[]
    {
        {
            "if( true ){ 10 }; 3333;",
            {10, 3333},
            {
                {
                    // 0000
                    bytecode::Make(bytecode::OpcodeType::OpTrue, {})
                },
                {
                    // 0001
                    bytecode::Make(bytecode::OpcodeType::OpJumpNotTruthy, {10})
                },
                {
                    // 0004
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    // 0007
                    bytecode::Make(bytecode::OpcodeType::OpJump, {11})
                },
                {
                    // 0010
                    bytecode::Make(bytecode::OpcodeType::OpNull, {})
                },
                {
                    // 0011
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
                {
                    // 0012
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    // 0015
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
            }
        },
        {
            "if( true ){ 10 } else { 20 }; 3333;",
            {10, 20, 3333},
            {
                {
                    // 0000
                    bytecode::Make(bytecode::OpcodeType::OpTrue, {})
                },
                {
                    // 0001
                    bytecode::Make(bytecode::OpcodeType::OpJumpNotTruthy, {10})
                },
                {
                    // 0004
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    // 0007
                    bytecode::Make(bytecode::OpcodeType::OpJump, {13})
                },
                {
                    // 0010
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    // 0013
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
                {
                    // 0014
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {2})
                },
                {
                    // 0017
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
            }
        },
    };

    for(auto &test: tests)
    {
        std::unique_ptr<ast::Node> astNode = TestHelper(test.input);
        std::shared_ptr<compiler::Compiler> compiler = compiler::New();
        
        auto resultObj = compiler->Compile(std::move(astNode));

        EXPECT_EQ(resultObj, nullptr);

        std::shared_ptr<compiler::ByteCode> bytecodeObj = compiler->Bytecode();
        
        testInstructions(test.expectedInstructions, bytecodeObj->Instructions);
        testConstans(test.expectedConstants, bytecodeObj->Constants);
    }
}

TEST(TestCompileGlobalStatements, BasicAssertions)
{
    std::vector<CompilerTestCase>  tests
    {
        {
            "let one = 1; let two = 2;",
            {1,2},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpSetGlobal, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpSetGlobal, {1})
                },
            }
        },
        {
            "let one = 1; one;",
            {1},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpSetGlobal, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpGetGlobal, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
            }
        },
        {
            "let one = 1; let two = one; two;",
            {1},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpSetGlobal, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpGetGlobal, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpSetGlobal, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpGetGlobal, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
            }
        },
    };

    runCompilerTests(tests);
}

TEST(TestCompileStringExpression, BasicAssertions)
{
    std::vector<CompilerTestCase>  tests
    {
        {
            R""(
                "monkey"
            )"",
            {"monkey"},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
            }
        },
        {
            R""(
                "mon" + "key"
            )"",
            {"mon","key"},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpAdd, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
            }
        },
    };

    runCompilerTests(tests);
}

TEST(TestCompileArrayLiterals, BasicAssertions)
{
    std::vector<CompilerTestCase>  tests
    {
        {
            "[]",
            {},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpArray, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
            }
        },
        {
            "[1, 2, 3]",
            {1,2,3},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {2})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpArray, {3})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
            }
        },
        {
            "[1+2, 3-4, 5*6]",
            {1,2,3,4,5,6},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpAdd, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {2})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {3})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpSub, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {4})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {5})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpMul, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpArray, {3})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
            }
        },
    };

    runCompilerTests(tests);
}


TEST(TestCompileHashLiterals, BasicAssertions)
{
    std::vector<CompilerTestCase>  tests
    {
        {
            "{}",
            {},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpHash, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
            }
        },
        {
            "{1:2, 3:4, 5:6}",
            {1,2,3,4,5,6},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {2})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {3})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {4})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {5})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpHash, {6})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
            }
        },
        {
            "{1: 2+3, 4:5*6}",
            {1,2,3,4,5,6},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {2})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpAdd, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {3})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {4})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {5})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpMul, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpHash, {4})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
            }
        },
    };

    runCompilerTests(tests);
}


TEST(TestCompileIndexExpressions, BasicAssertions)
{
    std::vector<CompilerTestCase>  tests
    {
        {
            "[1, 2, 3][1 + 1]",
            {1, 2, 3, 1, 1},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {2})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpArray, {3})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {3})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {4})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpAdd, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpIndex, {})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
            }
        },
        {
            "{1: 2}[2 - 1]",
            {1,2,2,1},
            {
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {0})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {1})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpHash, {2})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {2})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpConstant, {3})
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpSub)
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpIndex)
                },
                {
                    bytecode::Make(bytecode::OpcodeType::OpPop, {})
                },
            }
        },
    };

    runCompilerTests(tests);
}


TEST(TestCompileFunctions, BasicAssertions)
{
    std::vector<std::vector<bytecode::Instructions>> ins{
        {
            {bytecode::Make(bytecode::OpcodeType::OpConstant, {0})},
            {bytecode::Make(bytecode::OpcodeType::OpConstant, {1})},
            {bytecode::Make(bytecode::OpcodeType::OpAdd)},
            {bytecode::Make(bytecode::OpcodeType::OpReturnValue)},
        },
        {
            {bytecode::Make(bytecode::OpcodeType::OpConstant, {0})},
            {bytecode::Make(bytecode::OpcodeType::OpPop)},
            {bytecode::Make(bytecode::OpcodeType::OpConstant, {1})},
            {bytecode::Make(bytecode::OpcodeType::OpReturnValue)},
        },
        {
            {bytecode::Make(bytecode::OpcodeType::OpReturn)},
        },
        {
            {bytecode::Make(bytecode::OpcodeType::OpConstant, {0})},
            {bytecode::Make(bytecode::OpcodeType::OpReturnValue)},
        },
        {
            {bytecode::Make(bytecode::OpcodeType::OpConstant, {0})},
            {bytecode::Make(bytecode::OpcodeType::OpReturnValue)},
        },
        {
            {bytecode::Make(bytecode::OpcodeType::OpReturn)},
        },
        {
            {bytecode::Make(bytecode::OpcodeType::OpGetLocal, {0})},
            {bytecode::Make(bytecode::OpcodeType::OpReturnValue)},
        },
        {
            {bytecode::Make(bytecode::OpcodeType::OpGetLocal, {0})},
            {bytecode::Make(bytecode::OpcodeType::OpPop)},
            {bytecode::Make(bytecode::OpcodeType::OpGetLocal, {1})},
            {bytecode::Make(bytecode::OpcodeType::OpPop)},
            {bytecode::Make(bytecode::OpcodeType::OpGetLocal, {2})},
            {bytecode::Make(bytecode::OpcodeType::OpReturnValue)},
        }
    };

    std::vector<CompilerTestCase> tests
    {
        {
            "fn(){ return 5 + 10}",
            {
                5, 
                10, 
                ins[0]
            },
            {
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {2})},
                {bytecode::Make(bytecode::OpcodeType::OpPop)},
            }
        },
        {
            "fn(){ 5 + 10}",
            {
                5, 
                10, 
                ins[0]
            },
            {
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {2})},
                {bytecode::Make(bytecode::OpcodeType::OpPop)},
            }
        },
        {
            "fn(){ 1; 2}",
            {
                1, 
                2, 
                ins[1]
            },
            {
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {2})},
                {bytecode::Make(bytecode::OpcodeType::OpPop)},
            }
        },
        {
            "fn(){}",
            {
                ins[2]
            },
            {
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpPop)},
            }
        },
        {
            "fn(){ 24 }()",
            {
                24,
                ins[3]
            },
            {
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {1})},
                {bytecode::Make(bytecode::OpcodeType::OpCall, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpPop)},
            }
        },
        {
            "let noArg = fn(){ 24 }; noArg();",
            {
                24,
                ins[4]
            },
            {
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {1})},
                {bytecode::Make(bytecode::OpcodeType::OpSetGlobal, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpGetGlobal, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpCall, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpPop)},
            }
        },
        {
            "let oneAgr = fn(a) { }; oneAgr(24);",
            {
                ins[5],
                24
            },
            {
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpSetGlobal, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpGetGlobal, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {1})},
                {bytecode::Make(bytecode::OpcodeType::OpCall, {1})},
                {bytecode::Make(bytecode::OpcodeType::OpPop)},
            }
        },
        {
            "let manyArg = fn(a, b, c){ }; manyArg(24, 25, 26);",
            {
                ins[5],
                24,
                25,
                26
            },
            {
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpSetGlobal, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpGetGlobal, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {1})},
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {2})},
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {3})},
                {bytecode::Make(bytecode::OpcodeType::OpCall, {3})},
                {bytecode::Make(bytecode::OpcodeType::OpPop)},
            }
        },
        {
            "let oneArg = fn(a) { a }; oneArg(24);",
            {
                ins[6],
                24
            },
            {
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpSetGlobal, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpGetGlobal, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {1})},
                {bytecode::Make(bytecode::OpcodeType::OpCall, {1})},
                {bytecode::Make(bytecode::OpcodeType::OpPop)},
            }
        },
        {
            "let manyArg = fn(a, b, c){ a; b; c; }; manyArg(24, 25, 26);",
            {
                ins[7],
                24,
                25,
                26
            },
            {
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpSetGlobal, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpGetGlobal, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {1})},
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {2})},
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {3})},
                {bytecode::Make(bytecode::OpcodeType::OpCall, {3})},
                {bytecode::Make(bytecode::OpcodeType::OpPop)},
            }
        },
    };

    runCompilerTests(tests);
} 


TEST(testCompilerScopes, basic)
{
    std::shared_ptr<compiler::Compiler> compiler = compiler::New();

    EXPECT_EQ(compiler->scopeIndex, 0);

    auto globalSymbolTable = compiler->symbolTable;

    compiler->emit(bytecode::OpcodeType::OpMul);

    compiler->enterScope();

    EXPECT_EQ(compiler->scopeIndex, 1);

    compiler->emit(bytecode::OpcodeType::OpSub);

    EXPECT_EQ(compiler->scopes[compiler->scopeIndex]->instructions.size(), 1u);

    auto last = compiler->scopes[compiler->scopeIndex]->lastInstruction;
    EXPECT_EQ(last.Opcode, bytecode::OpcodeType::OpSub);

    EXPECT_EQ(compiler->symbolTable->Outer, globalSymbolTable);

    compiler->leaveScope();

    EXPECT_EQ(compiler->scopeIndex, 0);

    EXPECT_EQ(compiler->symbolTable, globalSymbolTable);
    EXPECT_EQ(compiler->symbolTable->Outer, nullptr);

    compiler->emit(bytecode::OpcodeType::OpAdd);
    EXPECT_EQ(compiler->scopes[compiler->scopeIndex]->instructions.size(), 2u);

    last = compiler->scopes[compiler->scopeIndex]->lastInstruction;
    EXPECT_EQ(last.Opcode, bytecode::OpcodeType::OpAdd);

    auto previous = compiler->scopes[compiler->scopeIndex]->prevInstruction;
    EXPECT_EQ(previous.Opcode, bytecode::OpcodeType::OpMul);
}

TEST(testCompilerLetStatementScope, basic)
{
    std::vector<std::vector<bytecode::Instructions>> ins{
        {
            {bytecode::Make(bytecode::OpcodeType::OpGetGlobal, {0})},
            {bytecode::Make(bytecode::OpcodeType::OpReturnValue)},
        },
        {
            {bytecode::Make(bytecode::OpcodeType::OpConstant, {0})},
            {bytecode::Make(bytecode::OpcodeType::OpSetLocal, {0})},
            {bytecode::Make(bytecode::OpcodeType::OpGetLocal, {0})},
            {bytecode::Make(bytecode::OpcodeType::OpReturnValue)},
        },
        {
            {bytecode::Make(bytecode::OpcodeType::OpConstant, {0})},
            {bytecode::Make(bytecode::OpcodeType::OpSetLocal, {0})},
            {bytecode::Make(bytecode::OpcodeType::OpConstant, {1})},
            {bytecode::Make(bytecode::OpcodeType::OpSetLocal, {1})},
            {bytecode::Make(bytecode::OpcodeType::OpGetLocal, {0})},
            {bytecode::Make(bytecode::OpcodeType::OpGetLocal, {1})},
            {bytecode::Make(bytecode::OpcodeType::OpAdd)},
            {bytecode::Make(bytecode::OpcodeType::OpReturnValue)},
        },
    };

    std::vector<CompilerTestCase> tests
    {
        {
            "let num = 55; fn(){ num }",
            {
                55,  
                ins[0]
            },
            {
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpSetGlobal, {0})},
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {1})},
                {bytecode::Make(bytecode::OpcodeType::OpPop)},
            }
        },
        {
            "fn(){ let num = 55; num }",
            {
                55, 
                ins[1]
            },
            {
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {1})},
                {bytecode::Make(bytecode::OpcodeType::OpPop)},
            }
        },
        {
            "fn(){ let a = 55; let b = 77; a + b",
            {
                55, 
                77, 
                ins[2]
            },
            {
                {bytecode::Make(bytecode::OpcodeType::OpConstant, {2})},
                {bytecode::Make(bytecode::OpcodeType::OpPop)},
            }
        },
    };

    runCompilerTests(tests);
} 
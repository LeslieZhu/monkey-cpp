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

std::unique_ptr<ast::Node> TestHelper(const std::string& input)
{
    std::unique_ptr<lexer::Lexer> pLexer = lexer::New(input);
    std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
    std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
    printParserErrors(pParser->Errors());

    std::unique_ptr<ast::Node> astNode(reinterpret_cast<ast::Node *>(pProgram.release()));

    return astNode;
}

void testConstans(std::vector<std::variant<int>> expected, std::vector<std::shared_ptr<objects::Object>> actual)
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

        i += 1;
    }
}

bytecode::Instructions concatInstructions(std::vector<bytecode::Instructions>& s)
{
    bytecode::Instructions out{};

    for(auto &vins: s)
    {
        for(auto &ins: vins)
        {
            out.push_back(ins);
        }
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

struct CompilerTestCase{
    std::string input;
    std::vector<std::variant<int>> expectedConstants;
    std::vector<bytecode::Instructions> expectedInstructions;
};


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
                }
            }
        }
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

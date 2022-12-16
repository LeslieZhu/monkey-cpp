#include <gtest/gtest.h>

#include <vector>
#include <memory>
#include <string>
#include <variant>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "objects/objects.hpp"
#include "objects/environment.hpp"
#include "evaluator/evaluator.hpp"

std::shared_ptr<objects::Object> testEval(const std::string &input)
{
    auto env = objects::NewEnvironment();
    std::unique_ptr<lexer::Lexer> pLexer = lexer::New(input);
    std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
    std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};

    std::unique_ptr<ast::Node> astNode(reinterpret_cast<ast::Node *>(pProgram.release()));
    return evaluator::Eval(std::move(astNode), env);
}

void testIntegerObject(std::shared_ptr<objects::Object> obj, int64_t expected)
{
    std::shared_ptr<objects::Integer> result = std::dynamic_pointer_cast<objects::Integer>(obj);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->Value, expected);
}

void testStringObject(std::shared_ptr<objects::Object> obj, std::string expected)
{
    std::shared_ptr<objects::String> result = std::dynamic_pointer_cast<objects::String>(obj);

    EXPECT_NE(result, nullptr);
    EXPECT_STREQ(result->Value.c_str(), expected.c_str());
}

void testBooleanObject(std::shared_ptr<objects::Object> obj, bool expected)
{
    std::shared_ptr<objects::Boolean> result = std::dynamic_pointer_cast<objects::Boolean>(obj);
    EXPECT_NE(result, nullptr);

    if (expected)
    {
        EXPECT_TRUE(result->Value);
        EXPECT_STREQ(result->Inspect().c_str(), "true");
    }
    else
    {
        EXPECT_FALSE(result->Value);
        EXPECT_STREQ(result->Inspect().c_str(), "false");
    }
}

void testNullObject(std::shared_ptr<objects::Object> obj)
{
    std::shared_ptr<objects::Null> result = std::dynamic_pointer_cast<objects::Null>(obj);
    EXPECT_NE(result, nullptr);
}

TEST(TestEvalIntegerExpression, BasicAssertions)
{
    struct Input
    {
        std::string input;
        int64_t expected;
    };

    struct Input inputs[]
    {
        {"5", 5},
            {"10", 10},
            {"-5", -5},
            {"-10", -10},
            {"5 + 5 + 5 + 5 - 10", 10},
            {"2 * 2 * 2 * 2 * 2", 32},
            {"-50 + 100 + -50", 0},
            {"5 * 2 + 10", 20},
            {"5 + 2 * 10", 25},
            {"20 + 2 * -10", 0},
            {"50 / 2 * 2 + 10", 60},
            {"2 * (5 + 10)", 30},
            {"3 * 3 * 3 + 10", 37},
            {"3 * (3 * 3) + 10", 37},
        {
            "(5 + 10 * 2 + 15 / 3) * 2 + -10", 50
        }
    };

    for (const auto &item : inputs)
    {
        std::shared_ptr<objects::Object> evaluatedObj = testEval(item.input);
        testIntegerObject(evaluatedObj, item.expected);
    }
}

TEST(TestEvalBooleanExpression, BasicAssertions)
{
    struct Input
    {
        std::string input;
        bool expected;
    };

    struct Input inputs[]
    {
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
    };

    for (const auto &item : inputs)
    {
        std::shared_ptr<objects::Object> evaluatedObj = testEval(item.input);
        testBooleanObject(evaluatedObj, item.expected);
    }
}

TEST(TestEvalStringExpression, BasicAssertions)
{
    struct Input
    {
        std::string input;
        std::string expected;
    };

    struct Input inputs[]
    {
        {"\"5\"", "5"},
        {"\"10\"", "10"},
        {"\"-5\"", "-5"},
        {"\"-10\"", "-10"},
        {"\"hello\"", "hello"},
        {"\"hello world\"", "hello world"}
    };

    for (const auto &item : inputs)
    {
        std::shared_ptr<objects::Object> evaluatedObj = testEval(item.input);
        testStringObject(evaluatedObj, item.expected);
    }
}

TEST(TestEvalStringConcatenation, BasicAssertions)
{
    struct Input
    {
        std::string input;
        std::string expected;
    };

    struct Input inputs[]
    {
        {"\"hello\" + \" \" + \"world!\"", "hello world!"},
        {"\"hello world\" + \"!\"", "hello world!"}
    };

    for (const auto &item : inputs)
    {
        std::shared_ptr<objects::Object> evaluatedObj = testEval(item.input);
        testStringObject(evaluatedObj, item.expected);
    }
}

TEST(TestEvalBangOperator, BasicAssertions)
{
    struct Input
    {
        std::string input;
        bool expected;
    };

    struct Input inputs[]
    {
        {"!true", false},
            {"!false", true},
            {"!5", false},
            {"!!true", true},
            {"!!false", false},
            {"!!5", true},
    };

    for (const auto &item : inputs)
    {
        std::shared_ptr<objects::Object> evaluatedObj = testEval(item.input);
        testBooleanObject(evaluatedObj, item.expected);
    }
}

TEST(TestEvalIfElseExpressions, BasicAssertions)
{
    struct Input
    {
        std::string input;
        std::variant<int, void *> expected;
    };

    struct Input inputs[]
    {
        {"if (true) { 10 }", 10},
            {"if (false) { 10 }", nullptr},
            {"if (1) { 10 }", 10},
            {"if (1 < 2) { 10 }", 10},
            {"if (1 > 2) { 10 }", nullptr},
            {"if (1 > 2) { 10 } else { 20 }", 20},
            {"if (1 < 2) { 10 } else { 20 }", 10},
    };

    for (const auto &item : inputs)
    {
        std::shared_ptr<objects::Object> evaluatedObj = testEval(item.input);

        if (std::holds_alternative<int>(item.expected))
        {
            int integer = std::get<int>(item.expected);
            testIntegerObject(evaluatedObj, static_cast<int64_t>(integer));
        }
        else
        {
            testNullObject(evaluatedObj);
        }
    }
}

TEST(TestEvalReturnStatements, BasicAssertions)
{
    struct Input
    {
        std::string input;
        int64_t expected;
    };

    struct Input inputs[]
    {
        {"return 10;", 10},
            {"return 10; 9;", 10},
            {"return 2 * 5; 9;", 10},
            {"9; return 2 * 5; 9;", 10},
            {"if (10 > 1) { return 10; }", 10},
            {
                R""(
if (10 > 1) {
  if (10 > 1) {
    return 10;
  }

  return 1;
}
)"",
                10,
            },
            {
                R""(
let f = fn(x) {
  return x;
  x + 10;
};
f(10);)"",
                10,
            },
            {
                R""(
let f = fn(x) {
   let result = x + 10;
   return result;
   return 10;
};
f(10);)"",
                20,
            },
    };

    for (const auto &item : inputs)
    {
        std::shared_ptr<objects::Object> evaluatedObj = testEval(item.input);
        testIntegerObject(evaluatedObj, item.expected);
    }
}

TEST(TestEvalErrorHandling, BasicAssertions)
{
    struct Input
    {
        std::string input;
        std::string expectedMessage;
    };

    struct Input inputs[]
    {
        {
            "5 + true;",
            "type mismatch: INTEGER + BOOLEAN",
        },
            {
                "5 + true; 5;",
                "type mismatch: INTEGER + BOOLEAN",
            },
            {
                "-true",
                "unknown operator: -BOOLEAN",
            },
            {
                "true + false;",
                "unknown operator: BOOLEAN + BOOLEAN",
            },
            {
                "true + false + true + false;",
                "unknown operator: BOOLEAN + BOOLEAN",
            },
            {
                "5; true + false; 5",
                "unknown operator: BOOLEAN + BOOLEAN",
            },
            {
                "if (10 > 1) { true + false; }",
                "unknown operator: BOOLEAN + BOOLEAN",
            },
            {
                R""(
if (10 > 1) {
  if (10 > 1) {
    return true + false;
  }

  return 1;
}
)"",
                "unknown operator: BOOLEAN + BOOLEAN",
            },
            {
                "foobar",
                "identifier not found: foobar",
            },
            {
                "\"Hello\" - \"World\"",
                "unknown operator: STRING - STRING"
            },
            {
                R""(
                    {"name": "Monkey"}[fn(x){ x }];
                )"",
                "unusable as hash key: FUNCTION"
            }
    };

    for (const auto &item : inputs)
    {
        std::shared_ptr<objects::Object> evaluatedObj = testEval(item.input);

        std::shared_ptr<objects::Error> errorObj = std::dynamic_pointer_cast<objects::Error>(evaluatedObj);

        EXPECT_NE(errorObj, nullptr);

        EXPECT_STREQ(errorObj->Message.c_str(), item.expectedMessage.c_str());
    }
}

TEST(TestEvalLetStatements, BasicAssertions)
{
    struct Input
    {
        std::string input;
        int64_t expected;
    };

    struct Input inputs[]
    {
        {"let a = 5; a;", 5},
            {"let a = 5 * 5; a;", 25},
            {"let a = 5; let b = a; b;", 5},
            {"let a = 5; let b = a; let c = a + b + 5; c;", 15},
    };

    for (const auto &item : inputs)
    {
        std::shared_ptr<objects::Object> evaluatedObj = testEval(item.input);
        testIntegerObject(evaluatedObj, item.expected);
    }
}

TEST(TestEvalFunctionObject, BasicAssertions)
{
    std::string input = "fn(x) { x + 2; };";

    std::shared_ptr<objects::Object> evaluatedObj = testEval(input);

    std::shared_ptr<objects::Function> funcObj = std::dynamic_pointer_cast<objects::Function>(evaluatedObj);
    EXPECT_NE(funcObj, nullptr);
    EXPECT_EQ(funcObj->Parameters.size(), 1u);
    EXPECT_STREQ(funcObj->Parameters[0]->String().c_str(), "x");
    EXPECT_STREQ(funcObj->Body->String().c_str(), "(x + 2)");
}

TEST(TestEvalFunctionApplication, BasicAssertions)
{
    struct Input
    {
        std::string input;
        int64_t expected;
    };

    struct Input inputs[]
    {
        {"let identity = fn(x) { x; }; identity(5);", 5},
            {"let identity = fn(x) { return x; }; identity(5);", 5},
            {"let double = fn(x) { x * 2; }; double(5);", 10},
            {"let add = fn(x, y) { x + y; }; add(5, 5);", 10},
            {"let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20},
            {"fn(x) { x; }(5)", 5},
    };

    for (const auto &item : inputs)
    {
        std::shared_ptr<objects::Object> evaluatedObj = testEval(item.input);
        testIntegerObject(evaluatedObj, item.expected);
    }
}

TEST(TestEvalEnclosingEnvironments, BasicAssertions)
{
    std::string input = R""(
let first = 10;
let second = 10;
let third = 10;

let ourFunction = fn(first) {
  let second = 20;

  first + second + third;
};

ourFunction(20) + first + second;)"";

    std::shared_ptr<objects::Object> evaluatedObj = testEval(input);
    testIntegerObject(evaluatedObj, 70);
}


TEST(TestBuiltinFunctions, BasicAssertions)
{
    struct Input
    {
        std::string input;
        std::variant<int, std::string> expected;
    };

    struct Input inputs[]
    {
        {"len(\"\")", 0},
        {"len(\"four\")", 4},
        {"len(\"hello world\")", 11},
        {"len(1)", "argument to `len` not supported, got INTEGER"},
        {"len(\"one\", \"two\")", "wrong number of arguments. got=2, want=1"},
        {"first([1,2,3])", 1},
        {"let myArray=[1,2,3]; first(myArray);", 1},
        {"first(2)", "argument to `first` must be ARRAY, got INTEGER"},
        {"last([1,2,3]);", 3},
        {"let myArray=[1,2,3]; last(myArray);", 3},
        {"last(2)", "argument to `last` must be ARRAY, got INTEGER"},
        {"let a = [1, 2, 3, 4]; rest(a);", "[2, 3, 4]"},
        {"let a = [1, 2, 3, 4]; rest(rest(a));", "[3, 4]"},
        {"let a = [1, 2, 3, 4]; rest(rest(rest(a)));", "[4]"},
        {"let a = [1, 2, 3, 4]; rest(rest(rest(rest(a))));", "[]"},
        {"let a = [1, 2, 3, 4]; rest(rest(rest(rest(rest(a)))));", "null"},
        {"let a = [1, 2, 3, 4]; rest(rest(rest(rest(rest(a))))); a;", "[1, 2, 3, 4]"},
        {"let a = [1, 2, 3, 4]; let b = push(a, 5); a;", "[1, 2, 3, 4]"},
        {"let a = [1, 2, 3, 4]; let b = push(a, 5); b;", "[1, 2, 3, 4, 5]"},
        {
            R""(
let map = fn(arr,f){
    let iter=fn(arr,accumulated){
        if(len(arr) == 0){
            return accumulated;
        } else {
            iter(rest(arr),push(accumulated,f(first(arr))));
        }
    }; 
    iter(arr,[]);
};

let a = [1, 2, 3, 4];
let double = fn(x){ x * 2 };
map(a, double);
            )"",
            "[2, 4, 6, 8]"
        },
        {
            R""(
let reduce = fn(arr, initial, f) {
    let iter = fn(arr, result) {
        if(len(arr) == 0){
            result;
        } else {
            iter(rest(arr), f(result, first(arr)));
        }
    };

    iter(arr, initial);
};

let sum = fn(arr){
    reduce(arr, 0, fn(initial, el) {
        initial + el;
    });
};

sum([1, 2, 3, 4, 5]);
            )"",
            15
        }
    };

    for (const auto &item : inputs)
    {
        std::shared_ptr<objects::Object> evaluatedObj = testEval(item.input);

        if (std::holds_alternative<int>(item.expected))
        {
            int integer = std::get<int>(item.expected);
            testIntegerObject(evaluatedObj, static_cast<int64_t>(integer));
        }
        else
        {
            std::string strVal = std::get<std::string>(item.expected);

            if(std::shared_ptr<objects::Array> arrObj = std::dynamic_pointer_cast<objects::Array>(evaluatedObj); arrObj != nullptr)
            {
                EXPECT_NE(arrObj, nullptr);
                EXPECT_STREQ(arrObj->Inspect().c_str(), strVal.c_str());
            }
            else if(std::shared_ptr<objects::Error> errObj = std::dynamic_pointer_cast<objects::Error>(evaluatedObj); errObj != nullptr)
            {
                EXPECT_NE(errObj, nullptr);
                EXPECT_STREQ(errObj->Message.c_str(), strVal.c_str());
            }
            else
            {
                std::shared_ptr<objects::Null> nullObj = std::dynamic_pointer_cast<objects::Null>(evaluatedObj);
                EXPECT_NE(nullObj, nullptr);
                EXPECT_STREQ(nullObj->Inspect().c_str(), strVal.c_str());
            }
        }
    }
}


TEST(TestEvalArrayLiteral, BasicAssertions)
{
	std::string input = "[1, 2 * 2, 3 + 3]";

    std::shared_ptr<objects::Object> evaluatedObj = testEval(input);

    std::shared_ptr<objects::Array> result = std::dynamic_pointer_cast<objects::Array>(evaluatedObj);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->Elements.size(), 3u);

    testIntegerObject(result->Elements[0], 1);
    testIntegerObject(result->Elements[1], 4);
    testIntegerObject(result->Elements[2], 6);
}


TEST(TestEvalArrayIndexExpression, BasicAssertions)
{
	struct Input
    {
        std::string input;
        std::variant<int, void*> expected;
    };

    struct Input inputs[]
    {
        {
            "[1, 2, 3][0]",
            1,
        },
        {
            "[1, 2, 3][1]",
            2
        },
        {
            "[1, 2, 3][2]",
            3
        },
        {
            "let i = 0; [1][i];",
            1
        },
        {
            "[1, 2, 3][1 + 1];",
            3,
        },
        {
            "let myArray = [1, 2, 3]; myArray[2];",
            3,
        },
        {
            "let myArray = [1, 2, 3]; myArray[0] + myArray[1] + myArray[2];",
            6,
        },
        {
            "let myArray = [1, 2, 3]; let i = myArray[0]; myArray[i]",
            2,
        },
        {
            "[1, 2, 3][3]",
            nullptr
        },
        {
            "[1, 2, 3][-1]",
            nullptr
        }
    };

    for (const auto &item : inputs)
    {
        std::shared_ptr<objects::Object> evaluatedObj = testEval(item.input);
        if (std::holds_alternative<int>(item.expected))
        {
            int integer = std::get<int>(item.expected);
            testIntegerObject(evaluatedObj, static_cast<int64_t>(integer));
        }
        else
        {
            testNullObject(evaluatedObj);
        }
    }
}


TEST(TestEvalHashLiterals, BasicAssertions)
{
	std::string input = R""(
let two = "two";

{
    "one": 10 - 9,
    two: 1 + 1,
    "thr" + "ee": 6 / 2,
    4: 4,
    true: 5,
    false: 6
}
    )"";

    std::map<objects::HashKey, int64_t> expected{
        {objects::String("one").GetHashKey(), 1},
        {objects::String("two").GetHashKey(), 2},
        {objects::String("three").GetHashKey(), 3},
        {objects::Integer(4).GetHashKey(), 4},
        {objects::TRUE_OBJ->GetHashKey(), 5},
        {objects::FALSE_OBJ->GetHashKey(), 6},
    };

    std::shared_ptr<objects::Object> evaluatedObj = testEval(input);

    std::shared_ptr<objects::Hash> result = std::dynamic_pointer_cast<objects::Hash>(evaluatedObj);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->Pairs.size(), expected.size());

    for(auto &[key, value]: expected)
    {
        auto fit = result->Pairs.find(key);
        EXPECT_NE(fit, result->Pairs.end());

        if(fit != result->Pairs.end())
        {
            auto hashpair = fit->second;
            testIntegerObject(hashpair->Value, value);
        }
    }
}


TEST(TestEvalHashIndexExpression, BasicAssertions)
{
	struct Input
    {
        std::string input;
        std::variant<int, void*> expected;
    };

    struct Input inputs[]
    {
        {
            R""(
                {"foo": 5}["foo"]
            )"", 
            5
        },
        {
            R""(
                {"foo": 5}["bar"]
            )"",
            nullptr
        },
        {
            R""(
                let key = "foo";
                {"foo": 5}[key]
            )"",
            5
        },
        {
            R""(
                {}["foo"]
            )"",
            nullptr
        },
        {
            R""(
                {5: 5}[5]
            )"",
            5
        },
        {
            R""(
                {true: 5}[true]
            )"",
            5
        },
        {
            R""(
                {false: 5}[false]
            )"",
            5
        }
    };

    for (const auto &item : inputs)
    {
        std::shared_ptr<objects::Object> evaluatedObj = testEval(item.input);
        if (std::holds_alternative<int>(item.expected))
        {
            int integer = std::get<int>(item.expected);
            testIntegerObject(evaluatedObj, static_cast<int64_t>(integer));
        }
        else
        {
            testNullObject(evaluatedObj);
        }
    }
}

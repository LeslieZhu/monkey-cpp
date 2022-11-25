#include <gtest/gtest.h>

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <variant>

#include "token/token.hpp"
#include "ast/ast.hpp"
#include "parser/parser.hpp"

using namespace std::string_literals;

void printParserErrors(std::vector<std::string> errors)
{
	if (errors.size() == 0)
	{
		return;
	}
	std::cout << "errors:" << std::endl;
	for (auto &e : errors)
	{
		std::cout << "\t" << e << std::endl;
	}
}

void testIntegerLiteral(std::shared_ptr<ast::Expression> exp, int64_t value)
{
	std::shared_ptr<ast::IntegerLiteral> stmt = std::dynamic_pointer_cast<ast::IntegerLiteral>(exp);

	EXPECT_NE(stmt, nullptr);
	EXPECT_EQ(stmt->Value, value);
	EXPECT_EQ(stmt->TokenLiteral(), std::to_string(value));
}

void testIdentifier(std::shared_ptr<ast::Expression> exp, std::string value)
{
	std::shared_ptr<ast::Identifier> stmt = std::dynamic_pointer_cast<ast::Identifier>(exp);

	EXPECT_NE(stmt, nullptr);
	EXPECT_EQ(stmt->Value, value);
	EXPECT_EQ(stmt->TokenLiteral(), value);
}

void testBooleanLiteral(std::shared_ptr<ast::Expression> exp, bool value)
{
	std::shared_ptr<ast::Boolean> stmt = std::dynamic_pointer_cast<ast::Boolean>(exp);

	EXPECT_NE(stmt, nullptr);

	if (value)
	{
		EXPECT_TRUE(stmt->Value);
		EXPECT_STREQ(stmt->TokenLiteral().c_str(), "true");
	}
	else
	{
		EXPECT_FALSE(stmt->Value);
		EXPECT_STREQ(stmt->TokenLiteral().c_str(), "false");
	}
}

void testLiteralExpression(std::shared_ptr<ast::Expression> exp, std::variant<std::string, bool, long long int, int> expected)
{
	if (std::holds_alternative<long long int>(expected))
	{
		testIntegerLiteral(exp, std::get<long long int>(expected));
	}
	else if (std::holds_alternative<int>(expected))
	{
		testIntegerLiteral(exp, static_cast<long long int>(std::get<int>(expected)));
	}
	else if (std::holds_alternative<std::string>(expected))
	{
		testIdentifier(exp, std::get<std::string>(expected));
	}
	else if (std::holds_alternative<bool>(expected))
	{
		testBooleanLiteral(exp, std::get<bool>(expected));
	}
	else
	{
		std::cout << "type of exp not handled. got=" << exp->String() << std::endl;
	}
}

void testInfixExpression(std::shared_ptr<ast::Expression> exp, std::variant<std::string, bool, long long int, int> left, std::string operatorStr, std::variant<std::string, bool, long long int, int> right)
{
	std::shared_ptr<ast::InfixExpression> infixStmt = std::dynamic_pointer_cast<ast::InfixExpression>(exp);

	EXPECT_NE(infixStmt, nullptr);

	testLiteralExpression(infixStmt->pLeft, left);
	EXPECT_STREQ(infixStmt->Operator.c_str(), operatorStr.c_str());
	testLiteralExpression(infixStmt->pRight, right);
}

std::shared_ptr<ast::LetStatement> testLetStatement(std::shared_ptr<ast::Statement> s, std::string name)
{
	EXPECT_EQ(s->TokenLiteral(), "let");

	std::shared_ptr<ast::LetStatement> stmt = std::dynamic_pointer_cast<ast::LetStatement>(s);

	EXPECT_NE(stmt, nullptr);
	EXPECT_EQ(stmt->GetNodeType(), ast::NodeType::LetStatement);
	EXPECT_EQ(stmt->pName->Value, name);
	EXPECT_EQ(stmt->pName->TokenLiteral(), name);

	return stmt;
}

TEST(TestLetStatements, BasicAssertions)
{

	struct Input
	{
		std::string input;
		std::string expectedIdentifier;
		std::variant<std::string, bool, long long int, int> expectedValue;
	};

	struct Input inputs[]
	{
		{
			"let x = 5", "x", 5},
			{"let y = true", "y", true},
		{
			"let foobar = y", "foobar", "y"s
		}
	};

	for (auto &s : inputs)
	{
		std::unique_ptr<lexer::Lexer> pLexer = lexer::New(s.input);
		std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
		std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
		printParserErrors(pParser->Errors());

		EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

		auto letStmt = testLetStatement(pProgram->v_pStatements[0], s.expectedIdentifier);
		testLiteralExpression(letStmt->pValue, s.expectedValue);
	}
}

TEST(TestReturnStatements, BasicAssertions)
{
	struct Input
	{
		std::string input;
		std::variant<std::string, bool, long long int, int> expectedValue;
	};

	struct Input inputs[]
	{
		{"return 5", 5},
			{"return true", true},
		{
			"return foobar", "foobar"s
		}
	};

	for (auto &item : inputs)
	{
		std::unique_ptr<lexer::Lexer> pLexer = lexer::New(item.input);
		std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
		std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
		printParserErrors(pParser->Errors());

		EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

		std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
		std::shared_ptr<ast::ReturnStatement> returnStmt = std::dynamic_pointer_cast<ast::ReturnStatement>(stmt);

		EXPECT_NE(returnStmt, nullptr);
		EXPECT_STREQ(returnStmt->TokenLiteral().c_str(), "return");
		testLiteralExpression(returnStmt->pReturnValue, item.expectedValue);
	}
}

TEST(TestIdentifierExpression, BasicAssertions)
{
	std::string input = "foobar;";

	std::unique_ptr<lexer::Lexer> pLexer = lexer::New(input);
	std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
	std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
	printParserErrors(pParser->Errors());

	EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

	std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
	std::shared_ptr<ast::ExpressionStatement> expStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmt);

	EXPECT_NE(expStmt, nullptr);

	std::shared_ptr<ast::Expression> exp = expStmt->pExpression;
	std::shared_ptr<ast::Identifier> identStmt = std::dynamic_pointer_cast<ast::Identifier>(exp);

	EXPECT_NE(identStmt, nullptr);
	EXPECT_STREQ(identStmt->Value.c_str(), "foobar");
	EXPECT_STREQ(identStmt->TokenLiteral().c_str(), "foobar");
}

TEST(TestIntegerLiteralExpression, BasicAssertions)
{
	std::string input = "5;";

	std::unique_ptr<lexer::Lexer> pLexer = lexer::New(input);
	std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
	std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
	printParserErrors(pParser->Errors());

	EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

	std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
	std::shared_ptr<ast::ExpressionStatement> expStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmt);

	EXPECT_NE(expStmt, nullptr);

	std::shared_ptr<ast::Expression> exp = expStmt->pExpression;
	std::shared_ptr<ast::IntegerLiteral> intStmt = std::dynamic_pointer_cast<ast::IntegerLiteral>(exp);

	EXPECT_NE(intStmt, nullptr);

	EXPECT_EQ(intStmt->Value, 5);
	EXPECT_STREQ(intStmt->TokenLiteral().c_str(), "5");
}

TEST(TestStringLiteralExpression, BasicAssertions)
{
	std::string input = "\"hello world\";";

	std::unique_ptr<lexer::Lexer> pLexer = lexer::New(input);
	std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
	std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
	printParserErrors(pParser->Errors());

	EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

	std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
	std::shared_ptr<ast::ExpressionStatement> expStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmt);

	EXPECT_NE(expStmt, nullptr);

	std::shared_ptr<ast::Expression> exp = expStmt->pExpression;
	std::shared_ptr<ast::StringLiteral> strStmt = std::dynamic_pointer_cast<ast::StringLiteral>(exp);

	EXPECT_NE(strStmt, nullptr);

	EXPECT_STREQ(strStmt->Value.c_str(), "hello world");
	EXPECT_STREQ(strStmt->TokenLiteral().c_str(), "hello world");
}

TEST(TestArrayLiteralExpression, BasicAssertions)
{
	std::string input = "[1, 2 * 2, 3 + 3]";

	std::unique_ptr<lexer::Lexer> pLexer = lexer::New(input);
	std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
	std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
	printParserErrors(pParser->Errors());

	EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

	std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
	std::shared_ptr<ast::ExpressionStatement> expStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmt);

	EXPECT_NE(expStmt, nullptr);

	std::shared_ptr<ast::Expression> exp = expStmt->pExpression;
	std::shared_ptr<ast::ArrayLiteral> arrayStmt = std::dynamic_pointer_cast<ast::ArrayLiteral>(exp);

	EXPECT_NE(arrayStmt, nullptr);
	EXPECT_EQ(arrayStmt->Elements.size(), 3u);
	testIntegerLiteral(arrayStmt->Elements[0], 1);
	testInfixExpression(arrayStmt->Elements[1], 2, "*", 2);
	testInfixExpression(arrayStmt->Elements[2], 3, "+", 3);
}

TEST(TestIndexExpression, BasicAssertions)
{
	std::string input = "myArray[1 + 1]";

	std::unique_ptr<lexer::Lexer> pLexer = lexer::New(input);
	std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
	std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
	printParserErrors(pParser->Errors());

	std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
	std::shared_ptr<ast::ExpressionStatement> expStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmt);

	EXPECT_NE(expStmt, nullptr);

	std::shared_ptr<ast::Expression> exp = expStmt->pExpression;
	std::shared_ptr<ast::IndexExpression> indexStmt = std::dynamic_pointer_cast<ast::IndexExpression>(exp);

	EXPECT_NE(indexStmt, nullptr);

	testIdentifier(indexStmt->Left, "myArray");
	testInfixExpression(indexStmt->Index, 1, "+", 1);
}

TEST(TestParsingPrefixExpressions, BasicAssertions)
{
	struct Input
	{
		std::string input;
		std::string operatorStr;
		std::variant<std::string, bool, long long int, int> value;
	};

	struct Input inputs[]
	{
		{"!5;", "!", 5},
			{"-15;", "-", 15},
			{"!foobar;", "!", "foobar"s},
			{"-foobar;", "-", "foobar"s},
			{"!true;", "!", true},
		{
			"!false;", "!", false
		}
	};

	for (auto &item : inputs)
	{
		std::unique_ptr<lexer::Lexer> pLexer = lexer::New(item.input);
		std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
		std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
		printParserErrors(pParser->Errors());

		EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

		std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
		std::shared_ptr<ast::ExpressionStatement> expStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmt);

		EXPECT_NE(expStmt, nullptr);

		std::shared_ptr<ast::Expression> exp = expStmt->pExpression;
		std::shared_ptr<ast::PrefixExpression> prefixExpStmt = std::dynamic_pointer_cast<ast::PrefixExpression>(exp);

		EXPECT_NE(prefixExpStmt, nullptr);

		EXPECT_STREQ(prefixExpStmt->Operator.c_str(), item.operatorStr.c_str());
		testLiteralExpression(prefixExpStmt->pRight, item.value);
	}
}

TEST(TestParsingInfixExpressions, BasicAssertions)
{
	struct Input
	{
		std::string input;
		std::variant<std::string, bool, long long int, int> leftValue;
		std::string operatorStr;
		std::variant<std::string, bool, long long int, int> rightValue;
	};

	struct Input inputs[]
	{
		{"5 + 5;", 5, "+", 5},
			{"5 - 5;", 5, "-", 5},
			{"5 * 5;", 5, "*", 5},
			{"5 / 5;", 5, "/", 5},
			{"5 > 5;", 5, ">", 5},
			{"5 < 5;", 5, "<", 5},
			{"5 == 5;", 5, "==", 5},
			{"5 != 5;", 5, "!=", 5},
			{"foobar + barfoo;", "foobar"s, "+", "barfoo"s},
			{"foobar - barfoo;", "foobar"s, "-", "barfoo"s},
			{"foobar * barfoo;", "foobar"s, "*", "barfoo"s},
			{"foobar / barfoo;", "foobar"s, "/", "barfoo"s},
			{"foobar > barfoo;", "foobar"s, ">", "barfoo"s},
			{"foobar < barfoo;", "foobar"s, "<", "barfoo"s},
			{"foobar == barfoo;", "foobar"s, "==", "barfoo"s},
			{"foobar != barfoo;", "foobar"s, "!=", "barfoo"s},
			{"true == true", true, "==", true},
			{"true != false", true, "!=", false},
		{
			"false == false", false, "==", false
		}
	};

	for (auto &item : inputs)
	{
		std::unique_ptr<lexer::Lexer> pLexer = lexer::New(item.input);
		std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
		std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
		printParserErrors(pParser->Errors());

		EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

		std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
		std::shared_ptr<ast::ExpressionStatement> expStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmt);

		EXPECT_NE(expStmt, nullptr);
		testInfixExpression(expStmt->pExpression, item.leftValue, item.operatorStr, item.rightValue);
	}
}

TEST(TestOperatorPrecedenceParsing, BasicAssertions)
{
	struct Input
	{
		std::string input;
		std::string expected;
	};

	struct Input inputs[]
	{
		{
			"-a * b",
			"((-a) * b)",
		},
			{
				"!-a",
				"(!(-a))",
			},
			{
				"a + b + c",
				"((a + b) + c)",
			},
			{
				"a + b - c",
				"((a + b) - c)",
			},
			{
				"a * b * c",
				"((a * b) * c)",
			},
			{
				"a * b / c",
				"((a * b) / c)",
			},
			{
				"a + b / c",
				"(a + (b / c))",
			},
			{
				"a + b * c + d / e - f",
				"(((a + (b * c)) + (d / e)) - f)",
			},
			{
				"3 + 4; -5 * 5",
				"(3 + 4)((-5) * 5)",
			},
			{
				"5 > 4 == 3 < 4",
				"((5 > 4) == (3 < 4))",
			},
			{
				"5 < 4 != 3 > 4",
				"((5 < 4) != (3 > 4))",
			},
			{
				"3 + 4 * 5 == 3 * 1 + 4 * 5",
				"((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))",
			},
			{
				"true",
				"true",
			},
			{
				"false",
				"false",
			},
			{
				"3 > 5 == false",
				"((3 > 5) == false)",
			},
			{
				"3 < 5 == true",
				"((3 < 5) == true)",
			},
			{
				"1 + (2 + 3) + 4",
				"((1 + (2 + 3)) + 4)",
			},
			{
				"(5 + 5) * 2",
				"((5 + 5) * 2)",
			},
			{
				"2 / (5 + 5)",
				"(2 / (5 + 5))",
			},
			{
				"(5 + 5) * 2 * (5 + 5)",
				"(((5 + 5) * 2) * (5 + 5))",
			},
			{
				"-(5 + 5)",
				"(-(5 + 5))",
			},
			{
				"!(true == true)",
				"(!(true == true))",
			},
			{
				"a + add(b * c) + d",
				"((a + add((b * c))) + d)",
			},
			{
				"add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
				"add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))",
			},
		{
			"add(a + b + c * d / f + g)",
				"add((((a + b) + ((c * d) / f)) + g))",
		},
		{
			"a * [1, 2, 3, 4][b * c] * d",
			"((a * ([1, 2, 3, 4][(b * c)])) * d)"
		},
		{
			"add(a * b[2], b[1], 2 * [1,2][1])",
			"add((a * (b[2])), (b[1]), (2 * ([1, 2][1])))"
		}
	};

	for (auto &item : inputs)
	{
		std::unique_ptr<lexer::Lexer> pLexer = lexer::New(item.input);
		std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
		std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
		printParserErrors(pParser->Errors());

		EXPECT_STREQ(pProgram->String().c_str(), item.expected.c_str());
	}
}

TEST(TestBooleanExpression, BasicAssertions)
{
	struct Input
	{
		std::string input;
		bool expectedBoolean;
	};

	struct Input inputs[]
	{
		{"true;", true},
		{
			"false;", false
		}
	};

	for (auto &item : inputs)
	{
		std::unique_ptr<lexer::Lexer> pLexer = lexer::New(item.input);
		std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
		std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
		printParserErrors(pParser->Errors());

		EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

		std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
		std::shared_ptr<ast::ExpressionStatement> expStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmt);

		EXPECT_NE(expStmt, nullptr);

		std::shared_ptr<ast::Expression> exp = expStmt->pExpression;
		std::shared_ptr<ast::Boolean> boolStmt = std::dynamic_pointer_cast<ast::Boolean>(exp);

		EXPECT_NE(boolStmt, nullptr);

		EXPECT_EQ(boolStmt->Value, item.expectedBoolean);
	}
}

TEST(TestIfExpression, BasicAssertions)
{
	std::string input = "if (x < y) { x }";

	std::unique_ptr<lexer::Lexer> pLexer = lexer::New(input);
	std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
	std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
	printParserErrors(pParser->Errors());

	EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

	std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
	std::shared_ptr<ast::ExpressionStatement> expStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmt);

	EXPECT_NE(expStmt, nullptr);

	std::shared_ptr<ast::Expression> exp = expStmt->pExpression;
	std::shared_ptr<ast::IfExpression> ifStmt = std::dynamic_pointer_cast<ast::IfExpression>(exp);

	EXPECT_NE(ifStmt, nullptr);

	testInfixExpression(ifStmt->pCondition, "x"s, "<", "y"s);

	EXPECT_EQ(ifStmt->pConsequence->v_pStatements.size(), 1u);

	std::shared_ptr<ast::Statement> baseStmt = ifStmt->pConsequence->v_pStatements[0];
	std::shared_ptr<ast::ExpressionStatement> exprStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(baseStmt);

	EXPECT_NE(exprStmt, nullptr);

	testIdentifier(exprStmt->pExpression, "x"s);
	EXPECT_EQ(ifStmt->pAlternative, nullptr);
}

TEST(TestIfElseExpression, BasicAssertions)
{
	std::string input = "if (x < y) { x } else { y }";

	std::unique_ptr<lexer::Lexer> pLexer = lexer::New(input);
	std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
	std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
	printParserErrors(pParser->Errors());

	EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

	std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
	std::shared_ptr<ast::ExpressionStatement> expStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmt);

	EXPECT_NE(expStmt, nullptr);

	std::shared_ptr<ast::Expression> exp = expStmt->pExpression;
	std::shared_ptr<ast::IfExpression> ifStmt = std::dynamic_pointer_cast<ast::IfExpression>(exp);

	EXPECT_NE(ifStmt, nullptr);

	testInfixExpression(ifStmt->pCondition, "x"s, "<", "y"s);

	EXPECT_EQ(ifStmt->pConsequence->v_pStatements.size(), 1u);

	std::shared_ptr<ast::Statement> baseStmt = ifStmt->pConsequence->v_pStatements[0];
	std::shared_ptr<ast::ExpressionStatement> exprStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(baseStmt);

	EXPECT_NE(exprStmt, nullptr);

	testIdentifier(exprStmt->pExpression, "x"s);

	EXPECT_EQ(ifStmt->pAlternative->v_pStatements.size(), 1u);

	std::shared_ptr<ast::Statement> baseStmtAlt = ifStmt->pAlternative->v_pStatements[0];
	std::shared_ptr<ast::ExpressionStatement> exprStmtAlt = std::dynamic_pointer_cast<ast::ExpressionStatement>(baseStmtAlt);

	EXPECT_NE(exprStmtAlt, nullptr);
	testIdentifier(exprStmtAlt->pExpression, "y"s);
}

TEST(TestFunctionLiteralParsing, BasicAssertions)
{
	std::string input = "fn(x, y) { x + y; }";

	std::unique_ptr<lexer::Lexer> pLexer = lexer::New(input);
	std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
	std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
	printParserErrors(pParser->Errors());

	EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

	std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
	std::shared_ptr<ast::ExpressionStatement> expStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmt);

	EXPECT_NE(expStmt, nullptr);

	std::shared_ptr<ast::Expression> exp = expStmt->pExpression;
	std::shared_ptr<ast::FunctionLiteral> funcStmt = std::dynamic_pointer_cast<ast::FunctionLiteral>(exp);

	EXPECT_NE(funcStmt, nullptr);

	EXPECT_EQ(funcStmt->v_pParameters.size(), 2u);

	testLiteralExpression(funcStmt->v_pParameters[0], "x"s);
	testLiteralExpression(funcStmt->v_pParameters[1], "y"s);

	EXPECT_EQ(funcStmt->pBody->v_pStatements.size(), 1u);

	std::shared_ptr<ast::Statement> stmtBody = funcStmt->pBody->v_pStatements[0];
	std::shared_ptr<ast::ExpressionStatement> bodyStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmtBody);

	EXPECT_NE(bodyStmt, nullptr);

	testInfixExpression(bodyStmt->pExpression, "x"s, "+", "y"s);
}

TEST(TestFunctionParameterParsing, BasicAssertions)
{
	struct Input
	{
		std::string input;
		std::vector<std::string> expectedParams;
	};

	struct Input inputs[]
	{
		{"fn() {};", {}},
			{"fn(x) {};", {"x"}},
		{
			"fn(x, y, z) {};", { "x", "y", "z" }
		}
	};

	for (auto &item : inputs)
	{
		std::unique_ptr<lexer::Lexer> pLexer = lexer::New(item.input);
		std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
		std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
		printParserErrors(pParser->Errors());

		std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
		std::shared_ptr<ast::ExpressionStatement> expStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmt);

		EXPECT_NE(expStmt, nullptr);

		std::shared_ptr<ast::Expression> exp = expStmt->pExpression;
		std::shared_ptr<ast::FunctionLiteral> funcStmt = std::dynamic_pointer_cast<ast::FunctionLiteral>(exp);

		EXPECT_NE(funcStmt, nullptr);

		EXPECT_EQ(funcStmt->v_pParameters.size(), item.expectedParams.size());
		for (unsigned long i = 0; i < item.expectedParams.size(); i++)
		{
			testLiteralExpression(funcStmt->v_pParameters[i], item.expectedParams[i]);
		}
	}
}

TEST(TestCallExpressionParsing, BasicAssertions)
{
	std::string input = "add(1, 2 * 3, 4 + 5);";

	std::unique_ptr<lexer::Lexer> pLexer = lexer::New(input);
	std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
	std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
	printParserErrors(pParser->Errors());

	EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

	std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
	std::shared_ptr<ast::ExpressionStatement> expStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmt);

	EXPECT_NE(expStmt, nullptr);

	std::shared_ptr<ast::Expression> exp = expStmt->pExpression;
	std::shared_ptr<ast::CallExpression> callStmt = std::dynamic_pointer_cast<ast::CallExpression>(exp);

	EXPECT_NE(callStmt, nullptr);

	testIdentifier(callStmt->pFunction, "add");

	EXPECT_EQ(callStmt->pArguments.size(), 3u);

	testLiteralExpression(callStmt->pArguments[0], 1);
	testInfixExpression(callStmt->pArguments[1], 2, "*", 3);
	testInfixExpression(callStmt->pArguments[2], 4, "+", 5);
}

TEST(TestCallExpressionParameterParsing, BasicAssertions)
{
	struct Input
	{
		std::string input;
		std::string expectedIdent;
		std::vector<std::string> expectedArgs;
	};

	struct Input inputs[]
	{
		{
			"add();",
			"add",
			{},
		},
			{
				"add(1);",
				"add",
				{"1"},
			},
		{
			"add(1, 2 * 3, 4 + 5);",
				"add",
				{"1", "(2 * 3)", "(4 + 5)"},
		}
	};

	for (auto &item : inputs)
	{
		std::unique_ptr<lexer::Lexer> pLexer = lexer::New(item.input);
		std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
		std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
		printParserErrors(pParser->Errors());

		EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

		std::shared_ptr<ast::Statement> stmt = pProgram->v_pStatements[0];
		std::shared_ptr<ast::ExpressionStatement> expStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(stmt);

		EXPECT_NE(expStmt, nullptr);

		std::shared_ptr<ast::Expression> exp = expStmt->pExpression;
		std::shared_ptr<ast::CallExpression> callStmt = std::dynamic_pointer_cast<ast::CallExpression>(exp);

		EXPECT_NE(callStmt, nullptr);

		testIdentifier(callStmt->pFunction, item.expectedIdent);

		EXPECT_EQ(callStmt->pArguments.size(), item.expectedArgs.size());

		for (unsigned long i = 0; i < item.expectedArgs.size(); i++)
		{
			EXPECT_STREQ(callStmt->pArguments[i]->String().c_str(), item.expectedArgs[i].c_str());
		}
	}
}

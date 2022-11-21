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

void testIntegerLiteral(std::unique_ptr<ast::Expression> exp, int64_t value)
{
	auto x = reinterpret_cast<ast::IntegerLiteral *>(exp.release());
	std::unique_ptr<ast::IntegerLiteral> stmt(x);

	EXPECT_NE(stmt, nullptr);
	EXPECT_EQ(stmt->Value, value);
	EXPECT_EQ(stmt->TokenLiteral(), std::to_string(value));
}

void testIdentifier(std::unique_ptr<ast::Expression> exp, std::string value)
{
	auto x = reinterpret_cast<ast::Identifier *>(exp.release());
	std::unique_ptr<ast::Identifier> stmt(x);

	EXPECT_NE(stmt, nullptr);
	EXPECT_EQ(stmt->Value, value);
	EXPECT_EQ(stmt->TokenLiteral(), value);
}

void testBooleanLiteral(std::unique_ptr<ast::Expression> exp, bool value)
{
	auto x = reinterpret_cast<ast::Boolean *>(exp.release());
	std::unique_ptr<ast::Boolean> stmt(x);

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

void testLiteralExpression(std::unique_ptr<ast::Expression> exp, std::variant<std::string, bool, long long int, int> expected)
{
	if (std::holds_alternative<long long int>(expected))
	{
		testIntegerLiteral(std::move(exp), std::get<long long int>(expected));
	}
	else if (std::holds_alternative<int>(expected))
	{
		testIntegerLiteral(std::move(exp), static_cast<long long int>(std::get<int>(expected)));
	}
	else if (std::holds_alternative<std::string>(expected))
	{
		testIdentifier(std::move(exp), std::get<std::string>(expected));
	}
	else if (std::holds_alternative<bool>(expected))
	{
		testBooleanLiteral(std::move(exp), std::get<bool>(expected));
	}
	else
	{
		std::cout << "type of exp not handled. got=" << exp->String() << std::endl;
	}
}

void testInfixExpression(std::unique_ptr<ast::Expression> exp, std::variant<std::string, bool, long long int, int> left, std::string operatorStr, std::variant<std::string, bool, long long int, int> right)
{
	auto x = reinterpret_cast<ast::InfixExpression *>(exp.release());
	std::unique_ptr<ast::InfixExpression> infixStmt(x);

	EXPECT_NE(infixStmt, nullptr);

	testLiteralExpression(std::move(infixStmt->pLeft), left);
	EXPECT_STREQ(infixStmt->Operator.c_str(), operatorStr.c_str());
	testLiteralExpression(std::move(infixStmt->pRight), right);
}

std::unique_ptr<ast::LetStatement> testLetStatement(std::unique_ptr<ast::Statement> s, std::string name)
{
	EXPECT_EQ(s->TokenLiteral(), "let");

	auto x = reinterpret_cast<ast::LetStatement *>(s.release());
	std::unique_ptr<ast::LetStatement> stmt(x);

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

		auto letStmt = testLetStatement(std::move(pProgram->v_pStatements[0]), s.expectedIdentifier);
		testLiteralExpression(std::move(letStmt->pValue), s.expectedValue);
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

		std::unique_ptr<ast::Statement> stmt = std::move(pProgram->v_pStatements[0]);
		auto x = reinterpret_cast<ast::ReturnStatement *>(stmt.release());
		std::unique_ptr<ast::ReturnStatement> returnStmt(x);

		EXPECT_NE(returnStmt, nullptr);
		EXPECT_STREQ(returnStmt->TokenLiteral().c_str(), "return");
		testLiteralExpression(std::move(returnStmt->pReturnValue), item.expectedValue);
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

	std::unique_ptr<ast::Statement> stmt = std::move(pProgram->v_pStatements[0]);
	auto x = reinterpret_cast<ast::ExpressionStatement *>(stmt.release());
	std::unique_ptr<ast::ExpressionStatement> expStmt(x);

	EXPECT_NE(expStmt, nullptr);

	std::unique_ptr<ast::Expression> exp = std::move(expStmt->pExpression);
	auto y = reinterpret_cast<ast::Identifier *>(exp.release());
	std::unique_ptr<ast::Identifier> identStmt(y);

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

	std::unique_ptr<ast::Statement> stmt = std::move(pProgram->v_pStatements[0]);
	auto x = reinterpret_cast<ast::ExpressionStatement *>(stmt.release());
	std::unique_ptr<ast::ExpressionStatement> expStmt(x);

	EXPECT_NE(expStmt, nullptr);

	std::unique_ptr<ast::Expression> exp = std::move(expStmt->pExpression);
	auto y = reinterpret_cast<ast::IntegerLiteral *>(exp.release());
	std::unique_ptr<ast::IntegerLiteral> intStmt(y);

	EXPECT_NE(intStmt, nullptr);

	EXPECT_EQ(intStmt->Value, 5);
	EXPECT_STREQ(intStmt->TokenLiteral().c_str(), "5");
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

		std::unique_ptr<ast::Statement> stmt = std::move(pProgram->v_pStatements[0]);
		auto x = reinterpret_cast<ast::ExpressionStatement *>(stmt.release());
		std::unique_ptr<ast::ExpressionStatement> expStmt(x);

		EXPECT_NE(expStmt, nullptr);

		std::unique_ptr<ast::Expression> exp = std::move(expStmt->pExpression);
		auto y = reinterpret_cast<ast::PrefixExpression *>(exp.release());
		std::unique_ptr<ast::PrefixExpression> prefixExpStmt(y);

		EXPECT_NE(prefixExpStmt, nullptr);

		EXPECT_STREQ(prefixExpStmt->Operator.c_str(), item.operatorStr.c_str());
		testLiteralExpression(std::move(prefixExpStmt->pRight), item.value);
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

		std::unique_ptr<ast::Statement> stmt = std::move(pProgram->v_pStatements[0]);
		auto x = reinterpret_cast<ast::ExpressionStatement *>(stmt.release());
		std::unique_ptr<ast::ExpressionStatement> expStmt(x);

		EXPECT_NE(expStmt, nullptr);
		testInfixExpression(std::move(expStmt->pExpression), item.leftValue, item.operatorStr, item.rightValue);
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

		std::unique_ptr<ast::Statement> stmt = std::move(pProgram->v_pStatements[0]);
		auto x = reinterpret_cast<ast::ExpressionStatement *>(stmt.release());
		std::unique_ptr<ast::ExpressionStatement> expStmt(x);

		EXPECT_NE(expStmt, nullptr);

		std::unique_ptr<ast::Expression> exp = std::move(expStmt->pExpression);
		auto y = reinterpret_cast<ast::Boolean *>(exp.release());
		std::unique_ptr<ast::Boolean> boolStmt(y);

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

	std::unique_ptr<ast::Statement> stmt = std::move(pProgram->v_pStatements[0]);
	auto x = reinterpret_cast<ast::ExpressionStatement *>(stmt.release());
	std::unique_ptr<ast::ExpressionStatement> expStmt(x);

	EXPECT_NE(expStmt, nullptr);

	std::unique_ptr<ast::Expression> exp = std::move(expStmt->pExpression);
	auto y = reinterpret_cast<ast::IfExpression *>(exp.release());
	std::unique_ptr<ast::IfExpression> ifStmt(y);

	EXPECT_NE(ifStmt, nullptr);

	testInfixExpression(std::move(ifStmt->pCondition), "x"s, "<", "y"s);

	EXPECT_EQ(ifStmt->pConsequence->v_pStatements.size(), 1u);

	std::unique_ptr<ast::Statement> baseStmt = std::move(ifStmt->pConsequence->v_pStatements[0]);
	auto y2 = reinterpret_cast<ast::ExpressionStatement *>(baseStmt.release());
	std::unique_ptr<ast::ExpressionStatement> exprStmt(y2);

	EXPECT_NE(exprStmt, nullptr);

	testIdentifier(std::move(exprStmt->pExpression), "x"s);
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

	std::unique_ptr<ast::Statement> stmt = std::move(pProgram->v_pStatements[0]);
	auto x = reinterpret_cast<ast::ExpressionStatement *>(stmt.release());
	std::unique_ptr<ast::ExpressionStatement> expStmt(x);

	EXPECT_NE(expStmt, nullptr);

	std::unique_ptr<ast::Expression> exp = std::move(expStmt->pExpression);
	auto y = reinterpret_cast<ast::IfExpression *>(exp.release());
	std::unique_ptr<ast::IfExpression> ifStmt(y);

	EXPECT_NE(ifStmt, nullptr);

	testInfixExpression(std::move(ifStmt->pCondition), "x"s, "<", "y"s);

	EXPECT_EQ(ifStmt->pConsequence->v_pStatements.size(), 1u);

	std::unique_ptr<ast::Statement> baseStmt = std::move(ifStmt->pConsequence->v_pStatements[0]);
	auto y2 = reinterpret_cast<ast::ExpressionStatement *>(baseStmt.release());
	std::unique_ptr<ast::ExpressionStatement> exprStmt(y2);

	EXPECT_NE(exprStmt, nullptr);

	testIdentifier(std::move(exprStmt->pExpression), "x"s);

	EXPECT_EQ(ifStmt->pAlternative->v_pStatements.size(), 1u);

	std::unique_ptr<ast::Statement> baseStmtAlt = std::move(ifStmt->pAlternative->v_pStatements[0]);
	auto y3 = reinterpret_cast<ast::ExpressionStatement *>(baseStmtAlt.release());
	std::unique_ptr<ast::ExpressionStatement> exprStmtAlt(y3);

	EXPECT_NE(exprStmtAlt, nullptr);
	testIdentifier(std::move(exprStmtAlt->pExpression), "y"s);
}

TEST(TestFunctionLiteralParsing, BasicAssertions)
{
	std::string input = "fn(x, y) { x + y; }";

	std::unique_ptr<lexer::Lexer> pLexer = lexer::New(input);
	std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
	std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};
	printParserErrors(pParser->Errors());

	EXPECT_EQ(pProgram->v_pStatements.size(), 1u);

	std::unique_ptr<ast::Statement> stmt = std::move(pProgram->v_pStatements[0]);
	auto x = reinterpret_cast<ast::ExpressionStatement *>(stmt.release());
	std::unique_ptr<ast::ExpressionStatement> expStmt(x);

	EXPECT_NE(expStmt, nullptr);

	std::unique_ptr<ast::Expression> exp = std::move(expStmt->pExpression);
	auto y = reinterpret_cast<ast::FunctionLiteral *>(exp.release());
	std::unique_ptr<ast::FunctionLiteral> funcStmt(y);

	EXPECT_NE(funcStmt, nullptr);

	EXPECT_EQ(funcStmt->v_pParameters.size(), 2u);

	testLiteralExpression(std::move(funcStmt->v_pParameters[0]), "x"s);
	testLiteralExpression(std::move(funcStmt->v_pParameters[1]), "y"s);

	EXPECT_EQ(funcStmt->pBody->v_pStatements.size(), 1u);

	std::unique_ptr<ast::Statement> stmtBody = std::move(funcStmt->pBody->v_pStatements[0]);
	auto y2 = reinterpret_cast<ast::ExpressionStatement *>(stmtBody.release());
	std::unique_ptr<ast::ExpressionStatement> bodyStmt(y2);

	EXPECT_NE(bodyStmt, nullptr);

	testInfixExpression(std::move(bodyStmt->pExpression), "x"s, "+", "y"s);
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

		std::unique_ptr<ast::Statement> stmt = std::move(pProgram->v_pStatements[0]);
		auto x = reinterpret_cast<ast::ExpressionStatement *>(stmt.release());
		std::unique_ptr<ast::ExpressionStatement> expStmt(x);

		EXPECT_NE(expStmt, nullptr);

		std::unique_ptr<ast::Expression> exp = std::move(expStmt->pExpression);
		auto y = reinterpret_cast<ast::FunctionLiteral *>(exp.release());
		std::unique_ptr<ast::FunctionLiteral> funcStmt(y);

		EXPECT_NE(funcStmt, nullptr);

		EXPECT_EQ(funcStmt->v_pParameters.size(), item.expectedParams.size());
		for (unsigned long i = 0; i < item.expectedParams.size(); i++)
		{
			testLiteralExpression(std::move(funcStmt->v_pParameters[i]), item.expectedParams[i]);
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

	std::unique_ptr<ast::Statement> stmt = std::move(pProgram->v_pStatements[0]);
	auto x = reinterpret_cast<ast::ExpressionStatement *>(stmt.release());
	std::unique_ptr<ast::ExpressionStatement> expStmt(x);

	EXPECT_NE(expStmt, nullptr);

	std::unique_ptr<ast::Expression> exp = std::move(expStmt->pExpression);
	auto y = reinterpret_cast<ast::CallExpression *>(exp.release());
	std::unique_ptr<ast::CallExpression> callStmt(y);

	EXPECT_NE(callStmt, nullptr);

	testIdentifier(std::move(callStmt->pFunction), "add");

	EXPECT_EQ(callStmt->pArguments.size(), 3u);

	testLiteralExpression(std::move(callStmt->pArguments[0]), 1);
	testInfixExpression(std::move(callStmt->pArguments[1]), 2, "*", 3);
	testInfixExpression(std::move(callStmt->pArguments[2]), 4, "+", 5);
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

		std::unique_ptr<ast::Statement> stmt = std::move(pProgram->v_pStatements[0]);
		auto x = reinterpret_cast<ast::ExpressionStatement *>(stmt.release());
		std::unique_ptr<ast::ExpressionStatement> expStmt(x);

		EXPECT_NE(expStmt, nullptr);

		std::unique_ptr<ast::Expression> exp = std::move(expStmt->pExpression);
		auto y = reinterpret_cast<ast::CallExpression *>(exp.release());
		std::unique_ptr<ast::CallExpression> callStmt(y);

		EXPECT_NE(callStmt, nullptr);

		testIdentifier(std::move(callStmt->pFunction), item.expectedIdent);

		EXPECT_EQ(callStmt->pArguments.size(), item.expectedArgs.size());

		for (unsigned long i = 0; i < item.expectedArgs.size(); i++)
		{
			EXPECT_STREQ(callStmt->pArguments[i]->String().c_str(), item.expectedArgs[i].c_str());
		}
	}
}

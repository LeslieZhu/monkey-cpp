#include <gtest/gtest.h>

#include <vector>
#include <memory>

#include "token/token.hpp"
#include "ast/ast.hpp"

TEST(TestString, BasicAssertions)
{
	token::Token tokLet(token::types::LET, "let");
	token::Token tokIndent1(token::types::IDENT, "myVar");
	token::Token tokIndent2(token::types::IDENT, "anotherVar");

	auto letStmt = std::make_shared<ast::LetStatement>();
	letStmt->Token = tokLet;
	letStmt->pName = std::make_shared<ast::Identifier>(tokIndent1, "myVar");
	letStmt->pValue = std::make_shared<ast::Identifier>(tokIndent2,"anotherVar");

	ast::Program program;
	program.v_pStatements.push_back(letStmt);

	EXPECT_EQ(program.String(), "let myVar = anotherVar;");

}

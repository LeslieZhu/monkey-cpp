#include <gtest/gtest.h>

#include <vector>

#include "token/token.hpp"
#include "lexer/lexer.hpp"

TEST(TestNextToken, BasicAssertions)
{
    std::string input = R""(
        let five = 5;
        let ten = 10;
  
        let add = fn(x, y) {
             x + y;
        };
      
        let result = add(five, ten);
        !-/*5;
        5 < 10 > 5;
    
        if (5 < 10) {
            return true;
        } else {
            return false;
        }
 
        10 == 10;
        10 != 9;
        )"";

    std::vector<token::Token> tests{{token::types::LET, "let"},
                                    {token::types::IDENT, "five"},
                                    {token::types::ASSIGN, "="},
                                    {token::types::INT, "5"},
                                    {token::types::SEMICOLON, ";"},
                                    {token::types::LET, "let"},
                                    {token::types::IDENT, "ten"},
                                    {token::types::ASSIGN, "="},
                                    {token::types::INT, "10"},
                                    {token::types::SEMICOLON, ";"},
                                    {token::types::LET, "let"},
                                    {token::types::IDENT, "add"},
                                    {token::types::ASSIGN, "="},
                                    {token::types::FUNCTION, "fn"},
                                    {token::types::LPAREN, "("},
                                    {token::types::IDENT, "x"},
                                    {token::types::COMMA, ","},
                                    {token::types::IDENT, "y"},
                                    {token::types::RPAREN, ")"},
                                    {token::types::LBRACE, "{"},
                                    {token::types::IDENT, "x"},
                                    {token::types::PLUS, "+"},
                                    {token::types::IDENT, "y"},
                                    {token::types::SEMICOLON, ";"},
                                    {token::types::RBRACE, "}"},
                                    {token::types::SEMICOLON, ";"},
                                    {token::types::LET, "let"},
                                    {token::types::IDENT, "result"},
                                    {token::types::ASSIGN, "="},
                                    {token::types::IDENT, "add"},
                                    {token::types::LPAREN, "("},
                                    {token::types::IDENT, "five"},
                                    {token::types::COMMA, ","},
                                    {token::types::IDENT, "ten"},
                                    {token::types::RPAREN, ")"},
                                    {token::types::SEMICOLON, ";"},
                                    {token::types::BANG, "!"},
                                    {token::types::MINUS, "-"},
                                    {token::types::SLASH, "/"},
                                    {token::types::ASTERISK, "*"},
                                    {token::types::INT, "5"},
                                    {token::types::SEMICOLON, ";"},
                                    {token::types::INT, "5"},
                                    {token::types::LT, "<"},
                                    {token::types::INT, "10"},
                                    {token::types::GT, ">"},
                                    {token::types::INT, "5"},
                                    {token::types::SEMICOLON, ";"},
                                    {token::types::IF, "if"},
                                    {token::types::LPAREN, "("},
                                    {token::types::INT, "5"},
                                    {token::types::LT, "<"},
                                    {token::types::INT, "10"},
                                    {token::types::RPAREN, ")"},
                                    {token::types::LBRACE, "{"},
                                    {token::types::RETURN, "return"},
                                    {token::types::TRUE, "true"},
                                    {token::types::SEMICOLON, ";"},
                                    {token::types::RBRACE, "}"},
                                    {token::types::ELSE, "else"},
                                    {token::types::LBRACE, "{"},
                                    {token::types::RETURN, "return"},
                                    {token::types::FALSE, "false"},
                                    {token::types::SEMICOLON, ";"},
                                    {token::types::RBRACE, "}"},
                                    {token::types::INT, "10"},
                                    {token::types::EQ, "=="},
                                    {token::types::INT, "10"},
                                    {token::types::SEMICOLON, ";"},
                                    {token::types::INT, "10"},
                                    {token::types::NOT_EQ, "!="},
                                    {token::types::INT, "9"},
                                    {token::types::SEMICOLON, ";"},
                                    {token::types::EndOF, ""}};

    auto lexer = lexer::New(input);
    for (const auto &test : tests)
    {
        token::Token tok = lexer->NextToken();
        EXPECT_EQ(tok.Type, test.Type);
        EXPECT_EQ(tok.Literal, test.Literal);
    }
}
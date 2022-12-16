#ifndef H_TOKEN_H
#define H_TOKEN_H

#include <iostream>
#include <string>
#include <map>

namespace token
{

    using TokenType = std::string;

    struct Token
    {
        TokenType Type;
        std::string Literal;

        Token(){}
        Token(TokenType type, std::string literal): Type(type), Literal(literal){}
    };

    namespace types
    {
        const TokenType ILLEGAL = "ILLEGAL";
        const TokenType EndOF = "EOF";

        // Identifiers + literals
        const TokenType IDENT = "IDENT"; // add, foobar, x, y, ...
        const TokenType INT = "INT";     // 1343456
        const TokenType STRING = "STRING"; // "foo bar"

        // Operators
        const TokenType ASSIGN = "=";
        const TokenType PLUS = "+";
        const TokenType MINUS = "-";
        const TokenType BANG = "!";
        const TokenType ASTERISK = "*";
        const TokenType SLASH = "/";

        const TokenType LT = "<";
        const TokenType GT = ">";

        const TokenType EQ = "==";
        const TokenType NOT_EQ = "!=";

        // Delimiters
        const TokenType COMMA = ",";
        const TokenType SEMICOLON = ";";
        const TokenType COLON = ":";

        const TokenType LPAREN = "(";
        const TokenType RPAREN = ")";
        const TokenType LBRACE = "{";
        const TokenType RBRACE = "}";

        const TokenType LBRACKET = "[";
        const TokenType RBRACKET = "]";

        // Keywords
        const TokenType FUNCTION = "FUNCTION";
        const TokenType LET = "LET";
        const TokenType TRUE = "TRUE";
        const TokenType FALSE = "FALSE";
        const TokenType IF = "IF";
        const TokenType ELSE = "ELSE";
        const TokenType RETURN = "RETURN";
    }



    std::ostream &operator<<(std::ostream &out, Token &tok)
    {
        out << "{Type:" << tok.Type << " Literal:" << tok.Literal << "}";
        return out;
    }

    std::map<std::string, TokenType> keywords = {{"fn", token::types::FUNCTION},
                                                 {"let", token::types::LET},
                                                 {"true", token::types::TRUE},
                                                 {"false", token::types::FALSE},
                                                 {"if", token::types::IF},
                                                 {"else", token::types::ELSE},
                                                 {"return", token::types::RETURN}};

    TokenType LookupIdent(std::string ident)
    {
        auto fit = keywords.find(ident);

        if (fit != keywords.end())
        {
            return fit->second;
        }
        else
        {
            return token::types::IDENT;
        }
    }

}

#endif // H_TOKEN_H

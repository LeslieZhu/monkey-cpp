#ifndef H_LEXER_H
#define H_LEXER_H

#include <iostream>
#include <string>
#include <memory>

#include "token/token.hpp"

namespace lexer
{

    bool isLetter(char ch)
    {
        return (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_');
    }

    bool isDigit(char ch)
    {
        return ('0' <= ch && ch <= '9');
    }

    token::Token newToken(token::TokenType tokenType, char ch)
    {
        token::Token tok;
        tok.Type = tokenType;
        tok.Literal = std::string(1, ch);
        return tok;
    }

    struct Lexer
    {
        std::string input;
        int position;     // current position in input (points to current char)
        int readPosition; // current reading position in input (after current char)
        char ch;          // current char under examination

        Lexer(const std::string input_) : input(input_),
                                          position(0),
                                          readPosition(0),
                                          ch(' ')
        {
        }

        void skipWhitespace()
        {
            while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
            {
                readChar();
            }
        }

        void readChar()
        {
            if (readPosition >= static_cast<int>(input.size()))
            {
                ch = 0;
            }
            else
            {
                ch = input[readPosition];
            }

            position = readPosition;
            readPosition += 1;
        }

        char peekChar()
        {
            if (readPosition >= static_cast<int>(input.size()))
            {
                return 0;
            }
            else
            {
                return input[readPosition];
            }
        }

        std::string readIdentifier()
        {
            int oldPosition = position;
            while (isLetter(ch))
            {
                readChar();
            }

            return input.substr(oldPosition, position - oldPosition);
        }

        std::string readString()
        {
            int oldPosition = position + 1;
            while (true)
            {
                readChar();
                if(ch == '"' || ch == 0)
                {
                    break;
                }
            }

            return input.substr(oldPosition, position - oldPosition);
        }

        std::string readNumber()
        {
            int oldPosition = position;
            while (isDigit(ch))
            {
                readChar();
            }

            return input.substr(oldPosition, position - oldPosition);
        }

        token::Token NextToken()
        {
            token::Token tok;

            skipWhitespace();

            switch (ch)
            {
            case '=':
            {
                if (peekChar() == '=')
                {
                    char oldChar = ch;
                    readChar();
                    std::string literal = std::string(1, oldChar) + std::string(1, ch);
                    tok.Type = token::types::EQ;
                    tok.Literal = literal;
                }
                else
                {
                    tok = newToken(token::types::ASSIGN, ch);
                }
            }
            break;
            case '+':
            {
                tok = newToken(token::types::PLUS, ch);
            }
            break;
            case '-':
            {
                tok = newToken(token::types::MINUS, ch);
            }
            break;
            case '!':
            {
                if (peekChar() == '=')
                {
                    char oldChar = ch;
                    readChar();
                    std::string literal = std::string(1, oldChar) + std::string(1, ch);
                    tok.Type = token::types::NOT_EQ;
                    tok.Literal = literal;
                }
                else
                {
                    tok = newToken(token::types::BANG, ch);
                }
            }
            break;
            case '/':
                tok = newToken(token::types::SLASH, ch);
                break;
            case '*':
                tok = newToken(token::types::ASTERISK, ch);
                break;
            case '<':
                tok = newToken(token::types::LT, ch);
                break;
            case '>':
                tok = newToken(token::types::GT, ch);
                break;
            case ';':
                tok = newToken(token::types::SEMICOLON, ch);
                break;
            case ',':
                tok = newToken(token::types::COMMA, ch);
                break;
            case '{':
                tok = newToken(token::types::LBRACE, ch);
                break;
            case '}':
                tok = newToken(token::types::RBRACE, ch);
                break;
            case '(':
                tok = newToken(token::types::LPAREN, ch);
                break;
            case ')':
                tok = newToken(token::types::RPAREN, ch);
                break;
            case '[':
                tok = newToken(token::types::LBRACKET, ch);
                break;
            case ']':
                tok = newToken(token::types::RBRACKET, ch);
                break;
            case ':':
                tok = newToken(token::types::COLON, ch);
                break;
            case '"':
            {
                tok.Type = token::types::STRING;
                tok.Literal = readString();
            }
            break;
            case 0:
            {
                tok.Literal = "";
                tok.Type = token::types::EndOF;
            }
            break;
            default:
            {
                if (isLetter(ch))
                {
                    tok.Literal = readIdentifier();
                    tok.Type = token::LookupIdent(tok.Literal);
                    return tok;
                }
                else if (isDigit(ch))
                {
                    tok.Literal = readNumber();
                    tok.Type = token::types::INT;
                    return tok;
                }
                else
                {
                    tok = newToken(token::types::ILLEGAL, ch);
                }
            }
            }

            readChar();
            return tok;
        }
    };

    std::unique_ptr<Lexer> New(std::string input)
    {
        std::unique_ptr<Lexer> l = std::make_unique<Lexer>(input);
        l->readChar();
        return l;
    }
}

#endif // H_LEXER_H

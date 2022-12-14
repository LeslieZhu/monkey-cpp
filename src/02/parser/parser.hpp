#ifndef H_PARSER_H
#define H_PARSER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "lexer/lexer.hpp"
#include "token/token.hpp"
#include "ast/ast.hpp"

namespace parser
{

    enum class Priority
    {
        LOWEST = 1,
        EQUALS,      // ==
        LESSGREATER, // > or <
        SUM,         // +
        PRODUCT,     // *
        PREFIX,      // -X or !X
        CALL         // myFunction(X)
    };

    std::map<token::TokenType, Priority> precedences{{token::types::EQ, Priority::EQUALS},
                                                {token::types::NOT_EQ, Priority::EQUALS},
                                                {token::types::LT, Priority::LESSGREATER},
                                                {token::types::GT, Priority::LESSGREATER},
                                                {token::types::PLUS, Priority::SUM},
                                                {token::types::MINUS, Priority::SUM},
                                                {token::types::SLASH, Priority::PRODUCT},
                                                {token::types::ASTERISK, Priority::PRODUCT},
                                                {token::types::LPAREN, Priority::CALL}};

    struct Parser
    {
        using prefixParseFn = std::unique_ptr<ast::Expression> (Parser::*)();
        using infixParseFn = std::unique_ptr<ast::Expression> (Parser::*)(std::unique_ptr<ast::Expression>);

        std::unique_ptr<lexer::Lexer> pLexer;
        std::vector<std::string> errors;

        token::Token curToken;
        token::Token peekToken;

        std::map<token::TokenType, prefixParseFn> prefixParseFns;
        std::map<token::TokenType, infixParseFn> infixParseFns;

        Parser()
        {
            /*
            prefixParseFns.insert(std::make_pair(token::types::IDENT, (prefixParseFn)(&Parser::parseIdentifier)));
            prefixParseFns.insert(std::make_pair(token::types::INT, (prefixParseFn)(&Parser::parseIntegerLiteral)));
            prefixParseFns.insert(std::make_pair(token::types::BANG, (prefixParseFn)(&Parser::parsePrefixExpression)));
            prefixParseFns.insert(std::make_pair(token::types::MINUS, (prefixParseFn)(&Parser::parsePrefixExpression)));
            prefixParseFns.insert(std::make_pair(token::types::TRUE, (prefixParseFn)(&Parser::parseBoolean)));
            prefixParseFns.insert(std::make_pair(token::types::FALSE, (prefixParseFn)(&Parser::parseBoolean)));
            prefixParseFns.insert(std::make_pair(token::types::LPAREN, (prefixParseFn)(&Parser::parseGroupedExpression)));
            prefixParseFns.insert(std::make_pair(token::types::IF, (prefixParseFn)(&Parser::parseIfExpression)));
            prefixParseFns.insert(std::make_pair(token::types::FUNCTION, (prefixParseFn)(&Parser::parseFunctionLiteral)));

            infixParseFns.insert(std::make_pair(token::types::PLUS, (infixParseFn)(&Parser::parseInfixExpression)));
            infixParseFns.insert(std::make_pair(token::types::MINUS, (infixParseFn)(&Parser::parseInfixExpression)));
            infixParseFns.insert(std::make_pair(token::types::SLASH, (infixParseFn)(&Parser::parseInfixExpression)));
            infixParseFns.insert(std::make_pair(token::types::ASTERISK, (infixParseFn)(&Parser::parseInfixExpression)));
            infixParseFns.insert(std::make_pair(token::types::EQ, (infixParseFn)(&Parser::parseInfixExpression)));
            infixParseFns.insert(std::make_pair(token::types::NOT_EQ, (infixParseFn)(&Parser::parseInfixExpression)));
            infixParseFns.insert(std::make_pair(token::types::LT, (infixParseFn)(&Parser::parseInfixExpression)));
            infixParseFns.insert(std::make_pair(token::types::GT, (infixParseFn)(&Parser::parseInfixExpression)));
            infixParseFns.insert(std::make_pair(token::types::LPAREN, (infixParseFn)(&Parser::parseCallExpression)));
            */
        }

        void nextToken()
        {
            curToken = peekToken;
            peekToken = pLexer->NextToken();
        }

        bool curTokenIs(token::TokenType t)
        {
            return (curToken.Type == t);
        }

        bool peekTokenIs(token::TokenType t)
        {
            return (peekToken.Type == t);
        }

        bool expectPeek(token::TokenType t)
        {
            if (peekTokenIs(t))
            {
                nextToken();
                return true;
            }
            else
            {
                peekError(t);
                return false;
            }
        }

        std::vector<std::string> Errors()
        {
            return errors;
        }

        void peekError(token::TokenType t)
        {
            std::string msg = "expected next token to be " + t + ", got " + peekToken.Type + " instead";
            errors.push_back(msg);
        }

        void noPrefixParseFnError(token::TokenType t)
        {
            std::string msg = "no prefix parse function for " + t + " found";
            errors.push_back(msg);
        }

        std::unique_ptr<ast::Program> ParseProgram()
        {
            std::unique_ptr<ast::Program> pProgram = std::make_unique<ast::Program>();
            pProgram->v_pStatements.clear();

            while (!curTokenIs(token::types::EndOF))
            {
                std::unique_ptr<ast::Statement> pStmt{parseStatement()};

                if (pStmt)
                {
                    pProgram->v_pStatements.push_back(std::move(pStmt));
                }
                nextToken();
            }
            return pProgram;
        }

        std::unique_ptr<ast::Statement> parseStatement()
        {
            if (curToken.Type == token::types::LET)
            {
                return parseLetStatement();
            }
            else if (curToken.Type == token::types::RETURN)
            {
                return parseReturnStatement();
            }
            else
            {
                return parseExpressionStatement();
            }
        }

        std::unique_ptr<ast::LetStatement> parseLetStatement()
        {
            std::unique_ptr<ast::LetStatement> pStmt = std::make_unique<ast::LetStatement>();
            pStmt->Token = curToken;

            if (!expectPeek(token::types::IDENT))
            {
                return nullptr;
            }

            pStmt->pName = std::make_unique<ast::Identifier>(curToken, curToken.Literal);

            if (!expectPeek(token::types::ASSIGN))
            {
                return nullptr;
            }
            nextToken();

            pStmt->pValue = parseExpression(Priority::LOWEST);

            if (peekTokenIs(token::types::SEMICOLON))
            {
                nextToken();
            }

            return pStmt;
        }

        std::unique_ptr<ast::ReturnStatement> parseReturnStatement()
        {
            std::unique_ptr<ast::ReturnStatement> pStmt = std::make_unique<ast::ReturnStatement>(curToken);
            nextToken();
            pStmt->pReturnValue = parseExpression(Priority::LOWEST);
            if (peekTokenIs(token::types::SEMICOLON))
            {
                nextToken();
            }
            return pStmt;
        }

        std::unique_ptr<ast::ExpressionStatement> parseExpressionStatement()
        {
            std::unique_ptr<ast::ExpressionStatement> pStmt = std::make_unique<ast::ExpressionStatement>(curToken);
            pStmt->pExpression = parseExpression(Priority::LOWEST);
            if (peekTokenIs(token::types::SEMICOLON))
            {
                nextToken();
            }
            return pStmt;
        }

        std::unique_ptr<ast::Expression> parseExpression(Priority precedence)
        {
            prefixParseFn prefix = prefixParseFns[curToken.Type];
            if (prefix == nullptr)
            {
                noPrefixParseFnError(curToken.Type);
                return nullptr;
            }

            //std::unique_ptr<ast::Expression> leftExp = (this->*(prefix))();
            std::unique_ptr<ast::Expression> leftExp = (this->*prefix)();
            while (!peekTokenIs(token::types::SEMICOLON) && precedence < peekPrecedence())
            {
                infixParseFn infix = infixParseFns[peekToken.Type];
                if (!infix)
                {
                    return leftExp;
                }

                nextToken();
                //leftExp = (this->*(infix))(std::move(leftExp));
                leftExp = (this->*infix)(std::move(leftExp));
            }
            return leftExp;
        }

        Priority peekPrecedence()
        {
            auto fit = precedences.find(peekToken.Type);
            if (fit != precedences.end())
            {
                return fit->second;
            }

            return Priority::LOWEST;
        }

        Priority curPrecedence()
        {
            auto fit = precedences.find(curToken.Type);
            if (fit != precedences.end())
            {
                return fit->second;
            }
            return Priority::LOWEST;
        }

        std::unique_ptr<ast::Expression> parseIdentifier()
        {
            std::unique_ptr<ast::Identifier> pStmt = std::make_unique<ast::Identifier>(curToken, curToken.Literal);
            return pStmt;
        }

        std::unique_ptr<ast::Expression> parseIntegerLiteral()
        {
            std::unique_ptr<ast::IntegerLiteral> pLit = std::make_unique<ast::IntegerLiteral>(curToken);

            try
            {
                long long int value = std::stoll(curToken.Literal);
                pLit->Value = value;
            }
            catch (...)
            {
                std::string msg = "could not parse " + curToken.Literal + " as integer";
                errors.push_back(msg);
                return nullptr;
            }

            return pLit;
        }

        std::unique_ptr<ast::Expression> parsePrefixExpression()
        {
            std::unique_ptr<ast::PrefixExpression> pExpression = std::make_unique<ast::PrefixExpression>(curToken, curToken.Literal);
            nextToken();
            pExpression->pRight = parseExpression(Priority::PREFIX);
            return pExpression;
        }

        std::unique_ptr<ast::Expression> parseInfixExpression(std::unique_ptr<ast::Expression> left)
        {
            std::unique_ptr<ast::InfixExpression> pExpression = std::make_unique<ast::InfixExpression>(curToken, curToken.Literal, std::move(left));
            Priority precedence = curPrecedence();
            nextToken();
            pExpression->pRight = parseExpression(precedence);
            return pExpression;
        }

        std::unique_ptr<ast::Expression> parseBoolean()
        {
            std::unique_ptr<ast::Boolean> pStmt = std::make_unique<ast::Boolean>(curToken, curTokenIs(token::types::TRUE));
            return pStmt;
        }

        std::unique_ptr<ast::Expression> parseGroupedExpression()
        {
            nextToken();
            std::unique_ptr<ast::Expression> pExp = parseExpression(Priority::LOWEST);
            if (!expectPeek(token::types::RPAREN))
            {
                return nullptr;
            }
            return pExp;
        }

        std::unique_ptr<ast::Expression> parseIfExpression()
        {
            std::unique_ptr<ast::IfExpression> pExpression = std::make_unique<ast::IfExpression>(curToken);
            if (!expectPeek(token::types::LPAREN))
            {
                return nullptr;
            }
            nextToken();
            pExpression->pCondition = parseExpression(Priority::LOWEST);
            if (!expectPeek(token::types::RPAREN))
            {
                return nullptr;
            }
            if (!expectPeek(token::types::LBRACE))
            {
                return nullptr;
            }
            pExpression->pConsequence = parseBlockStatement();
            if (peekTokenIs(token::types::ELSE))
            {
                nextToken();
                if (!expectPeek(token::types::LBRACE))
                {
                    return nullptr;
                }
                pExpression->pAlternative = parseBlockStatement();
            }
            return pExpression;
        }

        std::unique_ptr<ast::BlockStatement> parseBlockStatement()
        {
            std::unique_ptr<ast::BlockStatement> pBlock = std::make_unique<ast::BlockStatement>(curToken);
            nextToken();
            while (!curTokenIs(token::types::RBRACE) && !curTokenIs(token::types::EndOF))
            {
                std::unique_ptr<ast::Statement> pStmt = parseStatement();
                if (pStmt != nullptr)
                {
                    pBlock->v_pStatements.push_back(std::move(pStmt));
                }
                nextToken();
            }

            return pBlock;
        }

        std::unique_ptr<ast::Expression> parseFunctionLiteral()
        {
            std::unique_ptr<ast::FunctionLiteral> pLit = std::make_unique<ast::FunctionLiteral>(curToken);
            if (!expectPeek(token::types::LPAREN))
            {
                return nullptr;
            }
            pLit->v_pParameters = parseFunctionParameters();

            if (!expectPeek(token::types::LBRACE))
            {
                return nullptr;
            }

            pLit->pBody = parseBlockStatement();

            return pLit;
        }

        std::vector<std::unique_ptr<ast::Identifier>> parseFunctionParameters()
        {
            std::vector<std::unique_ptr<ast::Identifier>> v_pIdentifiers{};

            if (peekTokenIs(token::types::RPAREN))
            {
                nextToken();
                return v_pIdentifiers;
            }
            nextToken();
            std::unique_ptr<ast::Identifier> pIdent = std::make_unique<ast::Identifier>(curToken, curToken.Literal);
            v_pIdentifiers.push_back(std::move(pIdent));

            while (peekTokenIs(token::types::COMMA))
            {
                nextToken();
                nextToken();
                std::unique_ptr<ast::Identifier> pIdent = std::make_unique<ast::Identifier>(curToken, curToken.Literal);
                v_pIdentifiers.push_back(std::move(pIdent));
            }
            if (!expectPeek(token::types::RPAREN))
            {
                return std::vector<std::unique_ptr<ast::Identifier>>{};
            }
            return v_pIdentifiers;
        }

        std::unique_ptr<ast::Expression> parseCallExpression(std::unique_ptr<ast::Expression> function)
        {
            std::unique_ptr<ast::CallExpression> pExp = std::make_unique<ast::CallExpression>(curToken, std::move(function));
            pExp->pArguments = parseCallArguments();
            return pExp;
        }

        std::vector<std::unique_ptr<ast::Expression>> parseCallArguments()
        {
            std::vector<std::unique_ptr<ast::Expression>> args{};
            if (peekTokenIs(token::types::RPAREN))
            {
                nextToken();
                return args;
            }
            nextToken();
            args.push_back(parseExpression(Priority::LOWEST));
            while (peekTokenIs(token::types::COMMA))
            {
                nextToken();
                nextToken();
                args.push_back(parseExpression(Priority::LOWEST));
            }
            if (!expectPeek(token::types::RPAREN))
            {
                return std::vector<std::unique_ptr<ast::Expression>>{};
            }
            return args;
        }

        void registerPrefix(token::TokenType tokenType, prefixParseFn fn)
        {
            prefixParseFns[tokenType] = fn;
        }

        void registerInfix(token::TokenType tokenType, infixParseFn fn)
        {
            infixParseFns[tokenType] = fn;
        }
    };

    std::unique_ptr<Parser> New(std::unique_ptr<lexer::Lexer> pLexer)
    {
        std::unique_ptr<Parser> pParser = std::make_unique<Parser>();
        pParser->pLexer = std::move(pLexer);
        pParser->errors.clear();

        pParser->prefixParseFns.clear();
        pParser->registerPrefix(token::types::IDENT, &Parser::parseIdentifier);
        pParser->registerPrefix(token::types::INT, &Parser::parseIntegerLiteral);
        pParser->registerPrefix(token::types::BANG, &Parser::parsePrefixExpression);
        pParser->registerPrefix(token::types::MINUS, &Parser::parsePrefixExpression);
        pParser->registerPrefix(token::types::TRUE, &Parser::parseBoolean);
        pParser->registerPrefix(token::types::FALSE, &Parser::parseBoolean);
        pParser->registerPrefix(token::types::LPAREN, &Parser::parseGroupedExpression);
        pParser->registerPrefix(token::types::IF, &Parser::parseIfExpression);
        pParser->registerPrefix(token::types::FUNCTION, &Parser::parseFunctionLiteral);

        pParser->infixParseFns.clear();
        pParser->registerInfix(token::types::PLUS, &Parser::parseInfixExpression);
        pParser->registerInfix(token::types::MINUS, &Parser::parseInfixExpression);
        pParser->registerInfix(token::types::SLASH, &Parser::parseInfixExpression);
        pParser->registerInfix(token::types::ASTERISK, &Parser::parseInfixExpression);
        pParser->registerInfix(token::types::EQ, &Parser::parseInfixExpression);
        pParser->registerInfix(token::types::NOT_EQ, &Parser::parseInfixExpression);
        pParser->registerInfix(token::types::LT, &Parser::parseInfixExpression);
        pParser->registerInfix(token::types::GT, &Parser::parseInfixExpression);
        pParser->registerInfix(token::types::LPAREN, &Parser::parseCallExpression);

        // Read two tokens, so curToken and peekToken are both set
        pParser->nextToken();
        pParser->nextToken();
        return pParser;
    }

}

#endif // H_PARSER_H

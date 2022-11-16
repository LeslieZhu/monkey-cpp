#ifndef H_AST_H
#define H_AST_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <memory>

#include "token/token.hpp"

namespace ast
{

    std::string Join(std::vector<std::string> vStr, std::string dlim)
    {
        std::stringstream oss;

        const int size = vStr.size();
        int i = 0;

        for (const auto &str : vStr)
        {
            oss << str;
            if (i != size - 1)
            {
                oss << dlim;
            }
            i++;
        }
        return oss.str();
    }

    enum class NodeType
    {
        Base, // 基类

        Statement,  // 语句类型
        Expression, // 表达式类型

        Identifier,          // 标志符
        LetStatement,        // Let语句
        ReturnStatement,     // Return语句
        ExpressionStatement, // 表达式
        BlockStatement,      // 代码块
        Boolean,             // 布尔
        IntegerLiteral,      // 整数
        PrefixExpression,    // 前缀表达式
        InfixExpression,     // 中缀表达式
        IfExpression,        // If表达式
        FunctionLiteral,     // 函数定义
        CallExpression,      // 调用

        Program // 程序
    };

    // The base Node interface
    struct Node
    {
        virtual ~Node() {}
        virtual std::string TokenLiteral() { return ""; }
        virtual std::string String() { return ""; }
        virtual bool Good() { return true; }
        virtual NodeType GetNodeType() { return ast::NodeType::Base; }
    };

    // All statement nodes implement this
    struct Statement : Node
    {
        virtual ~Statement() {}
        virtual void StatementNode() {}
        virtual NodeType GetNodeType() { return ast::NodeType::Statement; }
    };

    // All expression nodes implement this
    struct Expression : Node
    {
        virtual ~Expression() {}
        virtual void ExpressionNode() {}
        virtual NodeType GetNodeType() { return ast::NodeType::Expression; }
    };

    struct Identifier : Expression
    {
        token::Token Token; // the token.IDENT token
        std::string Value;

        Identifier(){}
        Identifier(token::Token tok, std::string literal) : Token(tok), Value(literal) {}

        virtual ~Identifier() {}
        virtual void ExpressionNode() {}
        virtual std::string TokenLiteral() { return Token.Literal; }
        virtual std::string String() { return Value; }
        virtual NodeType GetNodeType() { return ast::NodeType::Identifier; }
    };

    // ========== Statements  ==============

    struct LetStatement : Statement
    {
        token::Token Token; // the token.LET token
        std::unique_ptr<Identifier> pName;
        std::unique_ptr<Expression> pValue;

        virtual ~LetStatement() {}
        virtual void StatementNode() {}

        virtual std::string TokenLiteral()
        {
            return Token.Literal;
        }

        virtual std::string String()
        {
            std::stringstream oss;
            oss << TokenLiteral() + " "
                << pName->String()
                << " = ";

            if (pValue->Good())
            {
                oss << pValue->String();
            }

            oss << ";";

            return oss.str();
        }
        virtual NodeType GetNodeType() { return ast::NodeType::LetStatement; }
    };

    struct ReturnStatement : Statement
    {
        token::Token Token; // the 'return' token
        std::unique_ptr<Expression> pReturnValue;

        ReturnStatement(token::Token tok) : Token(tok), pReturnValue(nullptr) {}
        virtual ~ReturnStatement() {}

        virtual void StatementNode() {}

        virtual std::string TokenLiteral() { return Token.Literal; }

        virtual std::string String()
        {
            std::stringstream oss;
            oss << TokenLiteral() + " ";
            if (pReturnValue->Good())
            {
                oss << pReturnValue->String();
            }
            oss << ";";

            return oss.str();
        }

        virtual NodeType GetNodeType() { return ast::NodeType::ReturnStatement; }
    };

    struct ExpressionStatement : Statement
    {
        token::Token Token; // the first token of the expression
        std::unique_ptr<Expression> pExpression;

        ExpressionStatement(token::Token tok) : Token(tok), pExpression(nullptr) {}
        virtual ~ExpressionStatement() {}

        virtual void StatementNode() {}

        virtual std::string TokenLiteral() { return Token.Literal; }

        virtual std::string String()
        {
            if (pExpression->Good())
            {
                return pExpression->String();
            }

            return "";
        }

        virtual NodeType GetNodeType() { return ast::NodeType::ExpressionStatement; }
    };

    struct BlockStatement : Statement
    {
        token::Token Token; // the { token
        std::vector<std::unique_ptr<Statement>> v_pStatements;

        BlockStatement(token::Token tok) : Token(tok) {}
        virtual ~BlockStatement() {}

        virtual void StatementNode() {}

        virtual std::string TokenLiteral() { return Token.Literal; }
        virtual std::string String()
        {
            std::stringstream oss;
            for (auto &pStmt : v_pStatements)
            {
                oss << pStmt->String();
            }
            return oss.str();
        }

        virtual NodeType GetNodeType() { return ast::NodeType::BlockStatement; }
    };

    // ======== Expressions ============

    struct Boolean : Expression
    {
        token::Token Token;
        bool Value;

        Boolean(token::Token tok, bool val) : Token(tok), Value(val) {}
        virtual ~Boolean() {}

        virtual void ExpressionNode() {}
        virtual std::string TokenLiteral() { return Token.Literal; }
        virtual std::string String() { return Token.Literal; }
        virtual NodeType GetNodeType() { return ast::NodeType::Boolean; }
    };

    struct IntegerLiteral : Expression
    {
        token::Token Token;
        long long int Value;

        IntegerLiteral(token::Token tok) : Token(tok) {}
        virtual ~IntegerLiteral() {}

        virtual void ExpressionNode() {}
        virtual std::string TokenLiteral() { return Token.Literal; }
        virtual std::string String() { return Token.Literal; }
        virtual NodeType GetNodeType() { return ast::NodeType::IntegerLiteral; }
    };

    struct PrefixExpression : Expression
    {
        token::Token Token; // The prefix token, e.g. !
        std::string Operator;
        std::unique_ptr<Expression> pRight;

        PrefixExpression(token::Token tok, std::string literal) : Token(tok), Operator(literal) {}
        virtual ~PrefixExpression() {}

        virtual void ExpressionNode() {}
        virtual std::string TokenLiteral() { return Token.Literal; }
        virtual std::string String()
        {
            std::stringstream oss;
            oss << "(" << Operator << pRight->String() << ")";
            return oss.str();
        }
        virtual NodeType GetNodeType() { return ast::NodeType::PrefixExpression; }
    };

    struct InfixExpression : Expression
    {
        token::Token Token; // The operator token, e.g. +
        std::unique_ptr<Expression> pLeft;
        std::string Operator;
        std::unique_ptr<Expression> pRight;

        InfixExpression(token::Token tok, std::string literal, std::unique_ptr<Expression> left) : Token(tok), pLeft(std::move(left)), Operator(literal) {}

        virtual ~InfixExpression() {}

        virtual void ExpressionNode() {}
        virtual std::string TokenLiteral() { return Token.Literal; }
        virtual std::string String()
        {
            std::stringstream oss;

            oss << "(" << pLeft->String() << " " + Operator + " " << pRight->String() << ")";
            return oss.str();
        }
        virtual NodeType GetNodeType() { return ast::NodeType::InfixExpression; }
    };

    struct IfExpression : Expression
    {
        token::Token Token; // The 'if' token
        std::unique_ptr<Expression> pCondition;
        std::unique_ptr<BlockStatement> pConsequence;
        std::unique_ptr<BlockStatement> pAlternative;

        IfExpression(token::Token tok) : Token(tok) {}
        virtual ~IfExpression() {}

        virtual void ExpressionNode() {}
        virtual std::string TokenLiteral() { return Token.Literal; }
        virtual std::string String()
        {
            std::stringstream oss;
            oss << "if" << pCondition->String() << " " << pConsequence->String();
            if (pAlternative)
            {
                oss << "else " << pAlternative->String();
            }
            return oss.str();
        }
        virtual NodeType GetNodeType() { return ast::NodeType::IfExpression; }
    };

    struct FunctionLiteral : Expression
    {
        token::Token Token; // The 'fn' token
        std::vector<std::unique_ptr<Identifier>> v_pParameters;
        std::unique_ptr<BlockStatement> pBody;

        FunctionLiteral(token::Token tok) : Token(tok) {}
        virtual ~FunctionLiteral() {}

        virtual void ExpressionNode() {}
        virtual std::string TokenLiteral() { return Token.Literal; }
        virtual std::string String()
        {
            std::stringstream oss;
            std::vector<std::string> params{};
            for (auto &pParam : v_pParameters)
            {
                params.push_back(pParam->String());
            }
            oss << TokenLiteral() << "(" << Join(params, ", ") << ") " << pBody->String();
            return oss.str();
        }
        virtual NodeType GetNodeType() { return ast::NodeType::FunctionLiteral; }
    };

    struct CallExpression : Expression
    {
        token::Token Token;                           // The '(' token
        std::unique_ptr<Expression> pFunction; // Identifier or FunctionLiteral
        std::vector<std::unique_ptr<Expression>> pArguments;

        CallExpression(token::Token tok, std::unique_ptr<Expression> function) : Token(tok), pFunction(std::move(function)) {}
        virtual ~CallExpression() {}

        virtual void ExpressionNode() {}
        virtual std::string TokenLiteral() { return Token.Literal; }
        virtual std::string String()
        {
            std::stringstream oss;
            std::vector<std::string> args;
            for (auto &pArg : pArguments)
            {
                args.push_back(pArg->String());
            }
            oss << pFunction->String() << "(" << Join(args, ", ") << ")";
            return oss.str();
        }
        virtual NodeType GetNodeType() { return ast::NodeType::CallExpression; }
    };

    struct Program
    {
        std::vector<std::unique_ptr<Statement>> v_pStatements;

        std::string TokenLiteral()
        {
            if (v_pStatements.size() > 0)
            {
                return v_pStatements[0]->TokenLiteral();
            }
            else
            {
                return "";
            }
        }

        std::string String()
        {
            std::stringstream oss;
            for (auto &pStmt : v_pStatements)
            {
                oss << pStmt->String();
            }

            return oss.str();
        }

        NodeType GetNodeType() { return ast::NodeType::Program; }
    };

}

#endif // H_AST_H

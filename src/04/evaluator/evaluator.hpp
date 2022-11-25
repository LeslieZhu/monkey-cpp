
#ifndef H_EVALUATOR_H
#define H_EVALUATOR_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>

#include "ast/ast.hpp"
#include "objects/objects.hpp"
#include "objects/environment.hpp"

namespace evaluator
{

	std::shared_ptr<objects::Null> NULL_OBJ = std::make_shared<objects::Null>();
	std::shared_ptr<objects::Boolean> TRUE_OBJ = std::make_shared<objects::Boolean>(true);
	std::shared_ptr<objects::Boolean> FALSE_OBJ = std::make_shared<objects::Boolean>(false);

	std::shared_ptr<objects::Object> Eval(std::shared_ptr<ast::Node> node, std::shared_ptr<objects::Environment> env);

	std::shared_ptr<objects::Error> newError(std::string msg)
	{
		std::shared_ptr<objects::Error> error = std::make_shared<objects::Error>();
		error->Message = msg;
		return error;
	}

	bool isError(std::shared_ptr<objects::Object> obj)
	{
		if (obj != nullptr)
		{
			return (obj->Type() == objects::ObjectType::ERROR);
		}
		return false;
	}

	bool isTruthy(std::shared_ptr<objects::Object> obj)
	{
		if (obj == NULL_OBJ)
		{
			return false;
		}
		else if (obj == TRUE_OBJ)
		{
			return true;
		}
		else if (obj == FALSE_OBJ)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	std::shared_ptr<objects::Boolean> nativeBoolToBooleanObject(bool input)
	{
		if (input)
		{
			return TRUE_OBJ;
		}
		else
		{
			return FALSE_OBJ;
		}
	}

	std::shared_ptr<objects::Object> unwrapReturnValue(std::shared_ptr<objects::Object> obj)
	{
		std::shared_ptr<objects::ReturnValue> returnValue = std::dynamic_pointer_cast<objects::ReturnValue>(obj);
		if (returnValue != nullptr)
		{
			return returnValue->Value;
		}
		else
		{
			return obj;
		}
	}

	std::shared_ptr<objects::Environment> extendFunctionEnv(std::shared_ptr<objects::Function> fn, std::vector<std::shared_ptr<objects::Object>> &args)
	{
		std::shared_ptr<objects::Environment> env = objects::NewEnclosedEnvironment(fn->Env);
		for (unsigned long i = 0; i < fn->Parameters.size(); i++)
		{
			env->Set(fn->Parameters[i]->Value, args[i]);
		}
		return env;
	}

	std::shared_ptr<objects::Object> applyFunction(std::shared_ptr<objects::Object> fn, std::vector<std::shared_ptr<objects::Object>> &args)
	{
		std::shared_ptr<objects::Function> function = std::dynamic_pointer_cast<objects::Function>(fn);
		if (function == nullptr)
		{
			return newError("not a function: " + fn->TypeStr());
		}

		std::shared_ptr<objects::Environment> extendedEnv = extendFunctionEnv(function, args);
		std::shared_ptr<objects::Object> evaluated = Eval(function->Body, extendedEnv);
		return unwrapReturnValue(evaluated);
	}

	std::vector<std::shared_ptr<objects::Object>> evalExpressions(std::vector<std::shared_ptr<ast::Expression>> exps, std::shared_ptr<objects::Environment> env)
	{
		std::vector<std::shared_ptr<objects::Object>> result;

		for (auto &e : exps)
		{
			std::shared_ptr<objects::Object> evaluated = Eval(e, env);
			if (isError(evaluated))
			{
				std::vector<std::shared_ptr<objects::Object>> x;
				x.push_back(evaluated);
				return x;
			}
			result.push_back(evaluated);
		}

		return result;
	}

	std::shared_ptr<objects::Object> evalIdentifier(std::shared_ptr<ast::Identifier> node, std::shared_ptr<objects::Environment> env)
	{
#ifdef DEBUG
		std::cout << "\t\t evalIdentifier get by :" << node->Value << std::endl;
#endif
		std::shared_ptr<objects::Object> val = env->Get(node->Value);
		if (val == nullptr)
		{
			return newError("identifier not found: " + node->Value);
		}
#ifdef DEBUG
		std::cout << "\t\t evalIdentifier get: [" << val->Inspect() << "]" << std::endl;
#endif
		return val;
	}

	std::shared_ptr<objects::Object> evalIfExpression(std::shared_ptr<ast::IfExpression> ie, std::shared_ptr<objects::Environment> env)
	{
		std::shared_ptr<objects::Object> condition = Eval(ie->pCondition, env);
		if (isError(condition))
		{
			return condition;
		}
		if (isTruthy(condition))
		{
			return Eval(ie->pConsequence, env);
		}
		else if (ie->pAlternative != nullptr)
		{
			return Eval(ie->pAlternative, env);
		}
		else
		{
			return NULL_OBJ;
		}
	}

	std::shared_ptr<objects::Object> evalIntegerInfixExpression(std::string ops, std::shared_ptr<objects::Object> left, std::shared_ptr<objects::Object> right)
	{
		long long int leftValue = std::dynamic_pointer_cast<objects::Integer>(left)->Value;
		long long int rightValue = std::dynamic_pointer_cast<objects::Integer>(right)->Value;

		std::shared_ptr<objects::Integer> result = std::make_shared<objects::Integer>();

		if (ops == "+")
		{
			result->Value = leftValue + rightValue;
			return result;
		}
		else if (ops == "-")
		{
			result->Value = leftValue - rightValue;
			return result;
		}
		else if (ops == "*")
		{
			result->Value = leftValue * rightValue;
			return result;
		}
		else if (ops == "/")
		{
			result->Value = leftValue / rightValue;
			return result;
		}
		else if (ops == "<")
		{
			return nativeBoolToBooleanObject(leftValue < rightValue);
		}
		else if (ops == ">")
		{
			return nativeBoolToBooleanObject(leftValue > rightValue);
		}
		else if (ops == "==")
		{
			return nativeBoolToBooleanObject(leftValue == rightValue);
		}
		else if (ops == "!=")
		{
			return nativeBoolToBooleanObject(leftValue != rightValue);
		}
		else
		{
			return newError("unknown operator: " + left->TypeStr() + " " + ops + " " + right->TypeStr());
		}
	}


	std::shared_ptr<objects::Object> evalStringInfixExpression(std::string ops, std::shared_ptr<objects::Object> left, std::shared_ptr<objects::Object> right)
	{
		if(ops != "+")
		{
			return newError("unknown operator: " + left->TypeStr() + " " + ops + " " + right->TypeStr());
		}
		std::string leftValue = std::dynamic_pointer_cast<objects::String>(left)->Value;
		std::string rightValue = std::dynamic_pointer_cast<objects::String>(right)->Value;

		std::shared_ptr<objects::String> result = std::make_shared<objects::String>(leftValue + rightValue);
		return result;
	}

	std::shared_ptr<objects::Object> evalMinusPrefixOperatorExpression(std::shared_ptr<objects::Object> right)
	{
		if (right->Type() != objects::ObjectType::INTEGER)
		{
			return newError("unknown operator: -" + right->TypeStr());
		}

		long long int value = std::dynamic_pointer_cast<objects::Integer>(right)->Value;
		return std::make_shared<objects::Integer>(-value);
	}

	std::shared_ptr<objects::Object> evalBangOperatorExpression(std::shared_ptr<objects::Object> right)
	{
		if (right == TRUE_OBJ)
		{
			return FALSE_OBJ;
		}
		else if (right == FALSE_OBJ)
		{
			return TRUE_OBJ;
		}
		else if (right == NULL_OBJ)
		{
			return TRUE_OBJ;
		}
		else
		{
			return FALSE_OBJ;
		}
	}

	std::shared_ptr<objects::Object> evalPrefixExpression(std::string ops, std::shared_ptr<objects::Object> right)
	{
		if (ops == "!")
		{
			return evalBangOperatorExpression(right);
		}
		else if (ops == "-")
		{
			return evalMinusPrefixOperatorExpression(right);
		}
		else
		{
			return newError("unknown operator: " + ops + right->TypeStr());
		}
	}

	std::shared_ptr<objects::Object> evalInfixExpression(std::string ops, std::shared_ptr<objects::Object> left, std::shared_ptr<objects::Object> right)
	{
		if (left->Type() == objects::ObjectType::INTEGER && right->Type() == objects::ObjectType::INTEGER)
		{
			return evalIntegerInfixExpression(ops, left, right);
		}
		else if (left->Type() == objects::ObjectType::STRING && right->Type() == objects::ObjectType::STRING)
		{
			return evalStringInfixExpression(ops, left, right);
		}
		else if (ops == "==")
		{
			return nativeBoolToBooleanObject(left == right);
		}
		else if (ops == "!=")
		{
			return nativeBoolToBooleanObject(left != right);
		}
		else if (left->Type() != right->Type())
		{
			return newError("type mismatch: " + left->TypeStr() + " " + ops + " " + right->TypeStr());
		}
		else
		{
			return newError("unknown operator: " + left->TypeStr() + " " + ops + " " + right->TypeStr());
		}
	}

	std::shared_ptr<objects::Object> evalBlockStatement(std::shared_ptr<ast::BlockStatement> block, std::shared_ptr<objects::Environment> env)
	{
		std::shared_ptr<objects::Object> result;

		for (auto &stmt : block->v_pStatements)
		{
			result = Eval(stmt, env);
			if (result != nullptr)
			{
				objects::ObjectType rt = result->Type();
				if (rt == objects::ObjectType::RETURN_VALUE || rt == objects::ObjectType::ERROR)
				{
					return result;
				}
			}
		}

		return result;
	}

	std::shared_ptr<objects::Object> evalProgram(std::shared_ptr<ast::Program> program, std::shared_ptr<objects::Environment> env)
	{
#ifdef DEBUG
		std::cout << "\t evalProgram: [Enter]" << std::endl;
		std::cout << "\t evalProgram: program=" << program->String() << std::endl;
#endif
		std::shared_ptr<objects::Object> result = std::make_shared<objects::Object>();

#ifdef DEBUG
		std::cout << "\t evalProgram: result=" << result->Inspect() << std::endl;
		std::cout << "\t evalProgram: program Statements Size=" << program->v_pStatements.size() << std::endl;
#endif

		for (auto &stmt : program->v_pStatements)
		{

#ifdef DEBUG
			std::cout << "\t\t evalProgram: stmt=" << stmt->String() << std::endl;
#endif

			result = Eval(stmt, env);

			if (result == nullptr)
			{
				continue;
			}

#ifdef DEBUG
			std::cout << "\t\t evalProgram: result=" << result->Inspect() << std::endl;
#endif

			if (result->Type() == objects::ObjectType::RETURN_VALUE)
			{
				return std::dynamic_pointer_cast<objects::ReturnValue>(result)->Value;
			}
			else if (result->Type() == objects::ObjectType::ERROR)
			{
				return result;
			}
		}

		return result;
	}

	std::shared_ptr<objects::Object> Eval(std::shared_ptr<ast::Node> node, std::shared_ptr<objects::Environment> env)
	{
		// Statements
		if (node->GetNodeType() == ast::NodeType::Program)
		{
#ifdef DEBUG
			std::cout << "Eval: Program" << std::endl;
#endif
			std::shared_ptr<ast::Program> program = std::dynamic_pointer_cast<ast::Program>(node);
			return evalProgram(program, env);
		}
		else if (node->GetNodeType() == ast::NodeType::BlockStatement)
		{
#ifdef DEBUG
			std::cout << "Eval: BlockStatement" << std::endl;
#endif
			std::shared_ptr<ast::BlockStatement> blockStmt = std::dynamic_pointer_cast<ast::BlockStatement>(node);
			return evalBlockStatement(blockStmt, env);
		}
		else if (node->GetNodeType() == ast::NodeType::ExpressionStatement)
		{
#ifdef DEBUG
			std::cout << "Eval: ExpressionStatement" << std::endl;
#endif
			std::shared_ptr<ast::ExpressionStatement> exprStmt = std::dynamic_pointer_cast<ast::ExpressionStatement>(node);

#ifdef DEBUG
			std::cout << "\t exprStmt=" << exprStmt->String() << std::endl;
#endif

			return Eval(exprStmt->pExpression, env);
		}
		else if (node->GetNodeType() == ast::NodeType::ReturnStatement)
		{
#ifdef DEBUG
			std::cout << "Eval: ReturnStatement" << std::endl;
#endif
			std::shared_ptr<ast::ReturnStatement> returnStmt = std::dynamic_pointer_cast<ast::ReturnStatement>(node);
			std::shared_ptr<objects::Object> val = Eval(returnStmt->pReturnValue, env);
			if (isError(val))
			{
				return val;
			}
			return std::make_shared<objects::ReturnValue>(val);
		}
		else if (node->GetNodeType() == ast::NodeType::LetStatement)
		{
#ifdef DEBUG
			std::cout << "Eval: LetStatement" << std::endl;
#endif
			std::shared_ptr<ast::LetStatement> lit = std::dynamic_pointer_cast<ast::LetStatement>(node);

#ifdef DEBUG
			std::cout << "\t lit stmt=" << lit->String() << std::endl;
#endif

			std::shared_ptr<objects::Object> val = Eval(lit->pValue, env);

#ifdef DEBUG
			std::cout << "\t lit get val=" << val->Inspect() << std::endl;
#endif

			if (isError(val))
			{
				return val;
			}
#ifdef DEBUG
			std::cout << "\t lit set val to :" << lit->pName->Value << std::endl;
#endif
			env->Set(lit->pName->Value, val);
		}
		// Expressions
		else if (node->GetNodeType() == ast::NodeType::IntegerLiteral)
		{
#ifdef DEBUG
			std::cout << "Eval: IntegerLiteral" << std::endl;
#endif
			std::shared_ptr<ast::IntegerLiteral> integerLiteral = std::dynamic_pointer_cast<ast::IntegerLiteral>(node);

#ifdef DEBUG
			std::cout << "\t integerLiteral Value=" << integerLiteral->Value << std::endl;
#endif
			return std::make_shared<objects::Integer>(integerLiteral->Value);
		}
		else if (node->GetNodeType() == ast::NodeType::Boolean)
		{
#ifdef DEBUG
			std::cout << "Eval: Boolean" << std::endl;
#endif
			return nativeBoolToBooleanObject(std::dynamic_pointer_cast<ast::Boolean>(node)->Value);
		}
		else if(node->GetNodeType() == ast::NodeType::StringLiteral)
		{
			std::shared_ptr<ast::StringLiteral> stringLiteral = std::dynamic_pointer_cast<ast::StringLiteral>(node);
			return std::make_shared<objects::String>(stringLiteral->Value);
		}
		else if (node->GetNodeType() == ast::NodeType::PrefixExpression)
		{
#ifdef DEBUG
			std::cout << "Eval: PrefixExpression" << std::endl;
#endif
			std::shared_ptr<ast::PrefixExpression> infixObj = std::dynamic_pointer_cast<ast::PrefixExpression>(node);

			std::shared_ptr<objects::Object> right = Eval(infixObj->pRight, env);
			if (isError(right))
			{
				return right;
			}
			return evalPrefixExpression(infixObj->Operator, right);
		}
		else if (node->GetNodeType() == ast::NodeType::InfixExpression)
		{
#ifdef DEBUG
			std::cout << "Eval: InfixExpression" << std::endl;
#endif
			std::shared_ptr<ast::InfixExpression> infixObj = std::dynamic_pointer_cast<ast::InfixExpression>(node);

			std::shared_ptr<objects::Object> left = Eval(infixObj->pLeft, env);
			if (isError(left))
			{
				return left;
			}

			std::shared_ptr<objects::Object> right = Eval(infixObj->pRight, env);
			if (isError(right))
			{
				return right;
			}

			return evalInfixExpression(infixObj->Operator, left, right);
		}
		else if (node->GetNodeType() == ast::NodeType::IfExpression)
		{
#ifdef DEBUG
			std::cout << "Eval: IfExpression" << std::endl;
#endif
			std::shared_ptr<ast::IfExpression> x = std::dynamic_pointer_cast<ast::IfExpression>(node);
			return evalIfExpression(x, env);
		}
		else if (node->GetNodeType() == ast::NodeType::Identifier)
		{
#ifdef DEBUG
			std::cout << "Eval: Identifier" << std::endl;
#endif
			std::shared_ptr<ast::Identifier> ident = std::dynamic_pointer_cast<ast::Identifier>(node);

#ifdef DEBUG
			std::cout << "\t ident=" << ident->String() << std::endl;
#endif

			return evalIdentifier(ident, env);
		}
		else if (node->GetNodeType() == ast::NodeType::FunctionLiteral)
		{
#ifdef DEBUG
			std::cout << "Eval: FunctionLiteral" << std::endl;
#endif
			std::shared_ptr<ast::FunctionLiteral> funcObj = std::dynamic_pointer_cast<ast::FunctionLiteral>(node);

			std::shared_ptr<objects::Function> function = std::make_shared<objects::Function>();

			std::for_each(funcObj->v_pParameters.begin(), funcObj->v_pParameters.end(), [&](std::shared_ptr<ast::Identifier> &x)
						  { function->Parameters.push_back(x); });

			function->Env = env;
			function->Body = funcObj->pBody;

			return function;
		}
		else if (node->GetNodeType() == ast::NodeType::CallExpression)
		{
#ifdef DEBUG
			std::cout << "Eval: CallExpression" << std::endl;
#endif
			std::shared_ptr<ast::CallExpression> callObj = std::dynamic_pointer_cast<ast::CallExpression>(node);

			std::shared_ptr<objects::Object> function = Eval(callObj->pFunction, env);
			if (isError(function))
			{
				return function;
			}

			std::vector<std::shared_ptr<objects::Object>> args = evalExpressions(callObj->pArguments, env);
			if (args.size() == 1 && isError(args[0]))
			{
				return args[0];
			}

			return applyFunction(function, args);
		}

		return nullptr;
	}

}

#endif // H_EVALUATOR_H

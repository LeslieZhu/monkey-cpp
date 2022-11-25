#ifndef H_OBJECTS_H
#define H_OBJECTS_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <memory>

#include "ast/ast.hpp"

namespace objects
{

	enum class ObjectType
	{
		Null,
		ERROR,
		INTEGER,
		BOOLEAN,
		STRING,
		RETURN_VALUE,
		FUNCTION
	};

	struct Object
	{
		virtual ~Object() {}
		virtual ObjectType Type() { return ObjectType::Null; }
		virtual std::string Inspect() { return ""; }

		std::string TypeStr()
		{
			switch (Type())
			{
			case ObjectType::Null:
				return "Null";
			case ObjectType::ERROR:
				return "ERROR";
			case ObjectType::INTEGER:
				return "INTEGER";
			case ObjectType::BOOLEAN:
				return "BOOLEAN";
			case ObjectType::RETURN_VALUE:
				return "RETURN_VALUE";
			case ObjectType::FUNCTION:
				return "FUNCTION";
			default:
				return "BadType";
			}
		}
	};

	struct Integer : Object
	{
		long long int Value;

		Integer() {}
		Integer(long long int val) : Value(val) {}

		virtual ~Integer() {}
		virtual ObjectType Type() { return ObjectType::INTEGER; }
		virtual std::string Inspect()
		{
			std::stringstream oss;
			oss << Value;
			return oss.str();
		}
	};

	struct Boolean : Object
	{
		bool Value;

		Boolean(bool val) : Value(val) {}

		virtual ~Boolean() {}
		virtual ObjectType Type() { return ObjectType::BOOLEAN; }
		virtual std::string Inspect()
		{
			std::stringstream oss;
			oss << (Value ? "true" : "false");
			return oss.str();
		}
	};

	struct String : Object
	{
		std::string Value;

		String(std::string val) : Value(val) {}

		virtual ~String() {}
		virtual ObjectType Type() { return ObjectType::STRING; }
		virtual std::string Inspect()
		{
			return Value;
		}
	};

	struct Null : Object
	{
		Null() {}
		virtual ~Null() {}
		virtual ObjectType Type() { return ObjectType::Null; }
		virtual std::string Inspect() { return "null"; }
	};

	struct ReturnValue : Object
	{
		std::shared_ptr<Object> Value;

		ReturnValue(std::shared_ptr<Object> val) : Value(val) {}
		virtual ~ReturnValue() { Value.reset();}
		virtual ObjectType Type() { return ObjectType::RETURN_VALUE; }
		virtual std::string Inspect() { return Value->Inspect(); }
	};

	struct Error : Object
	{
		std::string Message;

		virtual ~Error() {}
		virtual ObjectType Type() { return ObjectType::ERROR; }
		virtual std::string Inspect() { return "ERROR: " + Message; }
	};

}

#endif // H_OBJECTS_H
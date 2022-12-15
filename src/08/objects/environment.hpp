#ifndef H_ENVIRONMENT_H
#define H_ENVIRONMENT_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "objects/objects.hpp"
#include "code/code.hpp"

namespace objects
{

	struct Environment
	{
		std::map<std::string, std::shared_ptr<Object>> store;
		std::shared_ptr<Environment> outer;

		~Environment(){
			store.clear();
			outer.reset();
		}

		std::shared_ptr<Object> Get(std::string name)
		{

#ifdef DEBUG
			std::cout << "store size: " << store.size() << std::endl;
			for (auto &obj : store)
			{
				std::cout << "\t" << obj.first << "->" << obj.second->Inspect() << std::endl;
			}
#endif
			auto fit = store.find(name);
			if (fit != store.end())
			{
#ifdef DEBUG
				std::cout << "\t Get [" << fit->second->Inspect() << "] for " << name << std::endl;
#endif
				return fit->second;
			}
			else if (outer != nullptr)
			{
				return outer->Get(name);
			}
			else
			{
#ifdef DEBUG
				std::cout << "\t Get null of " << name << std::endl;
#endif
				return nullptr;
			}
		}

		std::shared_ptr<Object> Set(std::string name, std::shared_ptr<Object> val)
		{
#ifdef DEBUG
			std::cout << "\t Set get val=" << val->Inspect() << ",Type=" << val->TypeStr() << std::endl;
#endif
			auto fit = store.find(name);
			if (fit != store.end())
			{
#ifdef DEBUG
				std::cout << "\t Set [" << val->Inspect() << "] of " << fit->first << std::endl;
#endif
				fit->second.reset();
				fit->second = val;
			}
			else
			{
#ifdef DEBUG
				std::cout << "\t Set insert [" << val->Inspect() << "] of " << name << std::endl;
#endif
				store.insert(std::make_pair(name, val));
			}

#ifdef DEBUG
            std::cout << "store size: " << store.size() << std::endl;
			for (auto &obj : store)
			{
				std::cout << "\t" << obj.first << "->" << obj.second->Inspect() << std::endl;
			}
#endif

			return val;
		}
	};

	std::shared_ptr<objects::Environment> NewEnvironment()
	{
		std::shared_ptr<Environment> env = std::make_shared<Environment>();
		env->outer = nullptr;
		return env;
	}

	std::shared_ptr<objects::Environment> NewEnclosedEnvironment(std::shared_ptr<objects::Environment> outer)
	{
		std::shared_ptr<objects::Environment> env = NewEnvironment();
		env->outer = outer;
		return env;
	}

	struct Function : Object
	{
		std::vector<std::shared_ptr<ast::Identifier>> Parameters;
		std::shared_ptr<ast::BlockStatement> Body;
		std::shared_ptr<Environment> Env;

		virtual ~Function() {
			Parameters.clear();
			Body.reset();
			Env.reset();
		}
		virtual ObjectType Type() { return ObjectType::FUNCTION; }
		virtual std::string Inspect()
		{
			std::stringstream oss;

			std::vector<std::string> params{};

			for (auto &p : Parameters)
			{
				params.push_back(p->String());
			}

			oss << "fn(" << ast::Join(params, ", ") << ") {\n"
				<< Body->String() << "\n}";
			return oss.str();
		}
	};

	struct CompiledFunction: Object
	{
		bytecode::Instructions Instructions;
		int NumLocals;
		int NumParameters;

		CompiledFunction(bytecode::Instructions &ins, const int &numLocals, const int &numParameters)
			: Instructions(ins),
			  NumLocals(numLocals),
			  NumParameters(numParameters)
		{
		}

		virtual ObjectType Type() { return ObjectType::COMPILED_FUNCTION; }
		virtual std::string Inspect()
		{
			std::stringstream oss;
			oss << "CompiledFunction[" << this << "]";
			return oss.str();
		}
	};
}

#endif // H_ENVIRONMENT_H

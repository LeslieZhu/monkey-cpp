#ifndef H_ENVIRONMENT_H
#define H_ENVIRONMENT_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "objects/objects.hpp"

namespace objects
{

	struct Environment
	{
		std::map<std::string, std::shared_ptr<Object>> store;
		std::shared_ptr<Environment> outer;

		std::shared_ptr<Object> Get(std::string name)
		{

			for (auto &obj : store)
			{
				std::cout << "\t" << obj.first << "->" << obj.second->Inspect() << std::endl;
			}

			auto fit = store.find(name);
			if (fit != store.end())
			{
				std::cout << "\t Get [" << fit->second->Inspect() << "] of " << name << std::endl;
				return fit->second;
			}
			else if (outer != nullptr)
			{
				return outer->Get(name);
			}
			else
			{
				std::cout << "\t Get null of " << name << std::endl;
				return nullptr;
			}
		}

		std::shared_ptr<Object> Set(std::string name, std::shared_ptr<Object> val)
		{
			std::cout << "\t Set get val=" << val->Inspect() << ",Type=" << val->TypeStr() << std::endl;

			auto fit = store.find(name);
			if (fit != store.end())
			{
				std::cout << "\t Set [" << val->Inspect() << "] of " << fit->first << std::endl;
				fit->second = val;
			}
			else
			{
				std::cout << "\t Set insert [" << val->Inspect() << "] of " << name << std::endl;
				store.insert(std::make_pair(name, val));
			}

			for (auto &obj : store)
			{
				std::cout << "\t" << obj.first << "->" << obj.second->Inspect() << std::endl;
			}

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

		virtual ~Function() {}
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
}

#endif // H_ENVIRONMENT_H

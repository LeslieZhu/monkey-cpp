#ifndef H_OBJECTS_BUILTINS_H
#define H_OBJECTS_BUILTINS_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "objects/objects.hpp"

namespace objects
{
    int fibonacci(const int& num){
        if(num == 0){
            return 0;
        } else if(num == 1){
            return 1;
        } else {
            return fibonacci(num - 1) + fibonacci(num - 2);
        }
    }

    using BuiltinFunction = std::shared_ptr<objects::Object> (*)(std::vector<std::shared_ptr<objects::Object>>& args);

	struct Builtin: Object
	{
		BuiltinFunction Fn;

		Builtin(BuiltinFunction fn): Fn(fn){}
		virtual ~Builtin(){}
		virtual ObjectType Type() { return ObjectType::BUILTIN; }
		virtual std::string Inspect() { return "builltin function"; }
	};

    std::shared_ptr<objects::Object> BuiltinFunc_Len([[maybe_unused]] std::vector<std::shared_ptr<objects::Object>>& args)
    {
        if(args.size() != 1)
        {
            return objects::newError("wrong number of arguments. got=" + std::to_string(args.size()) + ", want=1");
        }

        if(std::shared_ptr<objects::String> obj = std::dynamic_pointer_cast<objects::String>(args[0]); obj != nullptr)
        {
            return std::make_shared<objects::Integer>(obj->Value.size());
        }
        else if(std::shared_ptr<objects::Array> obj = std::dynamic_pointer_cast<objects::Array>(args[0]); obj != nullptr)
        {
            return std::make_shared<objects::Integer>(obj->Elements.size());
        }
        else
        {
            return objects::newError("argument to `len` not supported, got " + args[0]->TypeStr());
        }
    }

    std::shared_ptr<objects::Object> BuiltinFunc_First([[maybe_unused]] std::vector<std::shared_ptr<objects::Object>>& args)
    {
        if(args.size() != 1)
        {
            return objects::newError("wrong number of arguments. got=" + std::to_string(args.size()) + ", want=1");
        }

        if(std::shared_ptr<objects::Array> obj = std::dynamic_pointer_cast<objects::Array>(args[0]); obj != nullptr)
        {
            if(obj->Elements.size() > 0)
            {
                return obj->Elements[0];
            } else {
                return nullptr;
            }
        }
        else
        {
            return objects::newError("argument to `first` must be ARRAY, got " + args[0]->TypeStr());
        }
    }

    std::shared_ptr<objects::Object> BuiltinFunc_Last([[maybe_unused]] std::vector<std::shared_ptr<objects::Object>>& args)
    {
        if(args.size() != 1)
        {
            return objects::newError("wrong number of arguments. got=" + std::to_string(args.size()) + ", want=1");
        }

        if(std::shared_ptr<objects::Array> obj = std::dynamic_pointer_cast<objects::Array>(args[0]); obj != nullptr)
        {
            auto len = obj->Elements.size();
            if(len > 0)
            {
                return obj->Elements[len - 1];
            } else {
                return nullptr;
            }
        }
        else
        {
            return objects::newError("argument to `last` must be ARRAY, got " + args[0]->TypeStr());
        }
    }

    std::shared_ptr<objects::Object> BuiltinFunc_Rest([[maybe_unused]] std::vector<std::shared_ptr<objects::Object>>& args)
    {
        if(args.size() != 1)
        {
            return objects::newError("wrong number of arguments. got=" + std::to_string(args.size()) + ", want=1");
        }

        if(std::shared_ptr<objects::Array> obj = std::dynamic_pointer_cast<objects::Array>(args[0]); obj != nullptr)
        {
            auto len = obj->Elements.size();
            if(len > 0)
            {
                std::vector<std::shared_ptr<objects::Object>> elements;
                std::copy(obj->Elements.begin()+1, obj->Elements.end(), back_inserter(elements));
                return std::make_shared<objects::Array>(elements);
            } else {
                return nullptr;
            }
        }
        else
        {
            return objects::newError("argument to `rest` must be ARRAY, got " + args[0]->TypeStr());
        }
    }

    std::shared_ptr<objects::Object> BuiltinFunc_Push([[maybe_unused]] std::vector<std::shared_ptr<objects::Object>>& args)
    {
        if(args.size() != 2)
        {
            return objects::newError("wrong number of arguments. got=" + std::to_string(args.size()) + ", want=2");
        }

        if(std::shared_ptr<objects::Array> obj = std::dynamic_pointer_cast<objects::Array>(args[0]); obj != nullptr)
        {
            std::vector<std::shared_ptr<objects::Object>> elements;
            std::copy(obj->Elements.begin(), obj->Elements.end(), back_inserter(elements));
            elements.push_back(args[1]);
            return std::make_shared<objects::Array>(elements);
        }
        else
        {
            return objects::newError("argument to `push` must be ARRAY, got " + args[0]->TypeStr());
        }
    }

    std::shared_ptr<objects::Object> BuiltinFunc_Puts([[maybe_unused]] std::vector<std::shared_ptr<objects::Object>>& args)
    {
        for(const auto& obj: args)
        {
            std::cout << obj->Inspect() << std::endl;
        }
        
        return nullptr;
    }

    std::shared_ptr<objects::Object> BuiltinFunc_Fibonacci([[maybe_unused]] std::vector<std::shared_ptr<objects::Object>>& args)
    {
        if(args.size() != 1)
        {
            return objects::newError("wrong number of arguments. got=" + std::to_string(args.size()) + ", want=1");
        }

        if(std::shared_ptr<objects::Integer> obj = std::dynamic_pointer_cast<objects::Integer>(args[0]); obj != nullptr)
        {
            if(obj->Value < 0)
            {
                return objects::newError("argument to `fibonacci` can not be negative, got " + std::to_string(obj->Value));
            }

            return std::make_shared<objects::Integer>(fibonacci(obj->Value));
        }
        else
        {
            return objects::newError("argument to `fibonacci` must be Integer, got " + args[0]->TypeStr());
        }
    }

    struct BuiltinWithName
    {
        std::string Name;
        std::shared_ptr<objects::Builtin> Builtin;

        BuiltinWithName(const std::string name, BuiltinFunction fn)
            : Name(name)
        {
            Builtin = std::make_shared<objects::Builtin>(fn);
        }
    };

    std::vector<std::shared_ptr<objects::BuiltinWithName>> Builtins{
        std::make_shared<objects::BuiltinWithName>("len", &BuiltinFunc_Len),
        std::make_shared<objects::BuiltinWithName>("puts", &BuiltinFunc_Puts),
        std::make_shared<objects::BuiltinWithName>("first", &BuiltinFunc_First),
        std::make_shared<objects::BuiltinWithName>("last", &BuiltinFunc_Last),
        std::make_shared<objects::BuiltinWithName>("rest", &BuiltinFunc_Rest),
        std::make_shared<objects::BuiltinWithName>("push", &BuiltinFunc_Push),
        std::make_shared<objects::BuiltinWithName>("fibonacci", &BuiltinFunc_Fibonacci),
    };

    std::shared_ptr<objects::Builtin> GetBuiltinByName(const std::string& name)
    {
        for(auto &def: Builtins)
        {
            if(def->Name == name)
            {
                return def->Builtin;
            }
        }

        return nullptr;
    }
}

#endif // H_OBJECTS_BUILTINS_H
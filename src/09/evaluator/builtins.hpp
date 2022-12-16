#ifndef H_BUILTINS_H
#define H_BUILTINS_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "objects/objects.hpp"

namespace evaluator
{
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
            return objects::newError("argument to 'len' not supported, got " + args[0]->TypeStr());
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
                return objects::NULL_OBJ;
            }
        }
        else
        {
            return objects::newError("argument to 'first' must be ARRAY, got " + args[0]->TypeStr());
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
                return objects::NULL_OBJ;
            }
        }
        else
        {
            return objects::newError("argument to 'last' must be ARRAY, got " + args[0]->TypeStr());
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
                return objects::NULL_OBJ;
            }
        }
        else
        {
            return objects::newError("argument to 'rest' must be ARRAY, got " + args[0]->TypeStr());
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
            return objects::newError("argument to 'push' must be ARRAY, got " + args[0]->TypeStr());
        }
    }

    std::shared_ptr<objects::Object> BuiltinFunc_Puts([[maybe_unused]] std::vector<std::shared_ptr<objects::Object>>& args)
    {
        for(const auto& obj: args)
        {
            std::cout << obj->Inspect() << std::endl;
        }
        
        return objects::NULL_OBJ;
    }

    std::map<std::string, std::shared_ptr<objects::Builtin>> builtins
    {
        {
            "len", std::make_shared<objects::Builtin>(&BuiltinFunc_Len)
        },
        {
            "first", std::make_shared<objects::Builtin>(&BuiltinFunc_First)
        },
        {
            "last", std::make_shared<objects::Builtin>(&BuiltinFunc_Last)
        },
        {
            "rest", std::make_shared<objects::Builtin>(&BuiltinFunc_Rest)
        },
        {
            "push", std::make_shared<objects::Builtin>(&BuiltinFunc_Push)
        },
        {
            "puts", std::make_shared<objects::Builtin>(&BuiltinFunc_Puts)
        }
    };

}

#endif // H_BUILTINS_H
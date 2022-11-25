#ifndef H_BUILTINS_H
#define H_BUILTINS_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "objects/objects.hpp"

namespace evaluator
{
    std::shared_ptr<objects::Error> newError(std::string msg);

    std::shared_ptr<objects::Object> len_func([[maybe_unused]] std::vector<std::shared_ptr<objects::Object>>& args)
    {
        if(args.size() != 1)
        {
            return newError("wrong number of arguments. got=" + std::to_string(args.size()) + ", want=1");
        }

        if(std::shared_ptr<objects::String> obj = std::dynamic_pointer_cast<objects::String>(args[0]); obj != nullptr)
        {
            return std::make_shared<objects::Integer>(obj->Value.size());
        }
        else
        {
            return newError("argument to 'len' not supported, got " + args[0]->TypeStr());
        }
    }

    std::map<std::string, std::shared_ptr<objects::Builtin>> builtins
    {
        {
            "len", std::make_shared<objects::Builtin>(&len_func)
        }
    };

}

#endif // H_BUILTINS_H
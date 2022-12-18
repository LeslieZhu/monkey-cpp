#ifndef H_BUILTINS_H
#define H_BUILTINS_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "objects/objects.hpp"
#include "objects/builtins.hpp"

namespace evaluator
{
    std::map<std::string, std::shared_ptr<objects::Builtin>> builtins{
        {"len", objects::GetBuiltinByName("len")},
        {"puts", objects::GetBuiltinByName("puts")},
        {"first", objects::GetBuiltinByName("first")},
        {"last", objects::GetBuiltinByName("last")},
        {"rest", objects::GetBuiltinByName("rest")},
        {"push", objects::GetBuiltinByName("push")},
        {"fibonacci", objects::GetBuiltinByName("fibonacci")}
    };
}

#endif // H_BUILTINS_H
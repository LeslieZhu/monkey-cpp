#ifndef H_FRAME_H
#define H_FRAME_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>

#include "code/code.hpp"
#include "objects/environment.hpp"

namespace vm
{
    struct Frame{
        std::shared_ptr<objects::CompiledFunction> fn;
        int ip;

        Frame(std::shared_ptr<objects::CompiledFunction> f, const int i): fn(f), ip(i){}

        bytecode::Instructions Instruction()
        {
            return fn->Instructions;
        }
    };

    std::shared_ptr<Frame> NewFrame(std::shared_ptr<objects::CompiledFunction> f)
    {
        return std::make_shared<Frame>(f, -1);
    }
}

#endif // H_FRAME_H
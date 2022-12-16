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
        int basePointer;

        Frame(std::shared_ptr<objects::CompiledFunction> f, const int i, const int bp): fn(f), ip(i), basePointer(bp){}

        bytecode::Instructions Instruction()
        {
            return fn->Instructions;
        }
    };

    std::shared_ptr<Frame> NewFrame(std::shared_ptr<objects::CompiledFunction> f, int basePointer)
    {
        return std::make_shared<Frame>(f, -1, basePointer);
    }
}

#endif // H_FRAME_H
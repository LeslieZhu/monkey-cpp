#ifndef H_FRAME_H
#define H_FRAME_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>

#include "code/code.hpp"
#include "objects/objects.hpp"

namespace vm
{
    struct Frame{
        std::shared_ptr<objects::Closure> cl;
        int ip;
        int basePointer;

        Frame(std::shared_ptr<objects::Closure> cl, const int i, const int bp): cl(cl), ip(i), basePointer(bp){}

        bytecode::Instructions Instruction()
        {
            return cl->Fn->Instructions;
        }
    };

    std::shared_ptr<Frame> NewFrame(std::shared_ptr<objects::Closure> cl, int basePointer)
    {
        return std::make_shared<Frame>(cl, -1, basePointer);
    }
}

#endif // H_FRAME_H
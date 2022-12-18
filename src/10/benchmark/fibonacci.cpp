
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <chrono>

#define STRIP_FLAG_HELP 1
#include <gflags/gflags.h>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "objects/objects.hpp"
#include "objects/environment.hpp"
#include "evaluator/evaluator.hpp"
#include "compiler/compiler.hpp"
#include "vm/vm.hpp"

std::string input = R""(
let fibonacci = fn(x){
    if(x == 0){
        return 0;
    } else {
        if(x == 1){
            return 1;
        } else {
            return fibonacci(x - 1) + fibonacci(x - 2);
        }
    }
};

fibonacci(35);
)"";

std::string input2 = "fibonacci(35);";

DEFINE_string(engine, ":)", "use 'vm' or 'eval'");
DEFINE_bool(builtin, false, "use builtin fibonacci function");

int main(int argc, char **argv)
{
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    auto result = std::make_shared<objects::Object>();

    auto start = std::chrono::system_clock::now();
    auto end = start;

    auto pLexer = lexer::New(input);

    if(FLAGS_builtin){
        pLexer = lexer::New(input2);
    }

    auto pParser = parser::New(std::move(pLexer));
    auto pProgram = pParser->ParseProgram();

    std::shared_ptr<ast::Node> astNode(reinterpret_cast<ast::Node *>(pProgram.release()));

    if(FLAGS_engine == "vm")
    {
        auto comp = compiler::New();
        auto error = comp->Compile(astNode);
        if(objects::isError(error))
        {
            std::cout << "compiler error: " << error->Inspect() << std::endl;
            return -1;
        }

        auto machine = vm::New(comp->Bytecode());

        start = std::chrono::system_clock::now();

        result = machine->Run();
        if(objects::isError(result))
        {
            std::cout << "vm error: " << result->Inspect() << std::endl;
            return -1;
        }

        end = std::chrono::system_clock::now();

        result = machine->LastPoppedStackElem();
    } else if(FLAGS_engine == "eval") {
        auto env = objects::NewEnvironment();

        start = std::chrono::system_clock::now();

        result = evaluator::Eval(astNode, env);

        end = std::chrono::system_clock::now();
    } else {
        std::cout << "usage: fibonacci -engine vm|eval [-builtin]" << std::endl;
        return -1;
    }

    auto diff = std::chrono::duration_cast<std::chrono::seconds>(end - start);

    std::cout << "engine=" << FLAGS_engine << ", fibonacci(35)=" << result->Inspect() << ", duration=" << diff.count() << "s" << std::endl;

    return 0;
}
#ifndef H_REPL_H
#define H_REPL_H

#include <iostream>
#include <string>
#include <memory>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
//#include "objects/environment.hpp"
//#include "evaluator/evaluator.hpp"
#include "compiler/compiler.hpp"
#include "vm/vm.hpp"
#include "objects/builtins.hpp"

namespace repl
{

    const static std::string PROMPT = ">> ";

    const static std::string MONKEY_FACE = R""(
   .--.  .-"     "-.  .--.
  / .. \/  .-. .-.  \/ .. \
 | |  '|  /   Y   \  |'  | |
 | \   \  \ 0 | 0 /  /   / |
  \ '- ,\.-"""""""-./, -' /
   ''-' /_   ^ ^   _\ '-''
       |  \._   _./  |
       \   \ '~' /   /
        '._ '-=-' _.'
           '-----'
                              )"";

    void printParserErrors(std::vector<std::string> errors)
    {
        std::cout << MONKEY_FACE << std::endl;
        std::cout << "Woops! We ran into some monkey business here!" << std::endl;
        std::cout << " parser errors:" << std::endl;
        for (auto &e : errors)
        {
            std::cout << "\t" << e << std::endl;
        }
    }

    void Start()
    {
        //auto env = objects::NewEnvironment();

        std::vector<std::shared_ptr<objects::Object>> constants{};
        std::vector<std::shared_ptr<objects::Object>> globals(vm::GlobalsSize);
        auto symbolTable = compiler::NewSymbolTable();

        int i = -1;
        for(auto &fn: objects::Builtins)
        {
            i += 1;
            symbolTable->DefineBuiltin(i, fn->Name);
        }

        std::string line;
        while (true)
        {
            std::cout << PROMPT;

            getline(std::cin, line);

            if (line.size() == 0)
            {
                continue;
            }

            std::unique_ptr<lexer::Lexer> pLexer = lexer::New(line);
            std::unique_ptr<parser::Parser> pParser = parser::New(std::move(pLexer));
            std::unique_ptr<ast::Program> pProgram{pParser->ParseProgram()};

            std::vector<std::string> errors = pParser->Errors();
            if (errors.size() > 0)
            {
                printParserErrors(errors);
                continue;
            }

            std::unique_ptr<ast::Node> astNode(reinterpret_cast<ast::Node *>(pProgram.release()));

            /* auto evaluated = evaluator::Eval(std::move(astNode), env);

            if (evaluated != nullptr)
            {
                std::cout << evaluated->Inspect() << std::endl;
            } */

            //auto comp = compiler::New();
            auto comp = compiler::NewWithState(symbolTable, constants);
            auto result = comp->Compile(std::move(astNode));
            if(objects::isError(result))
            {
                std::cout << "Woops! Compilation failed: \n" + result->Inspect() << std::endl;
                continue;
            }

            //auto machine = vm::New(comp->Bytecode());
            auto code = comp->Bytecode();
            auto machine = vm::NewWithGlobalsStore(code, globals);

            auto runResult = machine->Run();
            if(objects::isError(runResult))
            {
                std::cout << "Woops! Executing bytecode failed: \n" + runResult->Inspect() << std::endl;
                continue;
            }

            // auto stackTop = machine->StackTop();
            auto stackTop = machine->LastPoppedStackElem();
            std::cout << stackTop->Inspect() << std::endl;

            constants = code->Constants;
            globals = machine->globals;
        }
    }
}
#endif // H_REPL_H

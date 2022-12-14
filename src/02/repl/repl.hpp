#ifndef H_REPL_H
#define H_REPL_H

#include <iostream>
#include <string>
#include <memory>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

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
        std::string line;
        while (true)
        {
            std::cout << PROMPT;

            getline(std::cin, line);

            if (line.size() == 0)
            {
                break;
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

            std::cout << pProgram->String() << std::endl;
        }
    }
}
#endif // H_REPL_H

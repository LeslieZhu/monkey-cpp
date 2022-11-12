#ifndef H_REPL_H
#define H_REPL_H

#include <iostream>
#include <string>
#include <memory>

#include "token/token.hpp"
#include "lexer/lexer.hpp"

namespace repl
{

    const static std::string PROMPT = ">> ";

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

            std::unique_ptr<lexer::Lexer> l = lexer::New(line);

            for (token::Token tok = l->NextToken(); tok.Type != token::types::EndOF; tok = l->NextToken())
            {
                std::cout << tok << std::endl;
            }
        }
    }
}
#endif // H_REPL_H

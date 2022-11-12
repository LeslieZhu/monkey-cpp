#include <iostream>
#include <unistd.h>

#include "repl/repl.hpp"

int main()
{
    char *user = getlogin();

    std::cout << "Hello " << user << "! This is the Monkey-CPP programming language!" << std::endl;
    std::cout << "Feel free to type in commands" << std::endl;

    repl::Start();

    return 0;
}

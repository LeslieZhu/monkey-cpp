#include <gtest/gtest.h>

#include "test/lexer_test.hpp"
#include "test/ast_test.hpp"
#include "test/parser_test.hpp"
#include "test/evaluator_test.hpp"

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    // Runs all tests using Google Test.
    return RUN_ALL_TESTS();
}
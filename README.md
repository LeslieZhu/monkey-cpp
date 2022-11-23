# Writing An Interpreter In C++

Reference the book [Writing An Interpreter In Go](https://interpreterbook.com/).

But re-write the interpreter with C++.

- src/01: token and lexer
- src/02: parser
- src/03: evaluation

# Run

For example:

```
$ cd src/02/
$ mkdir build/
$ cd build/
$ cmake ..
$ make
$ ./test_monkey
$ ./monkey
```

# Requires

- C++17
- [GoogleTest(Version 1.10.0)](https://github.com/google/googletest)

# Writing An Interpreter In C++: Monkey-CPP

Reference the book [Writing An Interpreter In Go](https://interpreterbook.com/).

But re-write the interpreter with C++.

- src/01: token and lexer
- src/02: parser
- src/03: evaluation

# Run

For example:

```
$ cd src/03/
$ mkdir build/
$ cd build/
$ cmake ..
$ make

$ ./test_monkey

$ ./monkey

Hello lesliezhu! This is the Monkey-CPP programming language!
Feel free to type in commands
>> let add = fn(x,y){ return x + y; }
>> add(3,4)
7
>>
```

# Requires

- C++17
- [GoogleTest(Version 1.10.0)](https://github.com/google/googletest)

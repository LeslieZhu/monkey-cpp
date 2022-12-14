# Monkey: Writing An Interpreter/Compiler In C++

Reference the book [Writing An Interpreter In Go](https://interpreterbook.com/) and [Writing A Compiler In Go](https://compilerbook.com/).

But re-write the Monkey interpreter/compiler with C++.

- src/01: token and lexer
- src/02: parser
- src/03: evaluation
- src/04: builtin funcs, string, array, hashmap
- src/05: bytecode and virutal machine, OpConstant and OpAdd only
- src/06: expression and condition statement
- src/07: symbol table, string, array, hashmap and index expression
- src/08: function

Also see the [MinYiLife Blogs](http://lesliezhu.com/tags/%E8%A7%A3%E9%87%8A%E5%99%A8%E4%B8%8E%E7%BC%96%E8%AF%91%E5%99%A8.html)

# Run

For example:

```
$ cd src/08/
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

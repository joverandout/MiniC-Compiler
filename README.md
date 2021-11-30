# MiniC-Compiler
A compiler for the Mini C language written in C++

The grammar for Mini C can be found in the file `grammar.txt`.

Load `Clang` modules and `LLVM `with the following commands
```
module load gcc9
export PATH=/modules/cs325/llvm-12.0.1/bin:$PATH
export LD_LIBRARY_PATH=/modules/cs325/llvm-12.0.1/lib:$LD_LIBRARY_PATH
```
If that throws an error please follow the instructions to install `LLVM` and `Clang` here: https://llvm.org/docs/GettingStarted.html#getting-started-quickly-a-summary

Compile and build the lexer with 
```
make
```

From there you can lex some MiniC code with the following command
```
./mccomp user_code.c
```

This will generate and AST tree which is also outputted for ease of reading. as seen here

![alt text](https://media.discordapp.net/attachments/192724811594596352/915193021748879380/unknown.png?width=602&height=678)

To run the code and evaluate it after parsing first create an output:

```
mccomp addition.c
```

Then use `output.ll` to build and execute alongside `Clang`

```
clang++ driver.cpp output.ll -o add
./add
```

This should output a result or throw an error.

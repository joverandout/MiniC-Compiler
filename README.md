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


For this example we will be using the MiniC code `addition.c`

```
// MiniC program to test addition
extern int print_int(int X);

int addition(int n, int m){
  int result;
  result = n + m;
  

  if(n == 4) {
    print_int(n+m);
  }
  else {
    print_int(n*m);
  }

  return result;
}
```


From there you can lex some MiniC code with the following command
```
./mccomp addition.c
```

This will generate and AST tree which is also outputted for ease of reading. as seen here

```
--------------AST-------------
Externs:
    ├──function: print_int
    |  type: 'INT'
    |  parameters: 
    |      ├──Variable: X
    |      |  type: 'INT'
Declarations:
    ├──function: addition
    |  body: 
    |      ├──Declarations of local variables: 
    |      |      ├──Variable declared:result
    |      |      |  Type: int
    |      ├──Statements: 
    |      |      ├──Assignment: 
    |      |      |      Name :result
    |      |      |      Value: Expression:
    |      |      |      |      ├──Left hand side: n
    |      |      |      |      ├──Operator: +
    |      |      |      |      ├──Right hand side: m
    |      |      ├──If statement:
    |      |      |      ├──Condition: Expression:
    |      |      |      |      ├──Left hand side: n
    |      |      |      |      ├──Operator: ==
    |      |      |      |      ├──Right hand side: 4
    |      |      |      ├──Block: 
    |      |      |      |      ├──Statements: 
    |      |      |      |      |      ├──call to: print_int
    |      |      |      |      |      |      ├──Argument: Expression:
    |      |      |      |      |      |      |      ├──Left hand side: n
    |      |      |      |      |      |      |      ├──Operator: +
    |      |      |      |      |      |      |      ├──Right hand side: m
    |      |      |      ├──Else block: 
    |      |      |      |      ├──Statements: 
    |      |      |      |      |      ├──call to: print_int
    |      |      |      |      |      |      ├──Argument: Expression:
    |      |      |      |      |      |      |      ├──Left hand side: n
    |      |      |      |      |      |      |      ├──Operator: *
    |      |      |      |      |      |      |      ├──Right hand side: m
    |      |      ├──Return Statement:
    |      |      |      ├──Expression: result
```

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

```
[u1917702@login-2 addition] $ ./add
18
18
PASSED Result:9
```

# Disclosure
The code in this git repository is the copyright of Joe Moore and distribution or use is not allowed without explicit permission and without giving full credit

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <string.h>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace llvm;
using namespace llvm::sys;

bool lineneeded = true;

int errorCount = 0;
int assign = 0;
int indentation = 0;
std::string indent = "    ";
bool isrhsorlhs = false;

std::string getIndent(){
  std::string id = "";
  for (size_t i = 0; i < indentation; i++)
  {
    id = id + indent;
  }
  return id;
}

FILE *pFile;

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

// The lexer returns one of these for known things.
enum TOKEN_TYPE {

  IDENT = -1,        // [a-zA-Z_][a-zA-Z_0-9]*
  ASSIGN = int('='), // '='

  // delimiters
  LBRA = int('{'),  // left brace
  RBRA = int('}'),  // right brace
  LPAR = int('('),  // left parenthesis
  RPAR = int(')'),  // right parenthesis
  SC = int(';'),    // semicolon
  COMMA = int(','), // comma

  // types
  INT_TOK = -2,   // "int"
  VOID_TOK = -3,  // "void"
  FLOAT_TOK = -4, // "float"
  BOOL_TOK = -5,  // "bool"

  // keywords
  EXTERN = -6,  // "extern"
  IF = -7,      // "if"
  ELSE = -8,    // "else"
  WHILE = -9,   // "while"
  RETURN = -10, // "return"
  // TRUE   = -12,     // "true"
  // FALSE   = -13,     // "false"

  // literals
  INT_LIT = -14,   // [0-9]+
  FLOAT_LIT = -15, // [0-9]+.[0-9]+
  BOOL_LIT = -16,  // "true" or "false" key words

  // logical operators
  AND = -17, // "&&"
  OR = -18,  // "||"

  // operators
  PLUS = int('+'),    // addition or unary plus
  MINUS = int('-'),   // substraction or unary negative
  ASTERIX = int('*'), // multiplication
  DIV = int('/'),     // division
  MOD = int('%'),     // modular
  NOT = int('!'),     // unary negation

  // comparison operators
  EQ = -19,      // equal
  NE = -20,      // not equal
  LE = -21,      // less than or equal to
  LT = int('<'), // less than
  GE = -23,      // greater than or equal to
  GT = int('>'), // greater than

  // special tokens
  EOF_TOK = 0, // signal end of file

  // invalid
  INVALID = -100 // signal invalid token
};


// TOKEN struct is used to keep track of information about a token
struct TOKEN {
  int type = -100;
  std::string lexeme;
  int lineNo;
  int columnNo;
};

static std::string IdentifierStr; // Filled in if IDENT
static int IntVal;                // Filled in if INT_LIT
static bool BoolVal;              // Filled in if BOOL_LIT
static float FloatVal;            // Filled in if FLOAT_LIT
static std::string StringVal;     // Filled in if String Literal
static int lineNo, columnNo;

static TOKEN returnTok(std::string lexVal, int tok_type) {
  TOKEN return_tok;
  return_tok.lexeme = lexVal;
  return_tok.type = tok_type;
  return_tok.lineNo = lineNo;
  return_tok.columnNo = columnNo - lexVal.length() - 1;
  return return_tok;
}

// Read file line by line -- or look for \n and if found add 1 to line number
// and reset column number to 0
/// gettok - Return the next token from standard input.
static TOKEN gettok() {

  static int LastChar = ' ';
  static int NextChar = ' ';

  // Skip any whitespace.
  while (isspace(LastChar)) {
    if (LastChar == '\n' || LastChar == '\r') {
      lineNo++;
      columnNo = 1;
    }
    LastChar = getc(pFile);
    columnNo++;
  }

  if (isalpha(LastChar) ||
      (LastChar == '_')) { // identifier: [a-zA-Z_][a-zA-Z_0-9]*
    IdentifierStr = LastChar;
    columnNo++;

    while (isalnum((LastChar = getc(pFile))) || (LastChar == '_')) {
      IdentifierStr += LastChar;
      columnNo++;
    }

    if (IdentifierStr == "int")
      return returnTok("int", INT_TOK);
    if (IdentifierStr == "bool")
      return returnTok("bool", BOOL_TOK);
    if (IdentifierStr == "float")
      return returnTok("float", FLOAT_TOK);
    if (IdentifierStr == "void")
      return returnTok("void", VOID_TOK);
    if (IdentifierStr == "bool")
      return returnTok("bool", BOOL_TOK);
    if (IdentifierStr == "extern")
      return returnTok("extern", EXTERN);
    if (IdentifierStr == "if")
      return returnTok("if", IF);
    if (IdentifierStr == "else")
      return returnTok("else", ELSE);
    if (IdentifierStr == "while")
      return returnTok("while", WHILE);
    if (IdentifierStr == "return")
      return returnTok("return", RETURN);
    if (IdentifierStr == "true") {
      BoolVal = true;
      return returnTok("true", BOOL_LIT);
    }
    if (IdentifierStr == "false") {
      BoolVal = false;
      return returnTok("false", BOOL_LIT);
    }

    return returnTok(IdentifierStr.c_str(), IDENT);
  }

  if (LastChar == '=') {
    NextChar = getc(pFile);
    if (NextChar == '=') { // EQ: ==
      LastChar = getc(pFile);
      columnNo += 2;
      return returnTok("==", EQ);
    } else {
      LastChar = NextChar;
      columnNo++;
      return returnTok("=", ASSIGN);
    }
  }

  if (LastChar == '{') {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok("{", LBRA);
  }
  if (LastChar == '}') {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok("}", RBRA);
  }
  if (LastChar == '(') {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok("(", LPAR);
  }
  if (LastChar == ')') {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok(")", RPAR);
  }
  if (LastChar == ';') {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok(";", SC);
  }
  if (LastChar == ',') {
    LastChar = getc(pFile);
    columnNo++;
    return returnTok(",", COMMA);
  }

  if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9]+.
    std::string NumStr;

    if (LastChar == '.') { // Floatingpoint Number: .[0-9]+
      do {
        NumStr += LastChar;
        LastChar = getc(pFile);
        columnNo++;
      } while (isdigit(LastChar));

      FloatVal = strtof(NumStr.c_str(), nullptr);
      return returnTok(NumStr, FLOAT_LIT);
    } else {
      do { // Start of Number: [0-9]+
        NumStr += LastChar;
        LastChar = getc(pFile);
        columnNo++;
      } while (isdigit(LastChar));

      if (LastChar == '.') { // Floatingpoint Number: [0-9]+.[0-9]+)
        do {
          NumStr += LastChar;
          LastChar = getc(pFile);
          columnNo++;
        } while (isdigit(LastChar));

        FloatVal = strtof(NumStr.c_str(), nullptr);
        return returnTok(NumStr, FLOAT_LIT);
      } else { // Integer : [0-9]+
        IntVal = strtod(NumStr.c_str(), nullptr);
        return returnTok(NumStr, INT_LIT);
      }
    }
  }

  if (LastChar == '&') {
    NextChar = getc(pFile);
    if (NextChar == '&') { // AND: &&
      LastChar = getc(pFile);
      columnNo += 2;
      return returnTok("&&", AND);
    } else {
      LastChar = NextChar;
      columnNo++;
      return returnTok("&", int('&'));
    }
  }

  if (LastChar == '|') {
    NextChar = getc(pFile);
    if (NextChar == '|') { // OR: ||
      LastChar = getc(pFile);
      columnNo += 2;
      return returnTok("||", OR);
    } else {
      LastChar = NextChar;
      columnNo++;
      return returnTok("|", int('|'));
    }
  }

  if (LastChar == '!') {
    NextChar = getc(pFile);
    if (NextChar == '=') { // NE: !=
      LastChar = getc(pFile);
      columnNo += 2;
      return returnTok("!=", NE);
    } else {
      LastChar = NextChar;
      columnNo++;
      return returnTok("!", NOT);
      ;
    }
  }

  if (LastChar == '<') {
    NextChar = getc(pFile);
    if (NextChar == '=') { // LE: <=
      LastChar = getc(pFile);
      columnNo += 2;
      return returnTok("<=", LE);
    } else {
      LastChar = NextChar;
      columnNo++;
      return returnTok("<", LT);
    }
  }

  if (LastChar == '>') {
    NextChar = getc(pFile);
    if (NextChar == '=') { // GE: >=
      LastChar = getc(pFile);
      columnNo += 2;
      return returnTok(">=", GE);
    } else {
      LastChar = NextChar;
      columnNo++;
      return returnTok(">", GT);
    }
  }

  if (LastChar == '/') { // could be division or could be the start of a comment
    LastChar = getc(pFile);
    columnNo++;
    if (LastChar == '/') { // definitely a comment
      do {
        LastChar = getc(pFile);
        columnNo++;
      } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

      if (LastChar != EOF)
        return gettok();
    } else
      return returnTok("/", DIV);
  }

  // Check for end of file.  Don't eat the EOF.
  if (LastChar == EOF) {
    columnNo++;
    return returnTok("0", EOF_TOK);
  }

  // Otherwise, just return the character as its ascii value.
  int ThisChar = LastChar;
  std::string s(1, ThisChar);
  LastChar = getc(pFile);
  columnNo++;
  return returnTok(s, int(ThisChar));
}

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
static TOKEN CurTok;
static std::deque<TOKEN> tok_buffer;

static TOKEN getNextToken() {

  if (tok_buffer.size() == 0)
    tok_buffer.push_back(gettok());

  TOKEN temp = tok_buffer.front();
  tok_buffer.pop_front();

  return CurTok = temp;
}



static void putBackToken(TOKEN tok) { tok_buffer.push_front(tok); }

//===----------------------------------------------------------------------===//
// AST nodes
//===----------------------------------------------------------------------===//

/// ASTnode - Base class for all AST nodes.
class ASTnode {
public:
  virtual ~ASTnode() {}
  virtual Value *codegen() = 0;
  virtual std::string to_string() const {
    std::cout << "AST NODE";
    return "";
  };
};

/// IntASTnode - Class for integer literals like 1, 2, 10,
class IntASTnode : public ASTnode {
  int Val;
  TOKEN Tok;
  std::string Name;

public:
  IntASTnode(TOKEN tok, int val) : Val(val), Tok(tok) {}
  virtual Value *codegen() override {};
  virtual std::string to_string() const override {
    return std::to_string(Val);
  }
};

class floatASTnode : public ASTnode {
  float Val;
  TOKEN Tok;
  std::string Name;

public:
  floatASTnode(TOKEN tok, float val) : Val(val), Tok(tok) {}
  virtual Value *codegen() override {};
  virtual std::string to_string() const override {
    return std::to_string(Val);
  }
};


class boolASTnode : public ASTnode {
  bool Val;
  TOKEN Tok;
  std::string Name;

public:
  boolASTnode(TOKEN tok, bool val) : Val(val), Tok(tok) {}
  virtual Value *codegen() override {};
  virtual std::string to_string() const override {
    return std::to_string(Val);
  }
};

class notAndNegativeASTnode : public ASTnode {
  char prefix;
  TOKEN token;
  std::string name;
  std::unique_ptr<ASTnode> expression;
public:
  notAndNegativeASTnode(char Prefix, TOKEN Token, std::unique_ptr<ASTnode> Expression) : prefix(Prefix), token(Token), expression(std::move(Expression)) {}
  virtual Value *codegen() override {};
  virtual std::string to_string() const override {
    return "\nPREFIX: " + std::string(1, prefix) + "\nNAME: " + name;
  }
};

class typeASTnode : public ASTnode{
  TOKEN token;

public:
  typeASTnode(TOKEN t) : token(t){}
  virtual Value *codegen() override {};
  virtual std::string to_string() const override{
    std::string returner = "";
    switch (token.type)
    {
    case INT_TOK:
      returner = returner + "'INT'";
      break;
    case BOOL_TOK:
      returner = returner + "'BOOL'";
      break;
    case FLOAT_TOK:
      returner = returner + "'FLOAT'";
      break;
    case VOID_TOK:
      returner = returner + "'VOID'";
      break;
    default:
      break;
    }
    returner = returner;
    return returner;
  };

  std::string typereturn(){
    return token.lexeme.c_str();
  }

  TOKEN getToken(){
    return token;
  }

  int getType(){
    return token.type;
  }

};

class returnASTnode : public ASTnode {
  std::unique_ptr<ASTnode> expression;
  std::unique_ptr<ASTnode> semiColon;
public:
  returnASTnode(std::unique_ptr<ASTnode> Expression) : expression(std::move(Expression)){}
  returnASTnode(){}

  virtual Value *codegen() override {};
  virtual std::string to_string() const override {
    std::string stringy = "";
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent;
      else stringy = stringy + "|      ";
    }
    stringy += "├──Return Statement:\n";
    if(expression){
      indentation++;
      for (size_t i = 0; i < indentation; i++)
      {
        if(i == 0) stringy += indent;
        else stringy = stringy + "|      ";
      }
      stringy = stringy + "├──Expression: " + expression->to_string().c_str() + "\n";
      indentation--;
    }
    return stringy;
  }
};


class identASTnode : public ASTnode {
  TOKEN token;
  std::string value;

public:
  identASTnode(TOKEN Token, std::string Value) : token(Token), value(Value) {}
  virtual Value *codegen() override {};
  virtual std::string to_string() const override{
    return value;
  }
  TOKEN getToken(){
    return token;
  }
};


class globalASTnode : public ASTnode {
  std::unique_ptr<typeASTnode> type;
  std::unique_ptr<identASTnode> ident;
public:
  globalASTnode(std::unique_ptr<typeASTnode> Type, std::unique_ptr<identASTnode> Ident)
  : type(std::move(Type)), ident(std::move(Ident)) {}
  virtual Value *codegen() override {};
  int getType() const {
    return type->getType();
  }
  std::string get_name(){
    return ident->to_string();
  }
  virtual std::string to_string() const override{
    std::string stringy = "";
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent;
      else stringy = stringy + "|      ";
    }
    stringy += "├──Variable declared:" +  ident->to_string() + "\n";;
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent;
      else stringy = stringy + "|      ";
    }
    stringy += "|  Type: " + type->typereturn();
    return stringy;
  }
};


class BlockASTnode : public ASTnode {
  std::vector<std::unique_ptr<globalASTnode>> declarations;
  std::vector<std::unique_ptr<ASTnode>> statements;

public:
  BlockASTnode(std::vector<std::unique_ptr<globalASTnode>> newDeclarations, std::vector<std::unique_ptr<ASTnode>> newStatements) :
  declarations(std::move(newDeclarations)), statements(std::move(newStatements)){}
  virtual Value *codegen() override {};
  virtual std::string to_string() const override {    
    std::string tostring = "";
    if(declarations.size() >= 1){
      tostring += "\n";
      for (size_t i = 0; i < indentation; i++)
      {
        if(i == 0) tostring += indent;
        else tostring = tostring + "|      ";
      }
      tostring += "├──";
      tostring = tostring + "Declarations of local variables: \n";
      indentation++;
      for (size_t i = 0; i < declarations.size(); i++)
      {
        tostring = tostring + std::string(declarations.at(i)->to_string().c_str()) + "\n";
      }
      indentation--;
    }
    if(statements.size() >= 1){
      for (size_t i = 0; i < indentation; i++)
      {
        if(i == 0) tostring += indent;
        else tostring = tostring + "|      ";
      }
      tostring += "├──";
      tostring = tostring + "Statements: \n";
      indentation++;
      for (size_t i = 0; i < statements.size(); i++)
      {
        tostring = tostring + "" + std::string(statements.at(i)->to_string().c_str());
      }
      indentation--;
    }
    return tostring;
  }
};

class ifASTnode : public ASTnode{
  std::unique_ptr<ASTnode> expr;
  std::unique_ptr<BlockASTnode> block;
  std::unique_ptr<BlockASTnode> elseBlock;

public:
  ifASTnode(std::unique_ptr<ASTnode> Expr, std::unique_ptr<BlockASTnode> Block, std::unique_ptr<BlockASTnode> ElseBlock) : expr(std::move(Expr)), block(std::move(Block)), elseBlock(std::move(ElseBlock)) {}
  virtual Value *codegen() override {};
  virtual std::string to_string() const override {
    std::string stringy ="";
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent ;
      else stringy = stringy + "|      ";
    }
    stringy += "├──If statement:\n";
    indentation++;
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent ;
      else stringy = stringy + "|      ";
    }
    stringy = stringy +   "├──Condition: " + expr->to_string();
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent ;
      else stringy = stringy + "|      ";
    }
    stringy = stringy +   "├──Block: \n";
    indentation++;
    stringy += block->to_string();
    indentation--;
    if(lineneeded) stringy += "\n";
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent ;
      else stringy = stringy + "|      ";
    }
    if(elseBlock){
      stringy = stringy + "├──Else block: \n";
      indentation++;
      stringy += elseBlock->to_string();
      indentation--;
    }
    indentation--;
    return stringy;
  }
};



class assignmentASTnode : public ASTnode {
    std::unique_ptr<identASTnode> ident;
    std::unique_ptr<ASTnode> expr;

public:
  assignmentASTnode(std::unique_ptr<identASTnode> Ident, std::unique_ptr<ASTnode> Expr) : ident(std::move(Ident)), expr(std::move(Expr)) {}
  virtual Value *codegen() override {};

  virtual std::string to_string() const override {
    std::string stringy = "";
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent;
      else stringy = stringy + "|      ";
    }
    stringy += "├──Assignment: \n";
    assign++;
    indentation++;
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent;
      else stringy = stringy + "|      ";
    }
    stringy = stringy + "Name :" + ident->to_string() +"\n";
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent;
      else stringy = stringy + "|      ";
    }
    indentation++;
    stringy = stringy + "Value: " + expr->to_string();
    if(lineneeded) stringy += "\n";
    lineneeded = true;
    indentation--;
    indentation--;
    return  stringy;
  }
};

class parameterASTnode : public ASTnode {
  std::unique_ptr<typeASTnode> type;
  std::unique_ptr<identASTnode> identifier;
public:
  parameterASTnode(std::unique_ptr<typeASTnode> Type, std::unique_ptr<identASTnode> Identifier) : type(std::move(Type)), identifier(std::move(Identifier)) {}
  virtual Value *codegen() override {};
  virtual std::string to_string() const override {
    std::string stringy = "";
    stringy = stringy + "Variable: ";
    if(identifier) stringy += identifier->to_string() +"\n";
    else stringy += "void";
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent;
      else stringy = stringy + "|      ";
    }
    
    stringy = stringy + "|  type: "  + type->to_string();
    return stringy;
  }
  int getType(){
    return type->getType();
  }
  TOKEN getTokenOfType(){
    return type->getToken();
  }
  TOKEN getTokenOfIdent(){
    return identifier->getToken();
  }
  std::string getName(){
    return identifier->to_string();
  }
};

class expressionASTnode : public ASTnode {
  std::unique_ptr<ASTnode> left;
  std::string operation;
  std::unique_ptr<ASTnode> right;
public:
  expressionASTnode(std::unique_ptr<ASTnode> LEFT, TOKEN Operation, std::unique_ptr<ASTnode> RIGHT) 
  : left(std::move(LEFT)), operation(Operation.lexeme), right(std::move(RIGHT)) {}
  virtual Value *codegen() override {};
  virtual std::string to_string() const override {
    bool indentb = false;
    if(isrhsorlhs){
      indentation++;
      indentb = true;
    }
    std::string stringy = "\n";
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent;
      else stringy = stringy + "|      ";
    }
    stringy += "├──Expression:\n";
    indentation++;
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent;
      else stringy = stringy + "|      ";
    }
    isrhsorlhs = true;
    stringy += "├──Left hand side: " + left->to_string() +"\n";
    isrhsorlhs = false;
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent;
      else stringy = stringy + "|      ";
    }
    stringy += "├──Operator: " + operation +"\n";
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent;
      else stringy = stringy + "|      ";
    }
    isrhsorlhs = true;
    stringy += "├──Right hand side: " + right->to_string() + "\n";
    lineneeded = false;
    isrhsorlhs = false;
    indentation--;
    if(indentb == true){
      indentation--;
    }
    return stringy;
  }
};

class functionCall : public ASTnode {
  std::unique_ptr<ASTnode> name;
  std::vector<std::unique_ptr<ASTnode>> arguments;
  std::string caller;
public:
  functionCall(std::unique_ptr<ASTnode> Name, std::vector<std::unique_ptr<ASTnode>> Arguments, TOKEN token) : name(std::move(Name)), arguments(std::move(Arguments)), caller(token.lexeme.c_str()){}
  virtual Value *codegen() override {};
  virtual std::string to_string() const override{
    std::string stringy = "";
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent;
      else stringy = stringy + "|      ";
    }
    stringy += "├──call to: " + name->to_string() +"\n";
    indentation++;
    if(arguments.size() > 0){
      for (size_t i = 0; i < indentation; i++)
      {
        if(i == 0) stringy += indent;
        else stringy = stringy + "|      ";
      }
      stringy = stringy +"├──Argument: " + arguments[0]->to_string();
      for (size_t i = 1; i < arguments.size(); i++)
      {
        stringy += "\n";
        for (size_t i = 0; i < indentation; i++)
        {
          if(i == 0) stringy += indent;
          else stringy = stringy + "|      ";
        }
        stringy = stringy +"├──Argument: " + arguments[i]->to_string();
      }      
    }
    stringy+="\n";
    indentation--;
    return stringy;
  }
};

class externASTnode : public ASTnode {
  std::unique_ptr<typeASTnode> type;
  std::unique_ptr<identASTnode> identifer;
  std::vector<std::unique_ptr<parameterASTnode>> parameters;
public:
  externASTnode(std::unique_ptr<typeASTnode> Type, std::unique_ptr<identASTnode> Identifier, std::vector<std::unique_ptr<parameterASTnode>> Parameters)
  : type(std::move(Type)), identifer(std::move(Identifier)), parameters(std::move(Parameters)) {}
  virtual Value *codegen() override {};

  int getType() {
    return type->getType();
  }
  std::string getName(){
    return identifer->to_string();
  }

  std::string to_string() const override {
    std::string stringy = "function: " + identifer->to_string() + "\n";
    stringy = stringy + getIndent() + "|  " + "type: " + type->to_string() + "\n";
    stringy = stringy + getIndent() + "|  " + "parameters: " + "\n";
    indentation++;
    for (size_t i = 0; i < parameters.size(); i++)
    {
      for (size_t j = 0; j < indentation -1; j++)
      {
        stringy = stringy + indent + "|";
      }
      stringy += indent + "  ├──";
      stringy = stringy + parameters.at(i)->to_string().c_str() + "\n";
    }
    indentation--;
    return stringy;
  }
};


class functionASTnode : public ASTnode{
  std::unique_ptr<externASTnode> function;
  std::unique_ptr<BlockASTnode> funcBody;
public:
  functionASTnode(std::unique_ptr<externASTnode> Function, std::unique_ptr<BlockASTnode> FuncBody) : function(std::move(Function)), funcBody(std::move(FuncBody)) {}
  virtual Function *codegen() override {};

  virtual std::string to_string() const override{
    std::string stringy = "function: ";
    stringy += function->getName() +"\n";
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent ;
      else stringy = stringy + "|      ";
    }
    stringy += "|  body: ";
    if(funcBody) {
      indentation++;
      stringy = stringy + funcBody->to_string().c_str();
    }
    else stringy += "[empty]";
    return stringy;
  }
};

class whileASTnode : public ASTnode{
  std::unique_ptr<ASTnode> expr;
  std::unique_ptr<ASTnode> stmt;

public:
  whileASTnode(std::unique_ptr<ASTnode> expression, std::unique_ptr<ASTnode> statement) : expr(std::move(expression)), stmt(std::move(statement)){}
  virtual Value *codegen() override {};

  virtual std::string to_string() const override{
    std::string stringy = "";
    for (size_t i = 0; i < indentation; i++)
    {
      if(i == 0) stringy += indent ;
      else stringy = stringy + "|      ";
    }
    stringy = stringy + "├──While statement: ";
    stringy = stringy  + expr->to_string().c_str();
    indentation++;
    stringy = stringy + stmt->to_string().c_str();
    indentation--;
    return stringy;
  }
};


class programASTnode : public ASTnode{
  std::vector<std::unique_ptr<externASTnode>> externList;
  std::vector<std::unique_ptr<ASTnode>> declList;
public:
  virtual ~programASTnode(){};
  programASTnode(std::vector<std::unique_ptr<externASTnode>> Externs, std::vector<std::unique_ptr<ASTnode>> Decls) : externList(std::move(Externs)), declList(std::move(Decls)) {}
  programASTnode(std::vector<std::unique_ptr<ASTnode>> Decls) : declList(std::move(Decls)) {}
  virtual Value *codegen() override {};

  virtual std::string to_string() const override{
    std::string stringy = "--------------AST-------------\n";
    int size1 = externList.size();
    int size2 = declList.size();
    if(size1>0) indentation++;
    stringy = stringy + "Externs:\n";
    for (size_t i = 0; i < size1; i++)
    {
      stringy = stringy + getIndent() + "├──"+ externList.at(i)->to_string().c_str();
    }
    if(size1>0) indentation--;
    if(size2>0) indentation++;
    stringy = stringy + "Declarations:\n";
    for (size_t i = 0; i < size2; i++)
    { 
      stringy = stringy + getIndent() + "├──"+ declList.at(i)->to_string().c_str();
    }
    if(size2>0) indentation--;
    return stringy;
  }
};


/* add other AST nodes as nessasary */

//===----------------------------------------------------------------------===//
// Recursive Descent Parser - Function call for each production
//===----------------------------------------------------------------------===//

/* Add function calls for each production */

//static std::unique_ptr<ASTnode> ElementParser(){

static void line(){
  errorCount++;
  printf("============================\n~ERROR %d~\n", errorCount);
}

static void errorMessage(){
  printf("TOKEN: Unexpected Token, %s, encountered.\nLOCATION: '%s' was found at Row,Column [%d,%d] \n", CurTok.lexeme.c_str(), CurTok.lexeme.c_str(), CurTok.lineNo, CurTok.columnNo);
}

static std::unique_ptr<ASTnode> expressionParser();
static std::unique_ptr<ASTnode> subExprParser();

static std::unique_ptr<ifASTnode> ifParser();
static std::unique_ptr<whileASTnode> whileParser();
static std::unique_ptr<BlockASTnode> blockParser();
static std::unique_ptr<parameterASTnode> paramParser();



static bool argListChecker(){
  if(CurTok.type == COMMA ) return false;
  if(CurTok.type == RPAR) return false;
  return true;
}

static bool curTokType(TOKEN Current){
  if (Current.type == INT_LIT || Current.type == FLOAT_LIT || Current.type == BOOL_LIT || Current.type == MINUS || Current.type == NOT || Current.type == IDENT || Current.type == LPAR){
    return true;
  }
  return false;
}

static bool exprstmt(){
  if(CurTok.type==INT_LIT || CurTok.type==BOOL_LIT || CurTok.type==FLOAT_LIT || CurTok.type==IDENT || CurTok.type==SC || CurTok.type==LBRA || CurTok.type==MINUS || CurTok.type==NOT || CurTok.type==LPAR) return false;
  return true;
}

static bool AndTerm(){
  return(CurTok.type==AND || (CurTok.type==IDENT || CurTok.type==SC || CurTok.type==COMMA || CurTok.type==RPAR || CurTok.type==MINUS || CurTok.type==NOT || CurTok.type==LPAR || CurTok.type==INT_LIT || CurTok.type==BOOL_LIT || CurTok.type==FLOAT_LIT || CurTok.type==OR));
}

static bool OrTerm(){
  return(CurTok.type == OR || CurTok.type==AND || (CurTok.type==IDENT || CurTok.type==SC || CurTok.type==COMMA || CurTok.type==RPAR || CurTok.type==MINUS || CurTok.type==NOT || CurTok.type==LPAR || CurTok.type==INT_LIT || CurTok.type==BOOL_LIT || CurTok.type==FLOAT_LIT || CurTok.type==OR));
}

static bool checkTerm(int size, int tokens[13]){
  for (size_t i = 0; i < size; i++)
  {
    if(CurTok.type == tokens[i]) return false;
  }
  return true;
}


static std::vector<std::unique_ptr<ASTnode>> ArgsListPrimeParser(){
  std::vector<std::unique_ptr<ASTnode>> stdList;
  std::vector<std::unique_ptr<ASTnode>> vector;

  if(argListChecker() == true){
    line();printf("ERROR: Missing ',' or ')'\n");
    errorMessage();
    return vector;
  }
  if(CurTok.type == COMMA){
    switch (CurTok.type)
    {
      case COMMA:
        getNextToken();
      default:
        std::string printable = "============================\nERROR: Token " + CurTok.lexeme + " is not ',' (COMMA) as expected\n";
        errorMessage();     
        break;
    }
    auto expr = expressionParser();
    auto args = ArgsListPrimeParser();
    if(expr) {stdList.push_back(std::move(expr));}
    int size = (int) args.size();
    for (size_t i = 0; i < size; i++)
    {
      stdList.push_back(std::move(args[i]));
    }
  }
  else{
    if(CurTok.type != RPAR){
      line();printf("ERROR: Expected token RPAR ')'\n");
      errorMessage();   
      return vector;
    }
  }
  
  return stdList;
}

static std::vector<std::unique_ptr<ASTnode>> ArgsListParser(){
  std::vector<std::unique_ptr<ASTnode>> stdList;
  std::vector<std::unique_ptr<ASTnode>> vector;

  if(curTokType(CurTok) == false && CurTok.type != RPAR){
    line();printf("ERROR: Expected an identifier, literal or one of [MINUS '-', NOT '!', LPAR '(']");
    errorMessage();   
    return vector;
  }

  auto expr = expressionParser();
  auto args = ArgsListPrimeParser();

  if(CurTok.type != RPAR){
    line();printf("ERROR: Expected token RPAR ')'");
    errorMessage();   
    return vector;
  }
  if(expr) {stdList.push_back(std::move(expr));}
  int size = (int) args.size();
  for (size_t i = 0; i < size; i++)
  {
    stdList.push_back(std::move(args[i]));
  }
  
  return stdList;
}

static std::unique_ptr<functionCall> leftParanthesis(TOKEN identifier){
  auto ident = std::make_unique<identASTnode>(identifier, identifier.lexeme);
  getNextToken();
  auto temp = ArgsListParser();
  getNextToken();
  return std::make_unique<functionCall>(std::move(ident), std::move(temp), identifier);
}


static std::unique_ptr<ASTnode> ElementParser(){
  if(CurTok.type == INT_LIT){
    auto returner = std::make_unique<IntASTnode>(CurTok, IntVal);
    auto inty = std::move(returner);
    getNextToken();
    if(inty) return inty;
  }
  else if(CurTok.type == FLOAT_LIT){
    auto returner = std::make_unique<floatASTnode>(CurTok, FloatVal);
    auto floaty = std::move(returner);
    getNextToken();
    if(floaty) return floaty;
  }
  else if(CurTok.type == BOOL_LIT){
    auto returner = std::make_unique<boolASTnode>(CurTok, BoolVal);
    auto booly = std::move(returner);
    getNextToken();
    if(booly) return booly;
  }
  else if(CurTok.type == MINUS){
    char oper = '-';
    TOKEN negativeToken = CurTok;
    getNextToken();
    auto element = ElementParser();
    auto newResult = nullptr;
    if(element){
      auto neg = std::make_unique<notAndNegativeASTnode>(oper, negativeToken, std::move(element));
      return std::move(neg);
    }
    if(newResult) return newResult;
  }
  else if(CurTok.type == NOT){
    char oper = '!';
    TOKEN notToken = CurTok;
    getNextToken();
    auto element = ElementParser();
    auto newResult = nullptr;
    if(element){
      auto neg = std::make_unique<notAndNegativeASTnode>(oper, notToken, std::move(element));
      return std::move(neg);
    }
    if(newResult) return newResult;
  }
  else if(CurTok.type == IDENT){
    TOKEN identifier = CurTok;
    getNextToken();
    if(CurTok.type == LPAR) {
      auto lpar = leftParanthesis(identifier);
      return std::move(lpar);
    }
    else{
      putBackToken(CurTok);
      putBackToken(identifier);
      getNextToken();
      auto ident = std::make_unique<identASTnode>(CurTok, CurTok.lexeme);
      getNextToken();
      if(std::move(ident)) return std::move(ident);
    }
  }
  else if(CurTok.type == LPAR){
    getNextToken();
    auto expression = expressionParser(); 
    getNextToken();
    if(expression != nullptr){
      getNextToken();
      return std::move(expression);
    }
  }
  else{
    line();
    printf("ERROR. Missing element -> Expected a literal, variable, identity, '(', '!', or '-'\n");
    errorMessage(); 
  }
  return nullptr;
}


static std::unique_ptr<ASTnode> factorParser(){
  if(curTokType(CurTok)){
    auto element = ElementParser();
    int t = CurTok.type;
    TOKEN ooperator = CurTok;
    if((t==ASTERIX) || (t==DIV) || (t==MOD)) {
      getNextToken();
      auto factor = factorParser();
      if(factor != nullptr && element != nullptr){
        return std::move(std::make_unique<expressionASTnode>(std::move(element), ooperator, std::move(factor)));
      }
    }
    else{
      if(element) return element;
    }
  }
  else{
    line();
    printf("ERROR. Missing element -> Expected a literal, variable, identity, '(', '!', or '-'\n");
    errorMessage(); 
  }
  return nullptr;
  
}

static std::unique_ptr<ASTnode> plusOrMinus(){
  getNextToken();
  return subExprParser();
}

static std::unique_ptr<ASTnode> subExprParser(){
  if(curTokType(CurTok)){
    auto facotr = factorParser();
    TOKEN operatfor = CurTok;
    if(CurTok.type == PLUS){
      auto operand = plusOrMinus();
      if(facotr != nullptr){
        if(operand != nullptr){
          return std::move(std::make_unique<expressionASTnode>(std::move(facotr), operatfor, std::move(operand)));
        }
      }
    }
    if(CurTok.type ==  MINUS){
      auto operand = plusOrMinus();
      if(facotr != nullptr){
        if(operand != nullptr){
          return std::move(std::make_unique<expressionASTnode>(std::move(facotr), operatfor, std::move(operand)));
        }
      }
    }
    if (facotr) return facotr;
  }
  else{
    line();printf("ERROR: Missing or invalid AND, OR, RPAR, an identifier, SC, COMMA, RPAR, MINUS, NOT, LPAR or a literal.\n");
    errorMessage();
    getNextToken();
  }
  return nullptr;
}

static std::unique_ptr<ASTnode> relationalParser(){
  if(curTokType(CurTok)){
    auto sub = subExprParser();
    TOKEN comp = CurTok;
    if(CurTok.type == LE){
      getNextToken();
      auto rel = relationalParser();
      if(sub != nullptr){
        if(rel != nullptr){
          return std::move(std::make_unique<expressionASTnode>(std::move(sub), comp, std::move(rel)));
        }
      }
    }
    else if(CurTok.type == LT){
      getNextToken();
      auto rel = relationalParser();
      if(sub != nullptr){
        if(rel != nullptr){
          return std::move(std::make_unique<expressionASTnode>(std::move(sub), comp, std::move(rel)));
        }
      }
    }
    else if(CurTok.type == GT){
      getNextToken();
      auto rel = relationalParser();
      if(sub != nullptr){
        if(rel != nullptr){
          return std::move(std::make_unique<expressionASTnode>(std::move(sub), comp, std::move(rel)));
        }
      }
    }
    else if(CurTok.type == GE){
      getNextToken();
      auto rel = relationalParser();
      if(sub != nullptr){
        if(rel != nullptr){
          return std::move(std::make_unique<expressionASTnode>(std::move(sub), comp, std::move(rel)));
        }
      }
    }
   else{
     if(sub != nullptr) return sub;
   }
  }
  else{
    line();printf("ERROR: Missing or invalid AND, OR, RPAR, an identifier, SC, COMMA, RPAR, MINUS, NOT, LPAR or a literal.\n");
    errorMessage();
    getNextToken();
  }
  return nullptr;
}

static std::unique_ptr<ASTnode> equivalenceParser(){
  if(curTokType(CurTok)){
    auto rel = relationalParser(); 
    //printf("\n%s\n", rel->to_string().c_str());  
    TOKEN eqne = CurTok;
    if(CurTok.type == EQ || CurTok.type == NE){
      getNextToken();
      auto eq = equivalenceParser();
      if(eq != nullptr && rel != nullptr){
          return std::move(std::make_unique<expressionASTnode>(std::move(rel), eqne, std::move(eq)));
      }
    }
    else{
      if(rel != nullptr){return rel;}
    }
  }
  else{
    line();printf("ERROR: Missing or invalid AND, OR, RPAR, an identifier, SC, COMMA, RPAR, MINUS, NOT, LPAR or a literal.");
    errorMessage();   
    getNextToken();
  }
  return nullptr;
}

static std::unique_ptr<ASTnode> termParser(){
  if(curTokType(CurTok)){
    auto eq = equivalenceParser();
    TOKEN storeCurrent =  CurTok;
    if (CurTok.type == AND){
      getNextToken();
      auto term = termParser();
      if(term != nullptr){
        if(eq != nullptr){
          return std::move(std::make_unique<expressionASTnode>(std::move(eq), storeCurrent, std::move(term)));
        }
      }
    }
    else{
      if(eq != nullptr) return eq;
    }
  }
  else if(!AndTerm()){
    line();printf("ERROR: Missing or invalid AND, OR, RPAR, an identifier, SC, COMMA, RPAR, MINUS, NOT, LPAR or a literal.\n");
    errorMessage();   
    getNextToken();
  }
  else {
    line();printf("ERROR: Missing term -> Expected a literal, variable, identity, '(', '!', or '-'\n");
    errorMessage();  
  }
  return nullptr;
}

static std::unique_ptr<ASTnode> rvalParser(){
  if(curTokType(CurTok)){
    auto term = termParser();
    TOKEN storeCurrent = CurTok;
    if(CurTok.type == OR){
      getNextToken();
      auto rval = rvalParser();
      if(term != nullptr){
        if(rval != nullptr){
          return std::move(std::make_unique<expressionASTnode>(std::move(term), storeCurrent, std::move(rval)));
        }
      }
    }
    else{
      if(term != nullptr) return term;
    }
  }
  else if(!OrTerm()){
    line();printf("ERROR: Missing or invalid AND, OR, RPAR, an identifier, SC, COMMA, RPAR, MINUS, NOT, LPAR or a literal.\n");
    errorMessage();  
    getNextToken(); 
    return nullptr;
  }
  else{
    line();printf("ERROR: Missing rval -> Expected a literal, variable, identity, '(', '!', or '-' or '||'\n"); 
    errorMessage();   
  }
  return nullptr;
}

static std::unique_ptr<ASTnode> expressionParser(){
  if (CurTok.type == IDENT){
    TOKEN temporaryIdentifierStorage = CurTok;
    getNextToken();
    if(CurTok.type == ASSIGN){
      auto identifier = std::make_unique<identASTnode>(temporaryIdentifierStorage, temporaryIdentifierStorage.lexeme);
      getNextToken();
      auto expr = expressionParser();
      if(expr){
        return std::move(std::make_unique<assignmentASTnode>(std::move(identifier), std::move(expr)));
      }
    }
    putBackToken(CurTok);
    putBackToken(temporaryIdentifierStorage);
    getNextToken();
  }
  if (curTokType(CurTok)){
    auto rval = rvalParser();
    if(rval != nullptr) return std::move(rval);
  }
  else{
    line();printf("ERROR: Missing assignment or expression \n");
    errorMessage();
  }
  return nullptr;
}

static std::unique_ptr<ASTnode> expressionStatementParser(){
  if(exprstmt() == true){
    line();printf("ERROR: Missing identifer, literal, or SC ';', NOT '!', LPAR '(', or a literal\n");
    errorMessage();  
    getNextToken(); 
    return nullptr;
  }
  if(CurTok.type == SC){
    getNextToken();
    int list[13] = {IDENT, SC, LBRA, WHILE, IF, RETURN, MINUS, NOT, LPAR, INT_LIT, BOOL_LIT, FLOAT_LIT, RBRA};
    if(checkTerm(13, list)){
      line();printf("ERROR: Missing identifier, or SC ';', LBRA '{', RBRA '{', WHILE, IF, MINUS '-', NOT '!', LPAR '(' RETURN, or a literal.\n");
      errorMessage();  
      getNextToken(); 
    }
  }
  else{
    auto ex = expressionParser();
    if(CurTok.type == SC){
      //printf("%s - %d\n", CurTok.lexeme.c_str(), CurTok.type);
      getNextToken();
      //printf("%s - %d\n", CurTok.lexeme.c_str(), CurTok.type);
    }
    else{
      line();printf("ERROR: No semi colon at line end, instead Token ");
      printf(" was encountered rather than ';' as expected.\n");
      errorMessage();  
      getNextToken(); 
    }
    if(!(CurTok.type == EOF_TOK || CurTok.type == EOF || CurTok.type==IDENT || CurTok.type==SC || CurTok.type==LBRA || CurTok.type==WHILE || CurTok.type==IF || CurTok.type==RETURN || CurTok.type==MINUS || CurTok.type==NOT || CurTok.type==LPAR || CurTok.type==INT_LIT || CurTok.type==BOOL_LIT || CurTok.type==FLOAT_LIT || CurTok.type==RBRA)){
      line();printf("ERROR: Missing identifier, or SC ';', LBRA '{', RBRA '{', WHILE, IF, MINUS '-', NOT '!', LPAR '(' RETURN, or a literal.\n");
      errorMessage();  
      getNextToken(); 
    }
    return ex;
  }
  return nullptr;
}

static std::unique_ptr<returnASTnode> returnStatementParser(){
  if(CurTok.type == RETURN){
      getNextToken();
      if(CurTok.type == SC){
        auto returner = std::make_unique<returnASTnode>();
        getNextToken();
        return returner;
      }
      else if(CurTok.type == INT_LIT || CurTok.type == FLOAT_LIT || CurTok.type == BOOL_LIT || CurTok.type == NOT || CurTok.type == LPAR || CurTok.type == IDENT || CurTok.type == MINUS){
        auto expression = expressionParser();
        if(CurTok.type == SC){
          auto returner = std::make_unique<returnASTnode>(std::move(expression));
          getNextToken();
          return returner;
        }
      }
      else{
        line();printf("ERROR: Missing semicolon ';' after expression in RETURN statement\n");
        errorMessage();
      }
  }
  else{
    line();printf("ERROR: Missing 'RETURN' before statement\n");
    errorMessage();
    return nullptr;
  }
  return nullptr;
}


static std::unique_ptr<ASTnode> statementParser(){
  if(CurTok.type == IF){ //call if;
    auto ifF = ifParser();
    if(ifF != nullptr) return std::move(ifF);
  }
  else if(CurTok.type == WHILE)//call while;
  {
    auto whileE = whileParser();
    if(whileE != nullptr) return std::move(whileE);
  }
  else if(CurTok.type == RETURN) //call block
  {
    auto returnN = returnStatementParser();
    if(returnN != nullptr) return std::move(returnN);
  }
  else if(CurTok.type == LBRA) //call block;
  {
    auto blockK = blockParser();
    return blockK;
  }
  else if(CurTok.type == INT_LIT || CurTok.type == BOOL_LIT || CurTok.type ==  FLOAT_LIT || CurTok.type == MINUS || CurTok.type == NOT ||CurTok.type == SC || CurTok.type == LPAR || CurTok.type == IDENT){
    auto expressionStatements = expressionStatementParser();
    if(expressionStatements) return std::move(expressionStatements);
    return nullptr;
  }
  else{
    line();printf("ERROR: No statement definition\n");
    errorMessage();
  }
  return nullptr;
}

static std::vector<std::unique_ptr<parameterASTnode>> parameterlistPrimeParser(){
  std::vector<std::unique_ptr<parameterASTnode>> parameters;
  std::vector<std::unique_ptr<parameterASTnode>> vector;
  if(CurTok.type != COMMA) {
    if(CurTok.type != RPAR){
      line();printf("ERROR: Missing COMMA ','\n");
      errorMessage();
      return vector;
    }
  }
  if(CurTok.type == COMMA){
    getNextToken();
    auto parameter = paramParser();
    auto paramPrimeList = parameterlistPrimeParser();

    if(parameter){
      parameters.push_back(std::move(parameter));
    }
    int size = (int)paramPrimeList.size();
    for (size_t i = 0; i < size; i++)
    {
      parameters.push_back(std::move(paramPrimeList[i]));
    }
    
    if(CurTok.type == RPAR){
      return parameters;
    }
    else{
      line();printf("ERROR: Missing RajghPAR ')'\n");
      errorMessage();
      return vector;
    }
    return parameters;
  }
  return vector;
}

static std::vector<std::unique_ptr<parameterASTnode>> parameterListParser(){
  std::vector<std::unique_ptr<parameterASTnode>> parameters;
  std::vector<std::unique_ptr<parameterASTnode>> vector;
  if(CurTok.type == EOF_TOK) return vector;
  if(CurTok.type != INT_TOK && CurTok.type != FLOAT_TOK && CurTok.type != BOOL_TOK){
    line();printf("ERROR: Variable has no type, expected type before variable declaration\n");
    errorMessage();
    return vector;
  }

  auto parameter = paramParser();
  auto parameterPrimes = parameterlistPrimeParser();

  if(parameter){
    parameters.push_back(std::move(parameter));
  }
  int size = (int) parameterPrimes.size();
  for (int i = 0; i < size; i++)
  {
    parameters.push_back(std::move(parameterPrimes[i]));
  }
  if(CurTok.type == RPAR){
      return parameters;
  }
  else{
      line();printf("ERROR: Missing RPAR ')'\n");
      errorMessage();
      return vector;
    }
  return parameters;
}


static std::unique_ptr<typeASTnode> vartypeParser(){
  if(CurTok.type == INT_TOK || CurTok.type == FLOAT_TOK || CurTok.type == BOOL_TOK){
    TOKEN storage = CurTok;
    getNextToken();
    return std::make_unique<typeASTnode>(storage);
  }
  else{
    line();printf("ERROR: invalid variable declaration. %s encountered when 'int' 'bool' or 'float' expected\n", CurTok.lexeme.c_str());
    errorMessage();
    getNextToken();
    return nullptr;
  }
}

static std::vector<std::unique_ptr<ASTnode>> statementListParser(){
  std::vector<std::unique_ptr<ASTnode>> statements;
  std::vector<std::unique_ptr<ASTnode>> listOfStatements;

  if(CurTok.type == RBRA){
    if(CurTok.type != RBRA){
      line();printf("ERROR: Missing RBRA '}', instead encountered %s\n", CurTok.lexeme.c_str());
    }
    return statements;
  }
  if(CurTok.type == INT_LIT || CurTok.type == BOOL_LIT || CurTok.type == FLOAT_LIT || CurTok.type == NOT || CurTok.type == MINUS || CurTok.type == SC || CurTok.type == LPAR || CurTok.type == IDENT || CurTok.type == IF || CurTok.type == RETURN || CurTok.type == WHILE || CurTok.type == LBRA){
    auto st = statementParser();
    if(st){
      statements.push_back(std::move(st));
    }
    if(CurTok.type == INT_LIT || CurTok.type == BOOL_LIT || CurTok.type == FLOAT_LIT || CurTok.type == NOT || CurTok.type == MINUS || CurTok.type == SC || CurTok.type == LPAR || CurTok.type == IDENT || CurTok.type == IF || CurTok.type == RETURN || CurTok.type == WHILE || CurTok.type == LBRA){
      auto stmts = statementListParser();
      int size = (int) stmts.size();
      for (size_t i = 0; i < size; i++)
      {
        statements.push_back(std::move(stmts[i]));
      }
      
    }
  }
  else{
    line();printf("ERROR: Statement defined incorrectly\n");
    errorMessage();
    getNextToken();
  }
  return statements;
}

static std::unique_ptr<globalASTnode> localDeclParser(){
  if(CurTok.type == RBRA) return nullptr;
  if(!(CurTok.type == INT_TOK || CurTok.type == BOOL_TOK || CurTok.type == FLOAT_TOK)){
    line();printf("ERROR: Locally declared variable has no type\n");
    errorMessage();
  }
  else{
    auto variableType = vartypeParser();
    TOKEN store = CurTok;
    if(CurTok.type == IDENT){
      TOKEN store = CurTok;
      getNextToken();
    }
    auto identifier = std::make_unique<identASTnode>(store, store.lexeme);
    if(CurTok.type == SC){
      getNextToken();
    }
    else{
      line();printf("ERROR: Missing semi colon at end of declaration. Expected ';'\n");
      errorMessage();
      getNextToken();
    }
    if(CurTok.type==INT_TOK || CurTok.type == RBRA || CurTok.type==FLOAT_TOK || CurTok.type==BOOL_TOK || CurTok.type==IDENT || CurTok.type==SC || CurTok.type==LBRA ||CurTok.type==WHILE || CurTok.type==IF || CurTok.type==RETURN || CurTok.type==MINUS || CurTok.type==NOT || CurTok.type==LPAR || CurTok.type==INT_LIT || CurTok.type==BOOL_LIT || CurTok.type==FLOAT_LIT){
      return std::make_unique<globalASTnode>(std::move(variableType), std::move(identifier));
    }
    
  }
  line();printf("ERROR: Missing IDENT in declaration. Expected 'IDENT'\n");
  errorMessage();
  nullptr;
}

static std::vector<std::unique_ptr<parameterASTnode>> paramsParser(){
  std::vector<std::unique_ptr<parameterASTnode>> parameters;
  if(CurTok.type == INT_TOK || CurTok.type == BOOL_TOK || CurTok.type == FLOAT_TOK){
    return parameterListParser();
  }
  if(CurTok.type == VOID_TOK){
    auto voidD = std::make_unique<typeASTnode>(CurTok);
    auto parameter = std::make_unique<parameterASTnode>(std::move(voidD), nullptr);
    parameters.push_back(std::move(parameter));
    getNextToken();
    return parameters;
  }
  if(CurTok.type != RPAR){
    line();printf("ERROR: Parameter has no type, expected either 'INT', 'BOOL', 'FLOAT', or 'VOID'");
    errorMessage();
  }
  
  return parameters;
}

static std::vector<std::unique_ptr<globalASTnode>> localDeclsParser(){
  std::vector<std::unique_ptr<globalASTnode>> declarations;
  if(CurTok.type == RBRA) return declarations;
  if(CurTok.type == INT_TOK || CurTok.type == FLOAT_TOK || CurTok.type == BOOL_TOK){
    auto local = localDeclParser();

    auto decls = localDeclsParser();
    if(local != nullptr){
      declarations.push_back(std::move(local));
    }
    for (auto &&i : decls)
    {
      declarations.push_back(std::move(i));
    }  
    return declarations;
  }
  else{
    if (CurTok.type==INT_TOK || CurTok.type==FLOAT_TOK || CurTok.type==BOOL_TOK || CurTok.type==IDENT || CurTok.type==SC || CurTok.type==LBRA ||CurTok.type==WHILE || CurTok.type==IF || CurTok.type==RETURN || CurTok.type==MINUS || CurTok.type==NOT || CurTok.type==LPAR || CurTok.type==INT_LIT || CurTok.type==BOOL_LIT || CurTok.type==FLOAT_LIT){
      return declarations;
    }
    line();printf("ERROR: Incorrect definition of local declaration\n");
    errorMessage();
    getNextToken();
    return declarations;
  }
}

static std::unique_ptr<BlockASTnode> blockParser(){
  if(CurTok.type != LBRA){
    line();printf("ERROR: Missing LBRA at beginning of block, expected to find '{'\n");
    errorMessage();
    return nullptr;
  }
  else{
    getNextToken();
    auto declarations = localDeclsParser();
    auto statements = statementListParser();
    if(CurTok.type != RBRA){
      line();printf("ERROR: Missing RBRA at end of block, expected to find '}'\n");
      errorMessage();
      return nullptr;
    }
    else{
      getNextToken();
      return std::make_unique<BlockASTnode>(std::move(declarations), std::move(statements));
    }
  }
}

static std::unique_ptr<BlockASTnode> elseParser(){
  if(CurTok.type != ELSE && CurTok.type != IDENT && CurTok.type!=SC && CurTok.type!=LBRA && CurTok.type!=WHILE && CurTok.type!=IF && CurTok.type!=RETURN && CurTok.type!=MINUS && CurTok.type!=NOT && CurTok.type!=LPAR && CurTok.type!=INT_LIT && CurTok.type!=BOOL_LIT && CurTok.type!=FLOAT_LIT && CurTok.type!=RBRA && CurTok.type != EOF_TOK){
    line();printf("ERROR: missing 'ELSE' declaration at the beginning of else block\n");
    errorMessage();
  }
  if(CurTok.type == ELSE){
    getNextToken();
    if(CurTok.type != LBRA){
      line();printf("ERROR: missing LBRA '{' after 'ELSE'\n");
      errorMessage();
    }
    auto blockstatement = blockParser();
    if(CurTok.type==IDENT || CurTok.type==SC || CurTok.type==LBRA || CurTok.type==WHILE || CurTok.type==IF || CurTok.type==RETURN || CurTok.type==MINUS || CurTok.type == EOF_TOK || CurTok.type==NOT || CurTok.type==LPAR || CurTok.type==INT_LIT || CurTok.type==BOOL_LIT || CurTok.type==FLOAT_LIT || CurTok.type==RBRA ) {
      return blockstatement;    
    }
    else{
      line();printf("ERRORsf: Missing literal, identifier or SC, RBRA, WHILE, IF, RETURN, MINUS, NOT LPAR in ELSE block\n");
      errorMessage();
    }
  }
  else{
    if(CurTok.type != IDENT && CurTok.type!=SC && CurTok.type!=LBRA && CurTok.type!=WHILE && CurTok.type!=IF && CurTok.type!=RETURN && CurTok.type!=MINUS && CurTok.type!=NOT && CurTok.type!=LPAR && CurTok.type!=INT_LIT && CurTok.type!=BOOL_LIT && CurTok.type!=FLOAT_LIT && CurTok.type!=RBRA && CurTok.type != EOF_TOK){
      line();printf("ERROR: Missing literal, identifier or SC, RBRA, WHILE, IF, RETURN, MINUS, NOT LPAR in ELSE block\n");
      errorMessage();
    }
  }
  
  return nullptr;
}

static std::unique_ptr<ifASTnode> ifParser(){
  if(CurTok.type != IF){
    line();printf("ERROR: Expected 'IF'\n");
    errorMessage();
    return nullptr;
  }
  else{
    getNextToken();
    if(CurTok.type == LPAR){
      getNextToken();
    }
    else{
      line();printf("ERROR: Missing required LPAR '(' after IF declaration\n");
      errorMessage();
      getNextToken();
    }
    auto expression = expressionParser();
    
    if(CurTok.type == RPAR){
      getNextToken();
    }
    else{
      line();printf("ERROR: Missing required RPAR '(' after IF expression\n");
      errorMessage();
      getNextToken();
    }
    auto ifBlock = blockParser();
    auto elseStatement = elseParser();

    if(CurTok.type==IDENT || CurTok.type==SC || CurTok.type==LBRA || CurTok.type==WHILE || CurTok.type==IF || CurTok.type==RETURN || CurTok.type==MINUS || CurTok.type==NOT || CurTok.type==LPAR || CurTok.type==INT_LIT || CurTok.type==BOOL_LIT || CurTok.type==FLOAT_LIT || CurTok.type==RBRA || CurTok.type == EOF_TOK ) {
      auto returnBlock = std::make_unique<ifASTnode>(std::move(expression), std::move(ifBlock), std::move(elseStatement));
      return std::move(returnBlock);
    }
    else{
      line();printf("ERROR: Missing literal, identifier or SC, RBRA, WHILE, IF, RETURN, MINUS, NOT LPAR in ELSE block\n");
      errorMessage();
      return nullptr;
    }
  }
}


static std::unique_ptr<whileASTnode> whileParser(){
  if(CurTok.type == WHILE){
    getNextToken();
    if(CurTok.type == LPAR){
      getNextToken();
      auto expression = expressionParser();
      if(CurTok.type == RPAR){
        getNextToken();
        auto statement = statementParser();
        if(statement != nullptr){
          if(expression != nullptr){
            return std::move(std::make_unique<whileASTnode>(std::move(expression), std::move(statement)));
          }
        }
      }
      else{
        line();printf("ERROR: Missing RPAR ')' after 'WHILE' declaration\n");
        errorMessage();
      }
    }
    else{
      line();printf("ERROR: Missing LPAR '(' after 'WHILE' declaration\n");
      errorMessage();
    }
  }
  else{
    line();printf("ERROR: Missing 'WHILE' statement declaration\n");
    errorMessage();
    return nullptr;
  }
  return nullptr;
}



static std::unique_ptr<typeASTnode> typeSpecParser(){
  if(CurTok.type != VOID_TOK){
    if(CurTok.type == INT_TOK || CurTok.type == BOOL_TOK || CurTok.type == FLOAT_TOK){
      return vartypeParser();
    }
    line();printf("ERROR: Missing a declaration type\n");
    return nullptr;
  }
  else{
    if(CurTok.type == VOID_TOK){
      auto returnValue = std::make_unique<typeASTnode>(CurTok);
      getNextToken();
      return returnValue;
    }
    else{
      line();printf("ERROR: %s doesn't match expected type VOID_TOK\n", CurTok.lexeme.c_str());
      errorMessage();
    }
  }
  return nullptr;
}

static std::unique_ptr<parameterASTnode> variableDeclarationParser(){
  if(CurTok.type != INT_TOK && CurTok.type != FLOAT_TOK && CurTok.type != BOOL_TOK){
    line();printf("ERROR: No type in variable declartion, needed INT BOOL or FLOAT\n");
    errorMessage();
    return nullptr;
  }
  auto type = vartypeParser();
  auto ident = std::make_unique<identASTnode>(CurTok, CurTok.lexeme);
  if(CurTok.type == IDENT){
    getNextToken();
  }
  else{
    line();printf("ERROR: Missing Identifier\n");
    errorMessage();
  }
  if(CurTok.type == SC){
    getNextToken();
  }
  else{
    line();printf("ERROR: Missing SC ';' after identifier\n");
    errorMessage();
  }
  if(CurTok.type != INT_TOK && CurTok.type != FLOAT_TOK && CurTok.type != BOOL_TOK && CurTok.type != VOID_TOK && CurTok.type != EOF){
    line();printf("ERROR: Expected a new declaration (INT BOOL FLOAT OR VOID) or an EOF\n");
    errorMessage();
    return nullptr;
  }
  return std::make_unique<parameterASTnode>(std::move(type), std::move(ident));
}

static std::unique_ptr<functionASTnode> functionDeclarationParser(){
  auto typeSpec = typeSpecParser();
  getNextToken();
  if(CurTok.type != IDENT){
    line();printf("ERROR: Expected an identifier\n");
    errorMessage();
  }
  auto identifier = std::make_unique<identASTnode>(CurTok, CurTok.lexeme);
  if(CurTok.type == IDENT){
    getNextToken();
  }
  if(CurTok.type != LPAR){
    line();printf("ERROR: Missing LPAR '('\n");
    errorMessage();
  }
  else{
    getNextToken();
  }
  auto parameters = paramsParser();
  if(CurTok.type != RPAR){
    line();printf("ERROR: Missing or incorrect placement of RPAR ')'\n");
    errorMessage();
  }
  getNextToken();
  auto block = blockParser();
  auto function = std::make_unique<externASTnode>(std::move(typeSpec), std::move(identifier), std::move(parameters));
  return std::make_unique<functionASTnode>(std::move(function), std::move(block));
}


static std::unique_ptr<parameterASTnode> paramParser(){
  // if(CurTok.type != INT_TOK && CurTok.type != FLOAT_TOK && CurTok.type != BOOL_TOK){
    
  // }
  auto variableType = vartypeParser();
  if(CurTok.type == IDENT){
    auto identifier = std::make_unique<identASTnode>(CurTok, CurTok.lexeme);
    getNextToken();
    if(CurTok.type != RPAR && CurTok.type != COMMA && CurTok.type != SC){
      line();printf("ERROR: Expected COMMA ',' SC';' OR RPAR ')' instead encountered %s", CurTok.lexeme.c_str());
      errorMessage();
      return nullptr;
    }
    return std::make_unique<parameterASTnode>(std::move(variableType), std::move(identifier));
  }
  else{
    line();printf("ERROR: Missing IDENT, %s is not of type IDENT", CurTok.lexeme.c_str());
    errorMessage();
    auto identifier = std::make_unique<identASTnode>(CurTok, CurTok.lexeme);
    if(CurTok.type != RPAR && CurTok.type != COMMA && CurTok.type != SC){
      line();printf("ERROR: Expected COMMA ',' SC';' OR RPAR ')' instead encountered %s", CurTok.lexeme.c_str());
      errorMessage();
      return nullptr;
    }
    return std::make_unique<parameterASTnode>(std::move(variableType), std::move(identifier));
  }
}

static std::unique_ptr<ASTnode> declParser(){
  if(CurTok.type!= INT_TOK && CurTok.type != BOOL_TOK && CurTok.type != FLOAT_TOK && CurTok.type != VOID_TOK){
    line();printf("ERROR: Missing type in delcaration expected one of 'INT', 'BOOL', 'FLOAT' and 'VOID'");
    errorMessage();
  }
  if(CurTok.type == VOID_TOK){
    auto function = functionDeclarationParser(); 
    if(CurTok.type==VOID_TOK || CurTok.type==INT_TOK || CurTok.type==FLOAT_TOK || CurTok.type==BOOL_TOK || CurTok.type==EOF_TOK){
      return function;
    }
    line();printf("ERROR: Expected EOF or a declaration");
    errorMessage();
    return nullptr;
  }
  else{
    TOKEN one = CurTok;
    getNextToken();
    TOKEN two = CurTok;
    getNextToken();
    TOKEN sc = CurTok;
    putBackToken(sc);
    putBackToken(two);
    putBackToken(one);
    CurTok = one;
    if(sc.type == SC){
      auto variableDeclaration = variableDeclarationParser();
      if(CurTok.type==VOID_TOK || CurTok.type==INT_TOK || CurTok.type==FLOAT_TOK || CurTok.type==BOOL_TOK || CurTok.type==EOF_TOK){
      return variableDeclaration;
    }
    line();printf("ERROR: Expected EOF or a declaration");
    errorMessage();
    return nullptr;
    }
    else{
      auto functionDeclaration = functionDeclarationParser(); //TODO
      if(CurTok.type==VOID_TOK || CurTok.type==INT_TOK || CurTok.type==FLOAT_TOK || CurTok.type==BOOL_TOK || CurTok.type==EOF_TOK){
        return functionDeclaration;
      }
      line();printf("ERROR: Expected EOF or a declaration");
      errorMessage();
      return nullptr;
    }
  }
  return nullptr;
}


static std::vector<std::unique_ptr<ASTnode>> EOFparser(){
  if(CurTok.type != EOF_TOK){
    line();printf("ERROR: expected end of file after the declarations\n");
    errorMessage();
    std::vector<std::unique_ptr<ASTnode>> nullReturner;
    return nullReturner;
  }
}

static std::vector<std::unique_ptr<ASTnode>> declPrimeParser(){
  std::vector<std::unique_ptr<ASTnode>> declarations;
  if(CurTok.type == EOF){
    std::vector<std::unique_ptr<ASTnode>> nullReturner;
    return nullReturner;
  }
  else if(CurTok.type == INT_TOK || CurTok.type == BOOL_TOK || CurTok.type == FLOAT_TOK){
    auto declaration = declParser();
    auto declarationPrime = declPrimeParser();

    if(declaration){
      declarations.push_back(std::move(declaration));
    }
    int size = (int) declarationPrime.size();
    for (size_t i = 0; i < size; i++)
    {
      declarations.push_back(std::move(declarationPrime[i]));
    }
  }
  
  if(CurTok.type == EOF){
    return declarations;
  }
  return EOFparser();
}


static std::vector<std::unique_ptr<ASTnode>> declListParser(){
  std::vector<std::unique_ptr<ASTnode>> declarations;

  if(CurTok.type == EOF || CurTok.type == EOF_TOK) return declarations;

  if(CurTok.type!= INT_TOK && CurTok.type != BOOL_TOK && CurTok.type != FLOAT_TOK && CurTok.type != VOID_TOK){
    line();printf("ERROR: Missing type in delcaration expected one of 'INT', 'BOOL', 'FLOAT' and 'VOID'");
    errorMessage();
    std::vector<std::unique_ptr<ASTnode>> null;
    return null;
  }
  auto declaration = declParser();
  auto declarationPrime = declPrimeParser();


  if(declaration){
    declarations.push_back(std::move(declaration));
  }
  int size = (int) declarationPrime.size();
  for (size_t i = 0; i < size; i++)
  {
    declarations.push_back(std::move(declarationPrime[i]));
  }
  if(CurTok.type == EOF){
    return declarations;
  }
  return EOFparser();
}

static std::unique_ptr<externASTnode> externParser(){
  if(CurTok.type == EXTERN){
    getNextToken();
    if(CurTok.type == INT_TOK || CurTok.type == BOOL_TOK || CurTok.type == FLOAT_TOK || CurTok.type == VOID_TOK){
      auto varType = std::make_unique<typeASTnode>(CurTok);
      getNextToken();
      if(CurTok.type == IDENT){
        auto ident = std::make_unique<identASTnode>(CurTok, CurTok.lexeme);
        getNextToken();
        if(CurTok.type!= LPAR){
          line();printf("ERROR: Missing LPAR '(' for function\n");
          errorMessage();
        }
        getNextToken();
        auto parameters = paramsParser();
        if(CurTok.type!= RPAR){
          line();printf("ERROR: Missing RPAR ')' for function\n");
          errorMessage();
        }
        getNextToken();
        if(CurTok.type!= SC){
          line();printf("ERROR: Missing SC ';' for function\n");
          errorMessage();
        }
        auto returner = std::make_unique<externASTnode>(std::move(varType), std::move(ident), std::move(parameters));
        getNextToken();
        return std::move(returner);
      }
      else{
        line();printf("ERROR: Missing IDENT for function\n");
        errorMessage();
      }
    }
    else{
      line();printf("ERROR: Missing function type, expected either INT BOOL FLOAT or VOID ';' for function\n");
      errorMessage();
    }
  }
  else{
    line();printf("ERROR: Missing 'extern'\n");
    errorMessage();
    return nullptr;
  }
  return nullptr;
}

static std::vector<std::unique_ptr<externASTnode>> externListPrimeParser(){
  std::vector<std::unique_ptr<externASTnode>> externListPrime;
  std::vector<std::unique_ptr<externASTnode>> returner;

  if(CurTok.type != EXTERN && CurTok.type != VOID_TOK && CurTok.type != INT_TOK && CurTok.type != FLOAT_TOK && CurTok.type != BOOL_TOK)
  {
    line();printf("ERROR: Missing 'extern' or a type - INT FLOAT BOOL or VOID\n");
    errorMessage();
    return returner;
  }

  if(CurTok.type == EXTERN){
    auto externN = externParser();
    auto externPrimeE = externListPrimeParser();

    if(externN){
      externListPrime.push_back(std::move(externN));
    }
    int size = externListPrime.size();
    for (size_t i = 0; i < size; i++)
    {
      externListPrime.push_back(std::move(externPrimeE.at(i)));
    }
    if(CurTok.type != VOID_TOK && CurTok.type != INT_TOK && CurTok.type != FLOAT_TOK && CurTok.type != BOOL_TOK){
      line();printf("ERROR: Missing type - INT FLOAT BOOL or VOID\n");
      errorMessage();
      return returner;
    }
    return externListPrime;
  }

  return returner;
}


static std::vector<std::unique_ptr<externASTnode>> externListParser(){
  std::vector<std::unique_ptr<externASTnode>> externList;
  auto externType = externParser();
  if(externType){
    externList.push_back(std::move(externType));
    auto externListPrime = externListPrimeParser();
    int size = (int) externListPrime.size();
    for (size_t i = 0; i < size; i++)
    {
      externList.push_back(std::move(externListPrime.at(i)));
    }
    
  }
  return externList;
}

static std::vector<std::unique_ptr<ASTnode>> globalsListParser(){
  std::vector<std::unique_ptr<ASTnode>> globalList;
  if(CurTok.type == EOF || CurTok.type == EOF_TOK) return globalList;
  auto global = declParser();
  auto globalListT = globalsListParser();
  if(global){
    globalList.push_back(std::move(global));
  }
  int size = (int) globalListT.size();
  for (size_t i = 0; i < size; i++)
  {
    globalList.push_back(std::move(globalListT.at(i)));
  }
  return globalList;
}



// program ::= extern_list decl_list
static std::unique_ptr<ASTnode> parser() {
  bool externListBool = false;
  auto externlist = nullptr;
  bool declBool = false;
  if(CurTok.type == EXTERN){
    externListBool = true;
  }
  if(externListBool == false && CurTok.type != VOID_TOK && CurTok.type != INT_TOK && CurTok.type != BOOL_TOK && CurTok.type != FLOAT_TOK){
    return nullptr;
  }
  if(externListBool){
    auto externlist = externListParser();
    //printf("%s", externlist.at(0)->to_string().c_str());
    auto declList = globalsListParser();
    if(CurTok.type != EOF_TOK){
      line();printf("ERROR: EOF expected after decls\n");
      errorMessage();
    }
    return std::make_unique<programASTnode>(std::move(externlist), std::move(declList));
    // printf("%s", ex->to_string().c_str());
    // return ex;
  }
  auto declList = globalsListParser();
  if(CurTok.type != EOF_TOK){
    line();printf("ERROR: EOF expected after decls\n");
    errorMessage();
    return nullptr;
  }
  return std::make_unique<programASTnode>(std::move(declList));
  // printf("%s", ex->to_string().c_str());
  // return ex;
  
}

//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule;

//===----------------------------------------------------------------------===//
// AST Printer
//===----------------------------------------------------------------------===//

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const ASTnode &ast) {
  os << ast.to_string();
  return os;
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main(int argc, char **argv) {
  if (argc == 2) {
    pFile = fopen(argv[1], "r");
    if (pFile == NULL)
      perror("Error opening file");
  } else {
    std::cout << "Usage: ./code InputFile\n";
    return 1;
  }

  // initialize line number and column numbers to zero
  lineNo = 1;
  columnNo = 1;

  // get the first token
  // getNextToken();
  // while (CurTok.type != EOF_TOK) {
  //   fprintf(stderr, "Token: %s with type %d\n", CurTok.lexeme.c_str(),
  //           CurTok.type);
  //   getNextToken();
  // }
  getNextToken();
  static std::unique_ptr<ASTnode> graphic = parser();
  //
  if(errorCount > 0) printf("============================\n");
  printf("%d Errors found\n", errorCount);
  fprintf(stderr, "Lexer Finished\n");
  

  // Make the module, which holds all the code.
  TheModule = std::make_unique<Module>("mini-c", TheContext);


  // Run the parser now.
  parser();
  fprintf(stderr, "Parsing Finished\n");

  outs() << *graphic << '\n';

  //********************* Start printing final IR **************************
  // Print out all of the generated code into a file called output.ll
  auto Filename = "output.ll";
  std::error_code EC;
  raw_fd_ostream dest(Filename, EC, sys::fs::F_None);

  if (EC) {
    errs() << "Could not open file: " << EC.message();
    return 1;
  }
  // TheModule->print(errs(), nullptr); // print IR to terminal
  TheModule->print(dest, nullptr);
  //********************* End printing final IR ****************************

  fclose(pFile); // close the file that contains the code that was parsed
  return 0;
}

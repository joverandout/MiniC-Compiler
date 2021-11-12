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

int errorCount = 0;

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
  virtual std::string to_string() const {};
};

/// IntASTnode - Class for integer literals like 1, 2, 10,
class IntASTnode : public ASTnode {
  int Val;
  TOKEN Tok;
  std::string Name;

public:
  IntASTnode(TOKEN tok, int val) : Val(val), Tok(tok) {}
  virtual Value *codegen() override;
  virtual std::string to_string() const override {
    return "a sting representation of this AST node";
  }
};

class typeASTnode : public ASTnode{
  TOKEN token;

public:
  typeASTnode(TOKEN t) : token(t){}
  virtual Value *codegen() override {};
  virtual std::string to_string() const override{
    std::string returner = "Value or Function type: '";
    switch (token.type)
    {
    case INT_TOK:
      returner = returner + "INT'";
      break;
    case BOOL_TOK:
      returner = returner + "BOOL'";
      break;
    case FLOAT_TOK:
      returner = returner + "FLOAT'";
      break;
    case VOID_TOK:
      returner = returner + "VOID'";
      break;
    default:
      break;
    }
    return returner;
  };

  int getType(){
    return token.type;
  }

};


class BlockASTnode : public ASTnode {
  std::vector<std::unique_ptr<ASTnode>> declarations;
  std::vector<std::unique_ptr<ASTnode>> statements;

public:
  BlockASTnode(std::vector<std::unique_ptr<ASTnode>> newDeclarations, std::vector<std::unique_ptr<ASTnode>> newStatements) :
  declarations(std::move(newDeclarations)), statements(std::move(newStatements)){}
  virtual Value *codegen() override;
  virtual std::string to_string() const override {
    std::string tostring = "BLOCK: \n";
    if(declarations.size() >= 1){
      tostring = tostring + "   -> Declarations of local variables: \n";
      for (size_t i = 0; i < declarations.size(); i++)
      {
        tostring = tostring + "    - " + std::string(declarations.at(i)->to_string().c_str()) + "\n";
      }
      
    }
    if(statements.size() >= 1){
      tostring = tostring + "   -> Statements: \n";
      for (size_t i = 0; i < statements.size(); i++)
      {
        tostring = tostring + "    - " + std::string(statements.at(i)->to_string().c_str()) + "\n";
      }
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
  virtual Value *codegen() override;
  virtual std::string to_string() const override {
    std::string stringy = "\nIF STATEMENT:\n";
    stringy = stringy +   "\nCONDITION:    " + expr->to_string();
    stringy = stringy +   "\nBLOCK:        " + block->to_string();
    if(elseBlock){
      stringy = stringy + "\nELSE BLOCK:   " + elseBlock->to_string();
    }
    return stringy;
  };
};

class whileASTnode : public ASTnode{
  std::unique_ptr<ASTnode> expr;
  std::unique_ptr<ASTnode> stmt;

public:
  whileASTnode(std::unique_ptr<ASTnode> expression, std::unique_ptr<ASTnode> statment) : expr(std::move(expression)), stmt(std::move(statement)){}
  virtual Value *codegen() override;

  virtual std::string to_string() const override{
    std::string stringy = "";
    stringy = stringy + "\nWHILE STATEMENT:";
    stringy = stringy + "\nExpression:     " + expr->to_string().c_str();
    stringy = stringy + "\nStatements:     " + stmt->to_string().c_str();
    return stringy;
  };
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
  printf("TOKEN: Unexpected Token, %s, Encountered.\nLOCATION: '%s' was found at Row,Column [%d,%d] \n", CurTok.lexeme.c_str(), CurTok.lexeme.c_str(), CurTok.lineNo, CurTok.columnNo);
}

static std::unique_ptr<ASTnode> expressionParser();
static void subExprParser();

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
        printf("%s", printable.c_str());
        errorMessage();     
        break;
    }
    expressionParser();
    ArgsListPrimeParser();
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

  expressionParser();
  ArgsListPrimeParser();

  if(CurTok.type != RPAR){
    line();printf("ERROR: Expected token RPAR ')'");
    errorMessage();   
    return vector;
  }
  return stdList;
}

static void leftParanthesis(TOKEN identifier){
  //auto identifierAuto = std::make_unique<IdentASTnode>(identifier);
  getNextToken();
  /*auto argumentLIst = */auto temp = ArgsListParser();
  getNextToken();
  //return something here
}


static void ElementParser(){
  //printf("TOKEN = %s  &d\n", CurTok.lexeme.c_str(), CurTok.type);
  if(CurTok.type == INT_LIT){
    //auto Result = std::make_unique<IntASTnode>(CurTok, IntVal);
    getNextToken();
    //auto inty = std::move(Result);
    //if(inty) return inty;
  }
  else if(CurTok.type == FLOAT_LIT){
    //auto Result = std::make_unique<IntASTnode>(CurTok, FloatVal);
    getNextToken();
    //auto floaty = std::move(Result);
    //if(floaty) return floaty;
  }
  else if(CurTok.type == BOOL_LIT){
    //auto Result = std::make_unique<IntASTnode>(CurTok, BoolVal);
    getNextToken();
    //auto booly = std::move(Result);
    //if(booly) return booly;
  }
  else if(CurTok.type == MINUS){
    char oper = '-';
    TOKEN negativeToken = CurTok;
    getNextToken();
    /*auto expression =*/ ElementParser();
    auto newResult = nullptr;
    //if(expression){
      //auto Result = std::make_unique<NegativeASTnode>(negativeToken, oper, std::move(expression));
      //auto newResult = std::move(Result);
    //}
    //if(newResult) return newResult;
  }
  else if(CurTok.type == NOT){
    char oper = '!';
    TOKEN notToken = CurTok;
    getNextToken();
    /*auto expression =*/ ElementParser();
    auto newResult = nullptr;
    //if(expression){
      //auto Result = std::make_unique<NegativeASTnode>(notToken, oper, std::move(expression));
      //auto newResult = std::move(Result);
    //}
    //if(newResult) return newResult;
  }
  else if(CurTok.type == IDENT){
    TOKEN identifier = CurTok;
    getNextToken();
    if(CurTok.type == LPAR) {
      leftParanthesis(identifier);
    }
    else{
      putBackToken(CurTok);
      putBackToken(identifier);
      getNextToken();
      //auto Result = std::make_unique<IdentASTnode>(CurTok);
      getNextToken();
      //if(std::move(Result)) return std::move(Result)
    }
  }
  else if(CurTok.type == LPAR){
    getNextToken();
    expressionParser(); 
    getNextToken();
    //if(expression exists etc get next token)
  }
  else{
    line();
    printf("ERROR. Missing element -> Expected a literal, variable, identity, '(', '!', or '-'\n");
    errorMessage(); 
    getNextToken();  
  }
}


static void factorParser(){
  if(curTokType(CurTok)){
    ElementParser();
    int t = CurTok.type;
    TOKEN op = CurTok;
    if((t==ASTERIX) || (t==DIV) || (t==MOD)) { //FIRST(rval6')
        //rval6 = rval7 rval6'
      getNextToken();
      factorParser();
    }
  }
  else{
    line();
    printf("ERROR. Missing element -> Expected a literal, variable, identity, '(', '!', or '-'\n");
    errorMessage();
    getNextToken();  
  }
  
}

static void plusOrMinus(){
  getNextToken();
  subExprParser();
}

static void subExprParser(){
  if(curTokType(CurTok)){
    factorParser();
    switch (CurTok.type)
    {
    case PLUS:
      plusOrMinus();
      break;
    case MINUS:
      plusOrMinus();
      break;
    default:
      return;
      break;
    }
  }
  else{
    line();printf("ERROR: Missing or invalid AND, OR, RPAR, an identifier, SC, COMMA, RPAR, MINUS, NOT, LPAR or a literal.\n");
    errorMessage();
    getNextToken();
  }
}

static void relationalParser(){
  if(curTokType(CurTok)){
    subExprParser();
    switch (CurTok.type)
    {
    case LE:
      getNextToken();
      relationalParser();
      break;
    case LT:
      getNextToken();
      relationalParser();
      break;
    case GE:
      getNextToken();
      relationalParser();
      break;
    case GT:
      getNextToken();
      relationalParser();
      break;
    default:
      return;
      break;
    }
  }
  else{
    line();printf("ERROR: Missing or invalid AND, OR, RPAR, an identifier, SC, COMMA, RPAR, MINUS, NOT, LPAR or a literal.\n");
    errorMessage();
    getNextToken();
  }
}

static void equivalenceParser(){
  if(curTokType(CurTok)){
    relationalParser();

    if(CurTok.type == EQ || CurTok.type == NE){
      getNextToken();
      equivalenceParser();
      //if relationa equivalence'
      //call equivalence
    }
  }
  else{
    line();printf("ERROR: Missing or invalid AND, OR, RPAR, an identifier, SC, COMMA, RPAR, MINUS, NOT, LPAR or a literal.");
    errorMessage();   
    getNextToken();
  }
}

static void termParser(){
  if(curTokType(CurTok)){
    equivalenceParser();
    TOKEN storeCurrent =  CurTok;
    if (CurTok.type == AND){
      getNextToken();
      termParser();
      //IF(TERM && EQUIVALENCE)
    }
    else{
      return;
      //IF epsiolon then just return equivalence
    }
  }
  else if(!AndTerm()){
    line();printf("ERROR: Missing or invalid AND, OR, RPAR, an identifier, SC, COMMA, RPAR, MINUS, NOT, LPAR or a literal.\n");
    errorMessage();   
    getNextToken();
    return;
  }
  else {
    line();printf("ERROR: Missing term -> Expected a literal, variable, identity, '(', '!', or '-'\n");
    errorMessage();  
    getNextToken(); 
  }
  return;
}

static void rvalParser(){
  if(curTokType(CurTok)){
    termParser();
    TOKEN storeCurrent = CurTok;
    if(CurTok.type == OR){
      getNextToken();
      rvalParser();
      return;
    }
    return;
  }
  else if(!OrTerm()){
    line();printf("ERROR: Missing or invalid AND, OR, RPAR, an identifier, SC, COMMA, RPAR, MINUS, NOT, LPAR or a literal.\n");
    errorMessage();   
    getNextToken();
    return;
  }
  else{
    line();printf("ERROR: Missing rval -> Expected a literal, variable, identity, '(', '!', or '-' or '||'\n"); 
    errorMessage();   
    getNextToken();
  }
}

static std::unique_ptr<ASTnode> expressionParser(){
  if (CurTok.type == IDENT){
    TOKEN temporaryIdentifierStorage = CurTok;
    getNextToken();
    if(CurTok.type == ASSIGN){
      //AST NODE
      getNextToken();
      expressionParser();
      //AST SOMETHING OR OTHER
      return nullptr;
    }
    putBackToken(CurTok);
    putBackToken(temporaryIdentifierStorage);
    getNextToken();
  }
  if (curTokType(CurTok)){
    rvalParser();
    return nullptr;
  }
  else{
    line();printf("ERROR: Missing assignment or expression \n");
    errorMessage();
    getNextToken();
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
    /*auto x = */ expressionParser();
    if(CurTok.type == SC){
      //printf("%s - %d\n", CurTok.lexeme.c_str(), CurTok.type);
      getNextToken();
      //printf("%s - %d\n", CurTok.lexeme.c_str(), CurTok.type);
    }
    else{
      line();printf("ERROR: No semi colon at line end, instead Token ");
      printf("%s", CurTok.lexeme.c_str());
      printf(" was encountered rather than ';' as expected.\n");
      errorMessage();  
      getNextToken(); 
    }
    if(!(CurTok.type == EOF_TOK || CurTok.type == EOF || CurTok.type==IDENT || CurTok.type==SC || CurTok.type==LBRA || CurTok.type==WHILE || CurTok.type==IF || CurTok.type==RETURN || CurTok.type==MINUS || CurTok.type==NOT || CurTok.type==LPAR || CurTok.type==INT_LIT || CurTok.type==BOOL_LIT || CurTok.type==FLOAT_LIT || CurTok.type==RBRA)){
      line();printf("ERROR: Missing identifier, or SC ';', LBRA '{', RBRA '{', WHILE, IF, MINUS '-', NOT '!', LPAR '(' RETURN, or a literal.\n");
      errorMessage();  
      getNextToken(); 
    }
  }
  return nullptr;
}


static std::unique_ptr<ASTnode> statementParser(){
  if(CurTok.type == IF) //call if;
    return nullptr;
  else if(CurTok.type == WHILE)//call while;
    return nullptr;
  else if(CurTok.type == RETURN) //call block
    return nullptr;
  else if(CurTok.type == LBRA) //call block;
    return nullptr;
  else{
    auto expressionStatements = expressionStatementParser();
    if(expressionStatements) return std::move(expressionStatements);
    return nullptr;
  }
  line();printf("ERROR: No statement definition\n");
  errorMessage();
  return nullptr;
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
    statementParser();
    while(CurTok.type == INT_LIT || CurTok.type == BOOL_LIT || CurTok.type == FLOAT_LIT || CurTok.type == NOT || CurTok.type == MINUS || CurTok.type == SC || CurTok.type == LPAR || CurTok.type == IDENT || CurTok.type == IF || CurTok.type == RETURN || CurTok.type == WHILE || CurTok.type == LBRA){
      statementParser();
    }
  }
  else{
    line();printf("ERROR: Statement defined incorrectly\n");
    errorMessage();
    getNextToken();
  }
  return statements;
}

static std::unique_ptr<typeASTnode> variableTypeParser(){
  if(CurTok.type != INT_TOK || CurTok.type != BOOL_TOK || CurTok.type != FLOAT_TOK){
    line();printf("ERROR: Missing variable type, expected either INT, BOOL or FLOAT\n");
    errorMessage();
  }
  if(CurTok.type == INT_TOK){
    if(CurTok.type == INT_TOK){
      TOKEN returnTok = CurTok;
      getNextToken();
      return std::make_unique<typeASTnode>(returnTok);
    }
    line();printf("ERROR: %s does not match expected type INT\n", CurTok.lexeme.c_str());
    errorMessage();
    getNextToken();
  }
  if(CurTok.type == BOOL_LIT){
    if(CurTok.type == BOOL_LIT){
      TOKEN returnTok = CurTok;
      getNextToken();
      return std::make_unique<typeASTnode>(returnTok);
    }
    line();printf("ERROR: %s does not match expected type BOOL\n", CurTok.lexeme.c_str());
    errorMessage();
    getNextToken();
  }
  if(CurTok.type == FLOAT_TOK){
    if(CurTok.type == FLOAT_TOK){
      TOKEN returnTok = CurTok;
      getNextToken();
      return std::make_unique<typeASTnode>(returnTok);
    }
    line();printf("ERROR: %s does not match expected type FLOAT\n", CurTok.lexeme.c_str());
    errorMessage();
    getNextToken();
  }
  return nullptr;
}

static std::unique_ptr<ASTnode> localDeclParser(){
  if(!(CurTok.type == INT_TOK || CurTok.type == BOOL_TOK || CurTok.type == FLOAT_TOK)){
    line();printf("ERROR: Locally declared variable has no type\n");
    errorMessage();
  }
  else{
    auto variableType = variableTypeParser();
    if(CurTok.type == IDENT){
      getNextToken();
      if(CurTok.type == SC){
        getNextToken();
      }
      else{
        line();printf("ERROR: Missing semi colon at end of declaration. Expected ';'\n");
        errorMessage();
        getNextToken();
      }
    }
    else{
      line();printf("ERROR: Missing IDENT in declaration. Expected 'IDENT'\n");
      errorMessage();
      getNextToken();
    }
  }
  return nullptr;
}

static std::vector<std::unique_ptr<ASTnode>> localDeclsParser(){
  std::vector<std::unique_ptr<ASTnode>> declarations;
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
    getNextToken();
    return nullptr;
  }
  else{
    getNextToken();
    auto declarations = localDeclsParser();
    auto statements = statementListParser();
    if(CurTok.type != RBRA){
      line();printf("ERROR: Missing RBRA at end of block, expected to find '}'\n");
      errorMessage();
      getNextToken();
      return nullptr;
    }
    else{
      getNextToken();
      return std::make_unique<BlockASTnode>(std::move(declarations), std::move(statements));
    }
  }
}

static std::unique_ptr<BlockASTnode> elseParser(){
  if(CurTok.type != ELSE){
    line();printf("ERROR: missing 'ELSE' declaration at the beginning of else block\n");
    errorMessage();
    getNextToken();
  }
  else{
    getNextToken();
    if(CurTok.type != LBRA){
      line();printf("ERROR: missing LBRA '{' after 'ELSE'\n");
      errorMessage();
      getNextToken();
    }
    auto blockstatement = blockParser();
    if(CurTok.type==IDENT || CurTok.type==SC || CurTok.type==LBRA || CurTok.type==WHILE || CurTok.type==IF || CurTok.type==RETURN || CurTok.type==MINUS || CurTok.type==NOT || CurTok.type==LPAR || CurTok.type==INT_LIT || CurTok.type==BOOL_LIT || CurTok.type==FLOAT_LIT || CurTok.type==RBRA ) {
      return blockstatement;    
    }
    else{
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

    if(CurTok.type==IDENT || CurTok.type==SC || CurTok.type==LBRA || CurTok.type==WHILE || CurTok.type==IF || CurTok.type==RETURN || CurTok.type==MINUS || CurTok.type==NOT || CurTok.type==LPAR || CurTok.type==INT_LIT || CurTok.type==BOOL_LIT || CurTok.type==FLOAT_LIT || CurTok.type==RBRA ) {
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
      return variableTypeParser();
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

static void functionDeclarationParser(){
  auto nuller = typeSpecParser();
  if(CurTok.type != IDENT){
    line();printf("ERROR: Expected an identifier\n");
    errorMessage();
  }
  getNextToken();
  if(CurTok.type != LPAR){
    line();printf("ERROR: Missing LPAR '('\n");
    errorMessage();
  }
  getNextToken();
  //CALL PARAMS
  if(CurTok.type != RPAR){
    line();printf("ERROR: Missing RPAR ')'\n");
    errorMessage();
  }
  getNextToken();
  //CALL BLOCK
}

// program ::= extern_list decl_list
static void parser() {
  // add body
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
  while(CurTok.type != EOF_TOK){
    auto blank = statementListParser();
  }
  if(errorCount > 0) printf("============================\n");
  printf("%d Errors found\n", errorCount);
  fprintf(stderr, "Lexer Finished\n");
  

  // Make the module, which holds all the code.
  TheModule = std::make_unique<Module>("mini-c", TheContext);


  // Run the parser now.
  parser();
  fprintf(stderr, "Parsing Finished\n");

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

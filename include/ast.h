
#ifndef AST_H
#define AST_H

#include "parser.h"

struct ast_node {

  enum {
    NONE = 0,
    FUNCTION,
    FUNCTION_CALL,
    STATEMENT,
    STRUCT,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    INT,
    FLOAT,
    VOID,
    INT_LITERAL,
    FLOAT_LITERAL,
    BOOL_LITERAL,
    STRING_LITERAL,
    VARIABLE,
    RETURN,
    ASSIGNMENT
  };

  std::string Id;
  std::vector<ast_node> Children;
  int Type;
  float FloatValue;
  long IntValue;

  enum { DECLARE = 1 << 0 };
  int Modifiers;

  void Append(const ast_node &A) {
    Children.insert(Children.end(), A.Children.begin(), A.Children.end());
  }
};

class ast {
  symtable *SymbolTable;

public:
  ast(symtable *S);
  ast_node BuildStatement(parse_node &P);
  ast_node BuildStatementList(parse_node &P);
  ast_node BuildFunctionCall(parse_node &P);
  ast_node BuildPrimaryExpression(parse_node &P);
  ast_node BuildAssignmentExpression(parse_node &P);
  ast_node BuildFunctionDefinition(parse_node &P);
  ast_node BuildDeclaration(parse_node &P);
  static ast_node BuildTranslationUnit(parse_node &P, symtable *S);
};

#endif

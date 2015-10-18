
#ifndef AST_H
#define AST_H

#include "parser.h"
#include <vector>
#include <string>

struct ast_node {

  enum {
    NONE = 0,
    FUNCTION,
    FUNCTION_CALL,
    STATEMENT,
    STRUCT,
    PLUS,
    MINUS,
    STAR,
    DIVIDE,
    INT,
    FLOAT,
    VOID,
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    VARIABLE,
    RETURN,
    ASSIGNMENT
  };

  int Type;
  int VarType;
  std::string Id;
  std::vector<ast_node> TypeTable;
  std::vector<ast_node> Children;
  float FloatValue;
  long IntValue;
};

ast_node ASTBuildFromParseTree(parse_node *Node);
ast_node ASTBuildFromIdentifier(parse_node *Node, parse_node *PNode);
ast_node ASTBuildStatement(parse_node *Node);

#endif

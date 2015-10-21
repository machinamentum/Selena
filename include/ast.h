
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
  std::vector<ast_node> DefinedTypes;
  float FloatValue;
  long IntValue;
  std::string Typename;
  ast_node *Parent;

  ast_node(ast_node *ASTParent) : Parent(ASTParent) {}

  void PushChild(ast_node Child) {
    Child.Parent = this;
    Children.push_back(Child);
  }

  ast_node *LookupType(std::string Typename);

  static ast_node BuildFunction(ast_node *Parent, parse_node *Node);
  static ast_node BuildFromIdentifier(ast_node *Parent, parse_node *PNode);
  static ast_node BuildFromParseTree(ast_node *Parent, parse_node *Node);
  static ast_node BuildStatement(ast_node *Parent, parse_node *Node);
  static ast_node BuildReturn(ast_node *Parent, parse_node *Node);
  static ast_node BuildStruct(ast_node *Parent, parse_node *Node);
  static ast_node BuildFunctionCall(ast_node *Parent, parse_node *Node);
  static ast_node BuildVariable(ast_node *Parent, parse_node *Node);
  static ast_node BuildConstantPrimitive(ast_node *Parent, parse_node *Node);
};

#endif

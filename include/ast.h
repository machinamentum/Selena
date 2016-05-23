
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
    STRING_LITERAL,
    VARIABLE,
    RETURN,
    ASSIGNMENT
  };

  enum { INLINE = 1, DECLARE = (1 << 1) };

  int Type = 0;
  int VarType = 0;
  int Modifiers = 0;
  std::string Id;
  std::vector<ast_node *> Children;
  float FloatValue;
  long IntValue;
  std::string Typename;
  ast_node *Parent = nullptr;

  ast_node(ast_node *ASTParent) : Parent(ASTParent) {}

  void PushChild(ast_node *Child) {
    Child->Parent = this;
    Children.push_back(Child);
  }

  ast_node *LookupType(std::string Typename);
  ast_node *LookupFunction(std::string FuncName);

  static ast_node *BuildFunction(ast_node *Parent, parse_node *Node);
  static ast_node *BuildFromIdentifier(ast_node *Parent, parse_node *PNode);
  static ast_node *BuildFromParseTree(ast_node *Parent, parse_node *Node);
  static ast_node *BuildStatement(ast_node *Parent, parse_node *Node);
  static ast_node *BuildReturn(ast_node *Parent, parse_node *Node);
  static ast_node *BuildStruct(ast_node *Parent, parse_node *Node);
  static ast_node *BuildFunctionCall(ast_node *Parent, parse_node *Node);
  static ast_node *BuildVariable(ast_node *Parent, parse_node *Node);
  static ast_node *BuildConstantPrimitive(ast_node *Parent, parse_node *Node);
};

#endif

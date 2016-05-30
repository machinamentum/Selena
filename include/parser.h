
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

struct parse_node {

  enum NodeType : int {
    DECLARATION,
    FUNCTION_DEFINITION,
    FUNCTION_CALL,
    TYPE_QUALIFIER,
    TYPE_SPECIFIER,
    PRECISION_QUALIFER,
    ASSIGNMENT_EXPR,
    CONDITIONAL_EXPR,
    PRIMARY_EXPRESSION,
    EXPRESSION,
    LOGOR_EXPR,
    MULTIPLY_EXPR,
    E,
    T
  };

  std::vector<parse_node> Children;
  token Token;
  int Type;

  parse_node() { Type = E; }

  parse_node(token &Tok, NodeType NT = T) {
    Token = Tok;
    Type = NT;
  }

  parse_node(NodeType NT) { Type = NT; }

  void Append(const parse_node &P) {
    Children.insert(Children.end(), P.Children.begin(), P.Children.end());
  }

  bool Empty() { return Children.size() == 0 && Type == E; }
};

struct parser {
  lexer_state &Lex;
  token Token;
  struct parse_state {
    lexer_state L;
    token T;
  };
  std::vector<parse_state> ParseStateStack;
  int ErrorDisableCount;
  symtable *SymbolTable;
  void (*ErrorFunc)(const std::string &, const std::string &, int, int);

  typedef parse_node (parser::*ParseFuncPtr)();

  parser(lexer_state &L);
  void Match(int T);
  void DisableErrors();
  void EnableErrors();
  void PushState();
  void PopState();
  void RestoreState();

  static bool IsAssignmentOp(int T);
  static bool IsTypeQualifier(int T);
  static bool IsPrecisionQualifier(int T);
  static bool IsIterationToken(int T);
  static bool IsJumpToken(int T);
  static bool IsTypeSpecifier(int T);
  static bool IsConstructorIdentifier(int T);
  static bool IsParameterQualifier(int T);

  void GenError(const std::string &S, const token &T);

  parse_node ParseTypeSpecifier();
  parse_node ParseFullySpecifiedType();
  parse_node ParseParameterDeclaration();
  parse_node ParseFunctionHeaderWithParameters();
  parse_node ParseFunctionHeader();
  parse_node ParseFunctionDeclarator();
  parse_node ParseFunctionPrototype();
  parse_node ParseFunctionDefinition();

  parse_node ParseFunctionCall();
  parse_node ParseIntegerExpression();
  parse_node ParsePostfixExpression();
  parse_node ParsePrimaryExpression();
  parse_node ParseUnaryExpression();
  parse_node ParseOperatorExpression(ParseFuncPtr R,
                                     std::vector<int> TokenTypes);
  parse_node ParseEqualityExpression();
  parse_node ParseMultiplicativeExpression();
  parse_node ParseAdditiveExpression();
  parse_node ParseRelationalExpression();
  parse_node ParseShiftExpression();
  parse_node ParseAndExpression();
  parse_node ParseExclusiveOrExpression();
  parse_node ParseInclusiveOrExpression();
  parse_node ParseLogicalAndExpression();
  parse_node ParseLogicalXOrExpression();
  parse_node ParseAssignmentOperator();
  parse_node ParseLogicalOrExpression();
  parse_node ParseConditionalExpression();
  parse_node ParseAssignmentExpression();
  parse_node ParseExpression();
  parse_node ParseExpressionStatement();
  parse_node ParseSimpleStatement();
  parse_node ParseStatementNoNewScope();
  parse_node ParseCompoundStatementWithScope();
  parse_node ParseStatementList();
  parse_node ParseCompoundStatementNoNewScope();

  parse_node ParseCondition();
  parse_node ParseStatementWithScope();
  parse_node ParseSelectionStatement();
  parse_node ParseIterationStatement();
  parse_node ParseJumpStatement();
  parse_node ParseDeclarationStatement();
  parse_node ParseTypeQualifier();
  parse_node ParseConstantExpression();
  parse_node ParseStructDeclarator();
  parse_node ParseStructDeclaratorList();
  parse_node ParseStructDeclaration();
  parse_node ParseStructDeclarationList();
  parse_node ParseStructSpecifier();
  parse_node ParseInitializer();
  parse_node ParsePrecisionQualifier();
  parse_node ParseTypeSpecifierNoPrecision();
  parse_node ParseInitDeclaratorList();
  parse_node ParseSingleDeclaration();
  parse_node ParseDeclaration();
  parse_node ParseExternalDeclaration();
  parse_node ParseTranslationUnit();
};

#endif

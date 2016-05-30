
#include "ast.h"

ast::ast(symtable *S) : SymbolTable(S) {}

ast_node ast::BuildFunctionCall(parse_node &P) {
  ast_node A;
  A.Type = ast_node::FUNCTION_CALL;
  A.Id = P.Children[0].Token.Id;
  for (parse_node &PN : P.Children[2].Children) {
    if (PN.Token.Type == token::COMMA)
      continue;
    A.Children.push_back(BuildAssignmentExpression(PN));
  }
  return A;
}

ast_node ast::BuildPrimaryExpression(parse_node &P) {
  ast_node A;
  switch (P.Token.Type) {
  case token::INTCONSTANT:
    A.Type = ast_node::INT_LITERAL;
    A.IntValue = P.Token.IntValue;
    break;
  case token::FLOATCONSTANT:
    A.Type = ast_node::FLOAT_LITERAL;
    A.FloatValue = P.Token.FloatValue;
    break;
  case token::BOOLCONSTANT:
    A.Type = ast_node::BOOL_LITERAL;
    A.IntValue = P.Token.BoolValue;
    break;
  case token::IDENTIFIER:
    A.Type = ast_node::VARIABLE;
    A.Id = P.Token.Id;
    break;
  case token::DQSTRING:
    A.Type = ast_node::STRING_LITERAL;
    A.Id = P.Token.Id;
    break;
  }
  return A;
}

ast_node ast::BuildAssignmentExpression(parse_node &P) {
  ast_node A;
  A.Type = ast_node::ASSIGNMENT;
  if (P.Children.size() == 3) {
    if (P.Children[1].Token.Type == token::STAR) {
      A.Type = ast_node::MULTIPLY;
    }
    A.Children.push_back(BuildPrimaryExpression(P.Children[0]));
    A.Children.push_back(BuildAssignmentExpression(P.Children[2]));
  } else if (P.Type == parse_node::PRIMARY_EXPRESSION) {
    return BuildPrimaryExpression(P);
  } else if (P.Children[0].Token.Type == token::EQUAL) {
    if (P.Children[1].Type == parse_node::FUNCTION_CALL) {
      A.Children.push_back(BuildFunctionCall(P.Children[1]));
    } else
      A.Children.push_back(BuildPrimaryExpression(P.Children[1]));
  } else if (P.Type == parse_node::FUNCTION_CALL) {
    return BuildFunctionCall(P);
  }
  return A;
}

ast_node ast::BuildStatement(parse_node &P) {
  ast_node A;
  switch (P.Type) {
  case parse_node::DECLARATION:
    return BuildDeclaration(P);
  case parse_node::E:
    return BuildStatement(P.Children[0]);
  case parse_node::ASSIGNMENT_EXPR:
    return BuildAssignmentExpression(P);
  case parse_node::EXPRESSION:
    return BuildAssignmentExpression(P.Children[0]);
  }
  return A;
}

ast_node ast::BuildStatementList(parse_node &P) {
  ast_node A;
  A.Type = ast_node::NONE;
  for (parse_node &PN : P.Children) {
    A.Children.push_back(BuildStatement(PN));
  }
  return A;
}

ast_node ast::BuildDeclaration(parse_node &P) {
  ast_node A;
  parse_node &Declarator = P.Children[0];
  std::string ID;
  int Qualifier;
  int Specifier;
  for (size_t i = 0; i < Declarator.Children.size(); ++i) {
    if (Declarator.Children[i].Type == parse_node::TYPE_QUALIFIER) {
      Qualifier = Declarator.Children[i].Token.Type;
    } else if (Declarator.Children[i].Type == parse_node::TYPE_SPECIFIER) {
      Specifier = Declarator.Children[i].Token.Type;
    } else if (Declarator.Children[i].Token.Type == token::IDENTIFIER) {
      ID = Declarator.Children[i].Token.Id;
    } else if (Declarator.Children[i].Type == parse_node::E &&
               Declarator.Children[i].Children[0].Token.Type == token::EQUAL) {
      A.Children.push_back(BuildAssignmentExpression(Declarator.Children[i]));
    } else if (Declarator.Children[i].Token.Type == token::SEMICOLON) {
      break;
    }
  }

  symtable_entry *E = SymbolTable->Lookup(ID);
  E->TypeSpecifier = Specifier;
  E->Qualifier = Qualifier;
  A.Type = ast_node::VARIABLE;
  A.Modifiers = ast_node::DECLARE;
  A.Id = ID;
  return A;
}

ast_node ast::BuildFunctionDefinition(parse_node &P) {
  ast_node A;
  A.Type = ast_node::FUNCTION;
  parse_node &Declarator = P;
  std::string ID;
  int Qualifier;
  int Specifier;
  for (size_t i = 0; i < Declarator.Children.size(); ++i) {
    if (Declarator.Children[i].Type == parse_node::TYPE_QUALIFIER) {
      Qualifier = Declarator.Children[i].Token.Type;
    } else if (Declarator.Children[i].Type == parse_node::TYPE_SPECIFIER) {
      Specifier = Declarator.Children[i].Token.Type;
    } else if (Declarator.Children[i].Token.Type == token::IDENTIFIER) {
      ID = Declarator.Children[i].Token.Id;
    } else if (Declarator.Children[i].Token.Type == token::LEFT_PAREN) {
      // TODO params
      ++i;
    } else if (Declarator.Children[i].Token.Type == token::LEFT_BRACE) {
      A.Children.push_back(BuildStatementList(Declarator.Children[++i]));
    }
  }

  symtable_entry *E = SymbolTable->Lookup(ID);
  E->TypeSpecifier = Specifier;
  E->Qualifier = Qualifier;
  E->Definition = ast_node::FUNCTION;
  A.Id = ID;
  return A;
}

ast_node ast::BuildTranslationUnit(parse_node &P, symtable *S) {
  ast AST = ast(S);
  ast_node RootNode;
  for (parse_node &PN : P.Children) {
    if (PN.Type == parse_node::DECLARATION) {
      RootNode.Children.push_back(AST.BuildDeclaration(PN));
    } else if (PN.Type == parse_node::FUNCTION_DEFINITION) {
      RootNode.Children.push_back(AST.BuildFunctionDefinition(PN));
    }
  }
  return RootNode;
}

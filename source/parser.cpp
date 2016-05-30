#include "parser.h"
#include <cstdlib>
#include <functional>

void parser::GenError(const std::string &S, const token &T) {
  std::string Line = LexerGetLine(&Lex, T.Line);
  if (ErrorFunc)
    if (ErrorDisableCount == 0)
      ErrorFunc(S, Line, T.Line, T.Offset);
}

parser::parser(lexer_state &L) : Lex(L) { SymbolTable = Lex.Table; }

void parser::Match(int T) {
  if (T == Token.Type) {
    Token = Lex.GetToken();
  } else {
    GenError("expected " + TokenToString(T) + " before token " +
                 TokenToString(Token),
             Token);
  }
}

void parser::DisableErrors() { ++ErrorDisableCount; }

void parser::EnableErrors() { --ErrorDisableCount; }

void parser::PushState() {
  ParseStateStack.push_back((parse_state){Lex, Token});
}

void parser::PopState() {
  parse_state S = ParseStateStack.back();
  Lex = S.L;
  Token = S.T;
  ParseStateStack.pop_back();
}

void parser::RestoreState() {
  parse_state S = ParseStateStack.back();
  Lex = S.L;
  Token = S.T;
}

parse_node parser::ParseTypeSpecifier() {
  parse_node N;
  if (IsPrecisionQualifier(Token.Type)) {
    N.Children.push_back(ParsePrecisionQualifier());
  }
  N.Children.push_back(ParseTypeSpecifierNoPrecision());
  return N;
}

parse_node parser::ParseTypeQualifier() {
  parse_node N;
  if (Token.Type == token::INVARIANT) {
    N.Children.push_back(parse_node(Token));
    Match(token::INVARIANT);
    N.Children.push_back(parse_node(Token));
    Match(token::VARYING);
  } else if (IsTypeQualifier(Token.Type)) {
    N.Children.push_back(parse_node(Token, parse_node::TYPE_QUALIFIER));
    Match(Token.Type);
  } else {
    GenError("expected type qualifier before token " + TokenToString(Token),
             Token);
  }

  return N;
}

parse_node parser::ParseFullySpecifiedType() {
  parse_node N;
  if (IsTypeQualifier(Token.Type)) {
    N.Append(ParseTypeQualifier());
  }
  N.Append(ParseTypeSpecifier());
  return N;
}

parse_node parser::ParseParameterDeclaration() {
  parse_node N;
  if (IsParameterQualifier(Token.Type)) {
    N.Children.push_back(parse_node(Token));
    Match(Token.Type);
    if (IsPrecisionQualifier(Token.Type) || IsTypeSpecifier(Token.Type)) {
      N.Append(ParseTypeSpecifier());
      N.Children.push_back(parse_node(Token));
      Match(token::LEFT_BRACKET);
      N.Children.push_back(ParseConstantExpression());
      N.Children.push_back(parse_node(Token));
      Match(token::RIGHT_BRACKET);
    } else {
      N.Append(ParseTypeSpecifier());
      N.Children.push_back(parse_node(Token));
      Match(token::IDENTIFIER);
      if (Token.Type == token::LEFT_BRACKET) {
        N.Children.push_back(parse_node(Token));
        Match(token::LEFT_BRACKET);
        N.Children.push_back(ParseConstantExpression());
        N.Children.push_back(parse_node(Token));
        Match(token::RIGHT_BRACKET);
      }
    }
  } else if (IsPrecisionQualifier(Token.Type) || IsTypeSpecifier(Token.Type)) {
    if (IsTypeQualifier(Token.Type)) {
      N.Append(ParseTypeQualifier());
    }
    if (IsParameterQualifier(Token.Type)) {
      N.Children.push_back(parse_node(Token));
      Match(Token.Type);
    }
    N.Append(ParseTypeSpecifier());
    if (Token.Type == token::IDENTIFIER) {
      N.Children.push_back(parse_node(Token));
      Match(token::IDENTIFIER);
      if (Token.Type == token::LEFT_BRACKET) {
        N.Children.push_back(parse_node(Token));
        Match(token::LEFT_BRACKET);
        N.Children.push_back(ParseConstantExpression());
        N.Children.push_back(parse_node(Token));
        Match(token::RIGHT_BRACKET);
      }
    } else if (Token.Type == token::LEFT_BRACKET) {
      if (Token.Type == token::LEFT_BRACKET) {
        N.Children.push_back(parse_node(Token));
        Match(token::LEFT_BRACKET);
        N.Children.push_back(ParseConstantExpression());
        N.Children.push_back(parse_node(Token));
        Match(token::RIGHT_BRACKET);
      }
    } else {
      GenError("unexpected token", Token);
    }
  }

  return N;
}

parse_node parser::ParseFunctionHeader() {
  parse_node N;
  N.Append(ParseFullySpecifiedType());
  N.Children.push_back(Token);
  Match(token::IDENTIFIER);
  N.Children.push_back(parse_node(Token));
  Match(token::LEFT_PAREN);
  return N;
}

parse_node parser::ParseFunctionDeclarator() {
  parse_node N = ParseFunctionHeader();
  if (Token.Type != token::RIGHT_PAREN) {
    N.Children.push_back(ParseParameterDeclaration());
    while (Token.Type == token::COMMA) {
      N.Children.push_back(parse_node(Token));
      Match(token::COMMA);
      N.Children.push_back(ParseParameterDeclaration());
    }
  } else {
    N.Children.push_back(parse_node());
  }
  return N;
}

parse_node parser::ParseFunctionPrototype() {
  parse_node N;
  N.Append(ParseFunctionDeclarator());
  N.Children.push_back(parse_node(Token));
  Match(token::RIGHT_PAREN);
  return N;
}

bool parser::IsAssignmentOp(int T) {
  switch (T) {
  case token::EQUAL:
  case token::MUL_ASSIGN:
  case token::DIV_ASSIGN:
  case token::ADD_ASSIGN:
  case token::SUB_ASSIGN:
  case token::MOD_ASSIGN:
  case token::LEFT_ASSIGN:
  case token::RIGHT_ASSIGN:
  case token::AND_ASSIGN:
  case token::XOR_ASSIGN:
  case token::OR_ASSIGN:
    return true;
  default:
    return false;
  }
}

bool parser::IsTypeQualifier(int T) {
  switch (T) {
  case token::CONST:
  case token::ATTRIBUTE:
  case token::VARYING:
  case token::INVARIANT:
  case token::UNIFORM:
    return true;
  }
  return false;
}

bool parser::IsPrecisionQualifier(int T) {
  switch (T) {
  case token::HIGH_PRECISION:
  case token::MEDIUM_PRECISION:
  case token::LOW_PRECISION:
    return true;
  }
  return false;
}

bool parser::IsIterationToken(int T) {
  switch (T) {
  case token::WHILE:
  case token::DO:
  case token::FOR:
    return true;
  }
  return false;
}

bool parser::IsJumpToken(int T) {
  switch (T) {
  case token::CONTINUE:
  case token::BREAK:
  case token::RETURN:
  case token::DISCARD:
    return true;
  }
  return false;
}

bool parser::IsTypeSpecifier(int T) {
  switch (T) {
  case token::VOID:
  case token::FLOAT:
  case token::INT:
  case token::BOOL:
  case token::VEC2:
  case token::VEC3:
  case token::VEC4:
  case token::BVEC2:
  case token::BVEC3:
  case token::BVEC4:
  case token::IVEC2:
  case token::IVEC3:
  case token::IVEC4:
  case token::MAT2:
  case token::MAT3:
  case token::MAT4:
  case token::SAMPLER2D:
  case token::SAMPLERCUBE:
  case token::TYPE_NAME:
  case token::STRUCT:
    return true;
  }
  return false;
}

bool parser::IsConstructorIdentifier(int T) {
  switch (T) {
  case token::FLOAT:
  case token::INT:
  case token::BOOL:
  case token::VEC2:
  case token::VEC3:
  case token::VEC4:
  case token::BVEC2:
  case token::BVEC3:
  case token::BVEC4:
  case token::IVEC2:
  case token::IVEC3:
  case token::IVEC4:
  case token::MAT2:
  case token::MAT3:
  case token::MAT4:
  case token::TYPE_NAME:
    return true;
  }
  return false;
}

bool parser::IsParameterQualifier(int T) {
  return T == token::IN || T == token::OUT || T == token::INOUT;
}

parse_node parser::ParseAssignmentOperator() {
  switch (Token.Type) {
  case token::MOD_ASSIGN:
  case token::LEFT_ASSIGN:
  case token::RIGHT_ASSIGN:
  case token::AND_ASSIGN:
  case token::XOR_ASSIGN:
  case token::OR_ASSIGN:
    GenError("use of reserved operator " + TokenToString(Token) + " is illegal",
             Token);
  case token::EQUAL:
  case token::MUL_ASSIGN:
  case token::DIV_ASSIGN:
  case token::ADD_ASSIGN:
  case token::SUB_ASSIGN: {
    parse_node N = parse_node(Token);
    Match(Token.Type);
    return N;
  }

  default:
    GenError("expected assignement operator before token " +
                 TokenToString(Token),
             Token);
    return parse_node();
  }
}

parse_node parser::ParseFunctionCall() {
  parse_node N = parse_node(parse_node::FUNCTION_CALL);
  if (!(Token.Type == token::IDENTIFIER ||
        IsConstructorIdentifier(Token.Type) || Token.Type == token::ASM)) {
    GenError("expected function identifier or type constructor before token " +
                 TokenToString(Token),
             Token);
  }
  N.Children.push_back(parse_node(Token));
  Match(Token.Type);
  N.Children.push_back(parse_node(Token));
  Match(token::LEFT_PAREN);
  if (Token.Type != token::RIGHT_PAREN) {
    if (Token.Type == token::VOID) {
      N.Children.push_back(parse_node(Token));
      Match(token::VOID);
      N.Children.push_back(parse_node(Token));
      Match(token::RIGHT_PAREN);
    } else {
      parse_node NSub;
      NSub.Children.push_back(ParseAssignmentExpression());
      while (Token.Type == token::COMMA) {
        NSub.Children.push_back(parse_node(Token));
        Match(token::COMMA);
        NSub.Children.push_back(ParseAssignmentExpression());
      }
      N.Children.push_back(NSub);
      N.Children.push_back(parse_node(Token));
      Match(token::RIGHT_PAREN);
    }
  } else {
    N.Children.push_back(parse_node());
    N.Children.push_back(parse_node(Token));
    Match(token::RIGHT_PAREN);
  }

  return N;
}

parse_node parser::ParseIntegerExpression() { return ParseExpression(); }

parse_node parser::ParseOperatorExpression(ParseFuncPtr R,
                                           std::vector<int> TokenTypes) {
#define CALL_MEMBER_FN(object, ptrToMember) ((object).*(ptrToMember))
  auto IsOfType = [](int T, std::vector<int> &v) -> bool {
    for (int &i : v) {
      if (i == T)
        return true;
    }
    return false;
  };
  std::function<parse_node(parse_node &)> ParseExtL =
      [&](parse_node &P) -> parse_node {
    parse_node N;
    if (IsOfType(Token.Type, TokenTypes)) {
      N.Children.push_back(P);
      N.Children.push_back(parse_node(Token));
      Match(Token.Type);
      N.Children.push_back(CALL_MEMBER_FN(*this, R)());
      parse_node C = ParseExtL(N);
      if (!C.Empty()) {
        return C;
      }
    }
    return N;
  };
  parse_node RP = CALL_MEMBER_FN(*this, R)();
  parse_node LP = ParseExtL(RP);
  if (!LP.Empty()) {
    return LP;
  }
  return RP;
#undef CALL_MEMBER_FN
}

parse_node parser::ParseMultiplicativeExpression() {
  return ParseOperatorExpression(&parser::ParseUnaryExpression,
                                 {token::STAR, token::SLASH, token::PERCENT});
}

parse_node parser::ParseAdditiveExpression() {
  return ParseOperatorExpression(&parser::ParseMultiplicativeExpression,
                                 {token::PLUS, token::DASH});
}

parse_node parser::ParseShiftExpression() {
  return ParseOperatorExpression(&parser::ParseAdditiveExpression,
                                 {token::LEFT_OP, token::RIGHT_OP});
}

parse_node parser::ParseRelationalExpression() {
  return ParseOperatorExpression(
      &parser::ParseShiftExpression,
      {token::LEFT_ANGLE, token::RIGHT_ANGLE, token::LE_OP, token::GE_OP});
}

parse_node parser::ParseEqualityExpression() {
  return ParseOperatorExpression(&parser::ParseRelationalExpression,
                                 {token::EQ_OP, token::NE_OP});
}

parse_node parser::ParseAndExpression() {
  return ParseOperatorExpression(&parser::ParseEqualityExpression,
                                 {token::AMPERSAND});
}

parse_node parser::ParseExclusiveOrExpression() {
  return ParseOperatorExpression(&parser::ParseAndExpression, {token::CARET});
}

parse_node parser::ParseInclusiveOrExpression() {
  return ParseOperatorExpression(&parser::ParseExclusiveOrExpression,
                                 {token::VERTICLE_BAR});
}

parse_node parser::ParseLogicalAndExpression() {
  return ParseOperatorExpression(&parser::ParseInclusiveOrExpression,
                                 {token::AND_OP});
}

parse_node parser::ParseLogicalXOrExpression() {
  return ParseOperatorExpression(&parser::ParseLogicalAndExpression,
                                 {token::XOR_OP});
}

parse_node parser::ParseLogicalOrExpression() {
  return ParseOperatorExpression(&parser::ParseLogicalXOrExpression,
                                 {token::OR_OP});
}

parse_node parser::ParsePrimaryExpression() {
  parse_node N = parse_node(parse_node::PRIMARY_EXPRESSION);
  if (Token.Type == token::LEFT_PAREN) {
    N.Children.push_back(parse_node(Token));
    Match(token::LEFT_PAREN);
    N.Children.push_back(ParseExpression());
    N.Children.push_back(parse_node(Token));
    Match(token::RIGHT_PAREN);
    return N;
  }
  switch (Token.Type) {
  case token::INTCONSTANT:
  case token::FLOATCONSTANT:
  case token::BOOLCONSTANT:
  case token::IDENTIFIER:
  case token::DQSTRING:
    N = parse_node(Token, parse_node::PRIMARY_EXPRESSION);
    Match(Token.Type);
    return N;

  default: {
    GenError("unexpected token " + TokenToString(Token) +
                 " in pimary expression",
             Token);
    return parse_node();
  }
  }
}

parse_node parser::ParsePostfixExpression() {
  std::function<parse_node(parse_node &)> ParseExtPostfix =
      [&](parse_node &P) -> parse_node {
    parse_node N;
    if (Token.Type == token::LEFT_BRACKET) {
      N.Children.push_back(P);
      Match(token::LEFT_BRACKET);
      N.Children.push_back(ParseIntegerExpression());
      Match(token::RIGHT_BRACKET);
    } else if (Token.Type == token::DEC_OP || Token.Type == token::INC_OP) {
      N.Children.push_back(parse_node(Token));
      Match(Token.Type);
    } else if (Token.Type == token::DOT) {
      N.Children.push_back(parse_node(Token));
      Match(token::DOT);
      N.Children.push_back(parse_node(Token));
      Match(token::FIELD_SELECTION);
    }
    return N;
  };
  parse_node Main;
  if (Lex.PeekToken().Type == token::LEFT_PAREN) {
    Main = ParseFunctionCall();
  } else {
    Main = ParsePrimaryExpression();
  }
  parse_node Ext = ParseExtPostfix(Main);
  if (!Ext.Empty()) {
    return Ext;
  }
  return Main;
}

parse_node parser::ParseUnaryExpression() {
  parse_node N;
  switch (Token.Type) {
  case token::INC_OP:
  case token::DEC_OP:
  case token::PLUS:
  case token::DASH:
  case token::BANG:
  case token::TILDE:
    N.Children.push_back(parse_node(Token));
    Match(Token.Type);
    N.Children.push_back(ParseUnaryExpression());
    return N;
  }
  return ParsePostfixExpression();
}

parse_node parser::ParseConditionalExpression() {
  parse_node N = parse_node(parse_node::CONDITIONAL_EXPR);
  parse_node L = ParseLogicalOrExpression();
  if (Token.Type != token::QUESTION) {
    return L;
  }
  Match(token::QUESTION);
  N.Children.push_back(ParseExpression());
  N.Children.push_back(parse_node(Token));
  Match(token::COLON);
  N.Children.push_back(ParseAssignmentExpression());
  return N;
}

parse_node parser::ParseAssignmentExpression() {
  PushState();
  DisableErrors();
  parse_node U = ParseUnaryExpression();
  if (!IsAssignmentOp(Token.Type)) {
    PopState();
    EnableErrors();
    parse_node Cond = ParseConditionalExpression();
    if (!IsAssignmentOp(Token.Type)) {
      return Cond;
    }
    U = Cond;
  } else {
    PopState();
    EnableErrors();
    U = ParseUnaryExpression();
  }
  parse_node N = parse_node(parse_node::ASSIGNMENT_EXPR);
  N.Children.push_back(U);
  N.Children.push_back(ParseAssignmentOperator());
  N.Children.push_back(ParseAssignmentExpression());
  return N;
}

parse_node parser::ParsePrecisionQualifier() {
  if (IsPrecisionQualifier(Token.Type)) {
    parse_node N = parse_node(Token, parse_node::PRECISION_QUALIFER);
    Match(Token.Type);
    return N;
  }

  GenError("expeceted precision qualifier before token " + TokenToString(Token),
           Token);
  return parse_node();
}

parse_node parser::ParseStructDeclarator() {
  parse_node N;
  N.Children.push_back(parse_node(Token));
  Match(token::IDENTIFIER);
  if (Token.Type == token::LEFT_BRACKET) {
    N.Children.push_back(parse_node(Token));
    Match(token::LEFT_BRACKET);
    N.Children.push_back(ParseConstantExpression());
    N.Children.push_back(parse_node(Token));
    Match(token::RIGHT_BRACKET);
  }
  return N;
}

parse_node parser::ParseStructDeclaratorList() {
  parse_node N;
  N.Children.push_back(ParseStructDeclarator());
  while (Token.Type == token::COMMA) {
    N.Children.push_back(ParseStructDeclarator());
  }
  return N;
}

parse_node parser::ParseStructDeclaration() {
  parse_node N;
  N.Children.push_back(ParseTypeSpecifier());
  N.Children.push_back(ParseStructDeclaratorList());
  N.Children.push_back(parse_node(Token));
  Match(token::SEMICOLON);
  return N;
}

parse_node parser::ParseStructDeclarationList() {
  parse_node N;
  while (Token.Type != token::RIGHT_BRACE) {
    N.Children.push_back(ParseStructDeclaration());
  }
  return N;
}

parse_node parser::ParseStructSpecifier() {
  parse_node N;
  N.Children.push_back(parse_node(Token));
  Match(token::STRUCT);
  if (Token.Type == token::IDENTIFIER) {
    N.Children.push_back(parse_node(Token));
    Match(token::IDENTIFIER);
  }
  N.Children.push_back(parse_node(Token));
  Match(token::LEFT_BRACE);
  N.Children.push_back(ParseStructDeclarationList());
  N.Children.push_back(parse_node(Token));
  Match(token::RIGHT_BRACE);
  return N;
}

parse_node parser::ParseTypeSpecifierNoPrecision() {
  switch (Token.Type) {
  case token::VOID:
  case token::FLOAT:
  case token::INT:
  case token::BOOL:
  case token::VEC2:
  case token::VEC3:
  case token::VEC4:
  case token::BVEC2:
  case token::BVEC3:
  case token::BVEC4:
  case token::IVEC2:
  case token::IVEC3:
  case token::IVEC4:
  case token::MAT2:
  case token::MAT3:
  case token::MAT4:
  case token::SAMPLER2D:
  case token::SAMPLERCUBE:
  case token::TYPE_NAME: {
    parse_node N = parse_node(Token, parse_node::TYPE_SPECIFIER);
    Match(Token.Type);
    return N;
  }

  case token::STRUCT:
    return ParseStructSpecifier();
  }

  GenError("expected type specifier before token " + TokenToString(Token),
           Token);
  Match(Token.Type);
  return parse_node();
}

parse_node parser::ParseInitializer() { return ParseAssignmentExpression(); }

parse_node parser::ParseConstantExpression() {
  return ParseConditionalExpression();
}

parse_node parser::ParseExpression() {
  parse_node N = parse_node::EXPRESSION;
  N.Children.push_back(ParseAssignmentExpression());
  if (Token.Type == token::COMMA) {
    N.Children.push_back(parse_node(Token));
    Match(token::COMMA);
    N.Children.push_back(ParseExpression());
  }
  return N;
}

parse_node parser::ParseExpressionStatement() {
  parse_node N;
  if (Token.Type != token::SEMICOLON) {
    N.Children.push_back(ParseExpression());
  }
  N.Children.push_back(parse_node(Token));
  Match(token::SEMICOLON);
  return N;
}

parse_node parser::ParseStatementWithScope() {
  if (Token.Type == token::LEFT_BRACE) {
    return ParseCompoundStatementNoNewScope();
  }
  return ParseSimpleStatement();
}

parse_node parser::ParseDeclarationStatement() { return ParseDeclaration(); }

parse_node parser::ParseCondition() {
  if (IsTypeQualifier(Token.Type) || IsTypeSpecifier(Token.Type) ||
      IsPrecisionQualifier(Token.Type)) {
    parse_node N;
    N.Children.push_back(ParseFullySpecifiedType());
    N.Children.push_back(parse_node(Token));
    Match(token::IDENTIFIER);
    N.Children.push_back(parse_node(Token));
    Match(token::EQUAL);
    N.Children.push_back(ParseInitializer());
    return N;
  }
  return ParseExpression();
}

parse_node parser::ParseIterationStatement() {
  parse_node N;
  if (Token.Type == token::WHILE) {
    N.Children.push_back(parse_node(Token));
    Match(token::WHILE);
    N.Children.push_back(parse_node(Token));
    Match(token::LEFT_PAREN);
    N.Children.push_back(ParseCondition());
    N.Children.push_back(parse_node(Token));
    Match(token::RIGHT_PAREN);
    N.Children.push_back(ParseStatementNoNewScope());
  } else if (Token.Type == token::DO) {
    N.Children.push_back(parse_node(Token));
    Match(token::DO);
    N.Children.push_back(ParseStatementWithScope());
    N.Children.push_back(parse_node(Token));
    Match(token::WHILE);
    N.Children.push_back(parse_node(Token));
    Match(token::LEFT_PAREN);
    N.Children.push_back(ParseExpression());
    N.Children.push_back(parse_node(Token));
    Match(token::RIGHT_PAREN);
    N.Children.push_back(parse_node(Token));
    Match(token::SEMICOLON);
  } else if (Token.Type == token::FOR) {
    N.Children.push_back(parse_node(Token));
    Match(token::FOR);
    N.Children.push_back(parse_node(Token));
    Match(token::LEFT_PAREN);

    if (IsTypeQualifier(Token.Type) || IsTypeSpecifier(Token.Type) ||
        Token.Type == token::PRECISION) {
      N.Children.push_back(ParseDeclarationStatement());
    } else {
      N.Children.push_back(ParseExpressionStatement());
    }

    if (Token.Type != token::SEMICOLON) {
      N.Children.push_back(ParseCondition());
    }
    N.Children.push_back(parse_node(Token));
    Match(token::SEMICOLON);
    if (Token.Type != token::RIGHT_PAREN) {
      N.Children.push_back(ParseExpression());
    }
    N.Children.push_back(parse_node(Token));
    Match(token::RIGHT_PAREN);
    N.Children.push_back(ParseStatementNoNewScope());
  }
  return N;
}

parse_node parser::ParseSelectionStatement() {
  parse_node N;
  N.Children.push_back(parse_node(Token));
  Match(token::IF);
  N.Children.push_back(parse_node(Token));
  Match(token::LEFT_PAREN);
  N.Children.push_back(ParseExpression());
  N.Children.push_back(parse_node(Token));
  Match(token::RIGHT_PAREN);
  N.Children.push_back(ParseStatementWithScope());
  if (Token.Type == token::ELSE) {
    N.Children.push_back(parse_node(Token));
    Match(token::ELSE);
    N.Children.push_back(ParseStatementWithScope());
  }
  return N;
}

parse_node parser::ParseJumpStatement() {
  parse_node N;
  if (!IsJumpToken(Token.Type)) {
    GenError("expected jump token before token " + TokenToString(Token), Token);
    return parse_node();
  }
  int T = Token.Type;
  N.Children.push_back(parse_node(Token));
  Match(T);
  if (T == token::RETURN) {
    N.Children.push_back(ParseExpression());
  }
  N.Children.push_back(parse_node(Token));
  Match(token::SEMICOLON);
  return N;
}

parse_node parser::ParseSimpleStatement() {
  if (Token.Type == token::IF) {
    return ParseSelectionStatement();
  } else if (IsIterationToken(Token.Type)) {
    return ParseIterationStatement();
  } else if (IsJumpToken(Token.Type)) {
    return ParseJumpStatement();
  } else if (IsTypeQualifier(Token.Type) || IsTypeSpecifier(Token.Type) ||
             Token.Type == token::PRECISION) {
    return ParseDeclarationStatement();
  }
  return ParseExpressionStatement();
}

parse_node parser::ParseCompoundStatementWithScope() {
  parse_node N;
  N.Children.push_back(Token);
  Match(token::LEFT_BRACE);
  if (Token.Type != token::RIGHT_BRACE) {
    N.Children.push_back(ParseStatementList());
  }
  N.Children.push_back(Token);
  Match(token::RIGHT_BRACE);
  return N;
}

parse_node parser::ParseStatementNoNewScope() {
  if (Token.Type == token::LEFT_BRACE) {
    return ParseCompoundStatementWithScope();
  }
  return ParseSimpleStatement();
}

parse_node parser::ParseStatementList() {
  parse_node N;
  while (Token.Type != token::RIGHT_BRACE) {
    N.Children.push_back(ParseStatementNoNewScope());
  }
  return N;
}

parse_node parser::ParseCompoundStatementNoNewScope() {
  parse_node N;
  N.Children.push_back(Token);
  Match(token::LEFT_BRACE);
  if (Token.Type != token::RIGHT_BRACE) {
    N.Children.push_back(ParseStatementList());
  } else {
    N.Children.push_back(parse_node());
  }
  N.Children.push_back(Token);
  Match(token::RIGHT_BRACE);
  return N;
}

parse_node parser::ParseFunctionDefinition() {
  parse_node N = parse_node(parse_node::FUNCTION_DEFINITION);
  N.Append(ParseFunctionPrototype());
  N.Append(ParseCompoundStatementNoNewScope());
  return N;
}

parse_node parser::ParseSingleDeclaration() {
  parse_node N = parse_node();
  if (Token.Type == token::INVARIANT) {
    N.Children.push_back(parse_node(Token));
    Match(token::INVARIANT);
    N.Children.push_back(parse_node(Token));
    Match(token::IDENTIFIER);
    return N;
  }
  if (!(IsTypeSpecifier(Token.Type) || IsTypeQualifier(Token.Type) ||
        IsPrecisionQualifier(Token.Type))) {
    GenError("epected type specifier before token " + TokenToString(Token),
             Token);
    while (Token.Type != token::SEMICOLON && Token.Type != token::END) {
      Match(Token.Type);
    }
    return parse_node();
  }
  N.Append(ParseFullySpecifiedType());
  if (Token.Type != token::IDENTIFIER) {
    return N;
  }
  N.Children.push_back(parse_node(Token));
  Match(token::IDENTIFIER);
  if (Token.Type == token::LEFT_BRACKET) {
    N.Children.push_back(parse_node(Token));
    Match(token::LEFT_BRACKET);
    N.Children.push_back(ParseConstantExpression());
    N.Children.push_back(parse_node(Token));
    Match(token::RIGHT_BRACKET);
  } else if (Token.Type == token::EQUAL) {
    parse_node NSub;
    NSub.Children.push_back(parse_node(Token));
    Match(token::EQUAL);
    NSub.Children.push_back(ParseInitializer());
    N.Children.push_back(NSub);
  }
  return N;
}

parse_node parser::ParseInitDeclaratorList() {
  parse_node N;
  N.Children.push_back(ParseSingleDeclaration());
  if (Token.Type != token::COMMA)
    return N;
  while (Token.Type == token::COMMA) {
    N.Children.push_back(parse_node(Token));
    Match(token::COMMA);
    N.Children.push_back(parse_node(Token));
    Match(token::IDENTIFIER);
    if (Token.Type == token::LEFT_BRACKET) {
      N.Children.push_back(parse_node(Token));
      Match(token::LEFT_BRACKET);
      N.Children.push_back(ParseConstantExpression());
      N.Children.push_back(parse_node(Token));
      Match(token::RIGHT_BRACKET);
    } else if (Token.Type == token::EQUAL) {
      N.Children.push_back(parse_node(Token));
      Match(token::EQUAL);
      N.Children.push_back(ParseInitializer());
    }
  }
  return N;
}

parse_node parser::ParseDeclaration() {
  if (Token.Type == token::PRECISION) {
    parse_node N;
    N.Children.push_back(parse_node(Token));
    Match(token::PRECISION);
    N.Children.push_back(ParsePrecisionQualifier());
    N.Children.push_back(ParseTypeSpecifierNoPrecision());
    N.Children.push_back(parse_node(Token));
    Match(token::SEMICOLON);
    return N;
  }
  PushState();
  DisableErrors();
  ParseSingleDeclaration();
  if (Token.Type == token::LEFT_PAREN) {
    PopState();
    EnableErrors();
    parse_node F = ParseFunctionPrototype();
    if (Token.Type != token::SEMICOLON) {
      GenError("expected function body after function declarator", Token);
    }
    return F;
  }

  PopState();
  EnableErrors();
  parse_node N = parse_node(parse_node::DECLARATION);
  N.Append(ParseInitDeclaratorList());
  N.Children.push_back(parse_node(Token));
  Match(token::SEMICOLON);
  return N;
}

parse_node parser::ParseExternalDeclaration() {
  PushState();
  DisableErrors();
  ParseSingleDeclaration();
  if (Token.Type == token::LEFT_PAREN) {
    RestoreState();
    ParseFunctionPrototype();
    if (Token.Type == token::LEFT_BRACE) {
      PopState();
      EnableErrors();
      return ParseFunctionDefinition();
    }
  }
  PopState();
  EnableErrors();
  return ParseDeclaration();
}

parse_node parser::ParseTranslationUnit() {
  Token = Lex.GetToken();
  parse_node N;
  while (Token.Type != token::END) {
    N.Children.push_back(ParseExternalDeclaration());
  }

  return N;
}

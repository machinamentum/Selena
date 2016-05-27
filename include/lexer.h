
#ifndef LEXER_H
#define LEXER_H

#include "symbol.h"

#include <sstream>
namespace std {
template <typename T> string to_string(T Value) {
  stringstream ss;
  ss << Value;
  return ss.str();
}
};

struct token {

  enum {
    LEFT_PAREN = '(',
    RIGHT_PAREN = ')',
    LEFT_BRACKET = '[',
    RIGHT_BRACKET = ']',
    LEFT_BRACE = '{',
    RIGHT_BRACE = '}',
    DOT = '.',
    COMMA = ',',
    COLON = ':',
    EQUAL = '=',
    SEMICOLON = ';',
    BANG = '!',
    DASH = '-',
    TILDE = '~',
    PLUS = '+',
    STAR = '*',
    SLASH = '/',
    PERCENT = '%',
    LEFT_ANGLE = '<',
    RIGHT_ANGLE = '>',
    VERTICLE_BAR = '|',
    CARET = '^',
    AMPERSAND = '&',
    QUESTION = '?',

    END = 255,

    ATTRIBUTE,
    CONST,
    BOOL,
    FLOAT,
    INT,
    BREAK,
    CONTINUE,
    DO,
    ELSE,
    FOR,
    IF,
    DISCARD,
    RETURN,
    BVEC2,
    BVEC3,
    BVEC4,
    IVEC2,
    IVEC3,
    IVEC4,
    VEC2,
    VEC3,
    VEC4,
    MAT2,
    MAT3,
    MAT4,
    IN,
    OUT,
    INOUT,
    UNIFORM,
    VARYING,
    SAMPLER2D,
    SAMPLERCUBE,
    STRUCT,
    VOID,
    WHILE,

    IDENTIFIER,
    TYPE_NAME,
    FLOATCONSTANT,
    INTCONSTANT,
    BOOLCONSTANT,
    FIELD_SELECTION,
    LEFT_OP,
    RIGHT_OP,
    INC_OP,
    DEC_OP,
    LE_OP,
    GE_OP,
    EQ_OP,
    NE_OP,
    AND_OP,
    OR_OP,
    XOR_OP,
    MUL_ASSIGN,
    DIV_ASSIGN,
    ADD_ASSIGN,
    MOD_ASSIGN,
    LEFT_ASSIGN,
    RIGHT_ASSIGN,
    AND_ASSIGN,
    XOR_ASSIGN,
    OR_ASSIGN,
    SUB_ASSIGN,

    INVARIANT,
    HIGH_PRECISION,
    MEDIUM_PRECISION,
    LOW_PRECISION,
    PRECISION,

    DQSTRING,
    SQSTRING,

    // reserved
    ASM,
    INLINE,
  };

  int Type;
  int BoolValue;
  long IntValue;
  double FloatValue;
  std::string Id;
  int Line;
  int Offset;

  friend std::string TokenToString(const token &T) {
    if (T.Type < END)
      return std::string(1, (char)T.Type);
    if (T.Type >= ATTRIBUTE && T.Type <= FLOATCONSTANT)
      return T.Id;
    if (T.Type == FLOATCONSTANT)
      return std::to_string(T.FloatValue);
    if (T.Type == INTCONSTANT)
      return std::to_string(T.IntValue);
    if (T.Type == BOOLCONSTANT)
      return T.BoolValue ? "true" : "flase";
    if (T.Type == FIELD_SELECTION)
      return T.Id;
    if (T.Type >= INVARIANT && T.Type <= PRECISION)
      return T.Id;
    if (T.Type == DQSTRING)
      return "\"" + T.Id + "\"";
    if (T.Type == SQSTRING)
      return "\'" + T.Id + "\'";
    if (T.Type >= ASM)
      return T.Id;
    switch (T.Type) {
    case LEFT_OP:
      return "<<";
    case RIGHT_OP:
      return ">>";
    case INC_OP:
      return "++";
    case DEC_OP:
      return "--";
    case LE_OP:
      return "<=";
    case GE_OP:
      return ">=";
    case EQ_OP:
      return "==";
    case NE_OP:
      return "!=";
    case AND_OP:
      return "&&";
    case OR_OP:
      return "||";
    case XOR_OP:
      return "^^";
    case MUL_ASSIGN:
      return "*=";
    case DIV_ASSIGN:
      return "/=";
    case ADD_ASSIGN:
      return "+=";
    case MOD_ASSIGN:
      return "%%=";
    case LEFT_ASSIGN:
      return "<<=";
    case RIGHT_ASSIGN:
      return ">>=";
    case AND_ASSIGN:
      return "&=";
    case XOR_ASSIGN:
      return "^=";
    case OR_ASSIGN:
      return "|=";
    case SUB_ASSIGN:
      return "-=";
    }
    return "";
  }

  friend std::string TokenToString(const int &Type) {
    symtable S;
    if (Type < END)
      return std::string(1, (char)Type);
    if (Type == IDENTIFIER)
      return "identifier";
    if (Type == TYPE_NAME)
      return "type name";
    if (Type >= ATTRIBUTE && Type < FLOATCONSTANT)
      return S.FindFirstOfType(Type)->Name;
    if (Type == FLOATCONSTANT)
      return "float value";
    if (Type == INTCONSTANT)
      return "int value";
    if (Type == BOOLCONSTANT)
      return "bool value";
    if (Type == FIELD_SELECTION)
      return "field selector";
    if (Type >= INVARIANT && Type <= PRECISION)
      return S.FindFirstOfType(Type)->Name;
    if (Type == DQSTRING)
      return "constant string";
    if (Type == SQSTRING)
      return "constant string";
    if (Type >= ASM)
      return S.FindFirstOfType(Type)->Name;
    switch (Type) {
    case LEFT_OP:
      return "<<";
    case RIGHT_OP:
      return ">>";
    case INC_OP:
      return "++";
    case DEC_OP:
      return "--";
    case LE_OP:
      return "<=";
    case GE_OP:
      return ">=";
    case EQ_OP:
      return "==";
    case NE_OP:
      return "!=";
    case AND_OP:
      return "&&";
    case OR_OP:
      return "||";
    case XOR_OP:
      return "^^";
    case MUL_ASSIGN:
      return "*=";
    case DIV_ASSIGN:
      return "/=";
    case ADD_ASSIGN:
      return "+=";
    case MOD_ASSIGN:
      return "%%=";
    case LEFT_ASSIGN:
      return "<<=";
    case RIGHT_ASSIGN:
      return ">>=";
    case AND_ASSIGN:
      return "&=";
    case XOR_ASSIGN:
      return "^=";
    case OR_ASSIGN:
      return "|=";
    case SUB_ASSIGN:
      return "-=";
    }
    return "";
  }
};

std::string TokenToString(const int &Type);

struct lexer_state {
  char *SourcePtr;
  char *EndPtr;
  char *CurrentPtr;
  int LineCurrent;
  int OffsetCurrent;
  symtable *Table;

  token GetToken();
  token PeekToken();
};

void LexerInit(lexer_state *State, char *Source, char *End, symtable *T);
token LexerPeekToken(lexer_state *State);
token LexerGetToken(lexer_state *State);
std::string LexerGetLine(lexer_state *State, int Line);

#endif

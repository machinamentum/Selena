
#ifndef LEXER_H
#define LEXER_H

#include "symbol.h"

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
};

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

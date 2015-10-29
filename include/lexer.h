
#ifndef LEXER_H
#define LEXER_H

#include <string>

struct token {

  enum {
    END = 255,
    BOOL,
    INT,
    FLOAT,
    DQSTRING,
    SQSTRING,
    IDENTIFIER,
    EQEQ,
    PLUSPLUS,
    MINUSMINUS,
    PLUSEQ,
    MINUSEQ,
    SHIFTLEFT,
    LESSEQ,
    CPPSTRING,
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
};

void LexerInit(lexer_state *State, char *Source, char *End);
token LexerGetToken(lexer_state *State);

#endif

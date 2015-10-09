#include "lexer.h"
#include <cstdlib>

void LexerInit(lexer_state *State, char *Source, char *End) {
  State->SourcePtr = State->CurrentPtr = Source;
  State->EndPtr = End - 1;
  State->LineCurrent = 1;
  State->OffsetCurrent = 0;
}

token LexerGetToken(lexer_state *State) {
  token ReturnToken;

  auto IsWhiteSpace = [](char C) {
    return (C == ' ') || (C == '\n') || (C == '\t') || (C == '\r') ||
           (C == '\f');
  };

  char *Current = State->CurrentPtr;
  if (Current >= State->EndPtr)
    return {token::END};

  while (IsWhiteSpace(Current[0]) && (Current < State->EndPtr)) {
    ++State->OffsetCurrent;
    if (Current[0] == '\n') {
      ++State->LineCurrent;
      State->OffsetCurrent = 0;
    }
    ++Current;
  }

  auto IsAsciiLetter = [](char C) {
    return ((C >= 'A') && (C <= 'Z')) || ((C >= 'a') && (C <= 'z'));
  };

  if (IsAsciiLetter(Current[0])) {
    auto IsAsciiLetterOrNumber = [](char C) {
      return ((C >= '0') && (C <= '9')) || ((C >= 'A') && (C <= 'Z')) ||
             ((C >= 'a') && (C <= 'z'));
    };
    char *End = Current + 1;
    while (IsAsciiLetterOrNumber(*End) && (End < State->EndPtr)) {
      ++End;
    }
    ReturnToken.Id = std::string(Current, End - Current);
    ReturnToken.Type = token::IDENTIFIER;
    ReturnToken.Line = State->LineCurrent;
    ReturnToken.Offset = State->OffsetCurrent;
    State->OffsetCurrent += End - Current;
    Current = End;
    goto _Exit;
  }

  static auto IsNumberOrDot =
      [](char C) { return ((C >= '0') && (C <= '9')) || (C == '.'); };

  if (IsNumberOrDot(Current[0])) {
    char *End;
    ReturnToken.Type = token::FLOAT;
    ReturnToken.FloatValue = strtod(Current, &End);
    ReturnToken.Line = State->LineCurrent;
    ReturnToken.Offset = State->OffsetCurrent;
    State->OffsetCurrent += End - Current;
    Current = End;
    goto _Exit;
  }

  switch (Current[0]) {
  case '<': {
    if (Current < State->EndPtr) {
      if (Current[1] == '<') {
        ReturnToken.Type = token::SHIFTLEFT;
        ++State->OffsetCurrent;
      } else if (Current[1] == '=') {
        ReturnToken.Type = token::LESSEQ;
        ++State->OffsetCurrent;
      }
    }
    goto _BuildToken;
  }

  default:
  _BuildToken:
    ReturnToken.Line = State->LineCurrent;
    ReturnToken.Offset = State->OffsetCurrent;
    ++State->OffsetCurrent;
    ReturnToken.Type = Current[0];
  }

  ++Current;

_Exit:
  State->CurrentPtr = Current;
  return ReturnToken;
}

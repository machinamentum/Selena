#include "lexer.h"
#include <cstdlib>
#include <cstring>

token lexer_state::GetToken() { return LexerGetToken(this); }

token lexer_state::PeekToken() { return LexerPeekToken(this); }

void LexerInit(lexer_state *State, char *Source, char *End, symtable *T) {
  State->SourcePtr = State->CurrentPtr = Source;
  State->EndPtr = End - 1;
  State->LineCurrent = 1;
  State->OffsetCurrent = 0;
  State->Table = T;
}

token LexerPeekToken(lexer_state *State) {
  lexer_state TempState = *State;
  return LexerGetToken(&TempState);
}

std::string LexerGetLine(lexer_state *State, int Line) {
  char *Current = State->SourcePtr;
  int CurLine = 0;
  while (CurLine < (Line - 1) && (Current < State->EndPtr)) {
    if (Current[0] == '\n') {
      ++CurLine;
    }
    ++Current;
  }
  char *End = Current;
  while (*End != '\n' && End < State->EndPtr) {
    ++End;
  }

  return std::string(Current, End - Current);
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
_CheckWhiteSpace:
  while (IsWhiteSpace(Current[0]) && (Current < State->EndPtr)) {
    ++State->OffsetCurrent;
    if (Current[0] == '\n') {
      ++State->LineCurrent;
      State->OffsetCurrent = 0;
    }
    ++Current;
  }

  if (Current[0] == '/' && (Current < State->EndPtr)) {
    if (Current[1] == '/') {
      while (*Current != '\n' && (Current < State->EndPtr)) {
        ++Current;
      }
      goto _CheckWhiteSpace;
    }
  }
  // TODO preprocessor
  // if (Current[0] == '#' && (Current < State->EndPtr)) {
  //   char *End = Current;
  //   while (*End != '\n' && (End < State->EndPtr)) {
  //     ++End;
  //   }
  //   ReturnToken.Id = std::string(Current, End - Current);
  //   ReturnToken.Type = token::CPPSTRING;
  //   ReturnToken.Line = State->LineCurrent;
  //   ReturnToken.Offset = State->OffsetCurrent;
  //   State->OffsetCurrent += End - Current;
  //   Current = End;
  //   goto _Exit;
  // }

  static auto IsAsciiLetter = [](char C) {
    return (C == '_') || ((C >= 'A') && (C <= 'Z')) ||
           ((C >= 'a') && (C <= 'z'));
  };

  if (IsAsciiLetter(Current[0])) {
    auto IsAsciiLetterOrNumber = [](char C) {
      return (C == '_') || ((C >= '0') && (C <= '9')) ||
             ((C >= 'A') && (C <= 'Z')) || ((C >= 'a') && (C <= 'z'));
    };
    char *End = Current + 1;
    while (IsAsciiLetterOrNumber(*End) && (End < State->EndPtr)) {
      ++End;
    }
    std::string TheID = std::string(Current, End - Current);
    symtable_entry *Entry = State->Table->Lookup(TheID);
    if (Entry->SymbolType == 0)
      Entry = State->Table->Insert(TheID, token::IDENTIFIER);
    ReturnToken.Id = Entry->Name;
    ReturnToken.Type = Entry->SymbolType;
    ReturnToken.Line = State->LineCurrent;
    ReturnToken.Offset = State->OffsetCurrent;
    if (Entry->SymbolType == token::BOOLCONSTANT) {
      if (strcmp(Entry->Name.c_str(), "true") == 0)
        ReturnToken.BoolValue = 1;
      if (strcmp(Entry->Name.c_str(), "false") == 0)
        ReturnToken.BoolValue = 0;
    }
    State->OffsetCurrent += End - Current;
    Current = End;
    goto _Exit;
  }

  static auto IsNumberOrDot = [](char C) {
    return ((C >= '0') && (C <= '9')) || (C == '.');
  };

  if (IsNumberOrDot(Current[0])) {
    bool IsFloat = false;
    char *End = Current;
    while (IsNumberOrDot(*End) && (End < State->EndPtr)) {
      if (*End == '.') {
        IsFloat = true;
        break;
      }
      ++End;
    }
    if (IsFloat) {
      ReturnToken.Type = token::FLOATCONSTANT;
      ReturnToken.FloatValue = strtod(Current, &End);
    } else {
      ReturnToken.Type = token::INTCONSTANT;
      ReturnToken.IntValue = strtoul(Current, &End, 10);
    }
    ReturnToken.Line = State->LineCurrent;
    ReturnToken.Offset = State->OffsetCurrent;
    State->OffsetCurrent += End - Current;
    Current = End;
    goto _Exit;
  }

  static auto GetEsacpedChar = [](char Char) {
    switch (Char) {
    case 't':
      return '\t';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 'f':
      return '\f';
    case '"':
      return '\"';
    case '\'':
      return '\'';
    case '\\':
      return '\\';
    }

    return Char;
  };
  if (Current[0] == '\"') {
    ReturnToken.Type = token::DQSTRING;
    char *End = Current + 1;
    while ((*End != '\"') && (End < State->EndPtr)) {
      switch (*End) {
      case '\\':
        if (End + 1 < State->EndPtr) {
          ReturnToken.Id += GetEsacpedChar(*(End + 1));
          End += 2;
        }
        break;

      default:
        ReturnToken.Id += *End;
        ++End;
        break;
      }
    }

    State->OffsetCurrent += End - Current;
    Current = End + 1;
    ReturnToken.Line = State->LineCurrent;
    ReturnToken.Offset = State->OffsetCurrent;
    ++State->OffsetCurrent;
    goto _Exit;
  }

  if (Current[0] == '\'') {
    ReturnToken.Type = token::SQSTRING;

    char *End = Current + 1;
    while ((*End != '\'') && (End < State->EndPtr)) {
      switch (*End) {
      case '\\':
        if (End + 1 < State->EndPtr) {
          ReturnToken.Id += GetEsacpedChar(*(End + 1));
          End += 2;
        }
        break;

      default:
        ReturnToken.Id += *End;
        ++End;
        break;
      }
    }

    State->OffsetCurrent += End - Current;
    Current = End + 1;
    ReturnToken.Line = State->LineCurrent;
    ReturnToken.Offset = State->OffsetCurrent;
    ++State->OffsetCurrent;
    goto _Exit;
  }

  switch (Current[0]) {
  case '<': {
    if (Current < State->EndPtr) {
      if (Current[1] == '<') {
        ReturnToken.Type = token::LEFT_ASSIGN;
        ++State->OffsetCurrent;
      } else if (Current[1] == '=') {
        ReturnToken.Type = token::LE_OP;
        ++State->OffsetCurrent;
      }
    }
    goto _BuildToken;
  }

  case '|': {
    if (Current < State->EndPtr) {
      if (Current[1] == '|') {
        ReturnToken.Type = token::OR_OP;
        ++State->OffsetCurrent;
        ++Current;
      } else if (Current[1] == '=') {
        ReturnToken.Type = token::OR_ASSIGN;
        ++State->OffsetCurrent;
      }
      goto _BuildToken;
    }
  }

  default:
    ReturnToken.Type = Current[0];
  _BuildToken:
    ReturnToken.Line = State->LineCurrent;
    ReturnToken.Offset = State->OffsetCurrent;
    ++State->OffsetCurrent;
  }

  ++Current;

_Exit:
  State->CurrentPtr = Current;
  return ReturnToken;
}


#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "lexer.h"

struct parse_node {

  enum { E, T };

  std::vector<parse_node> Children;
  token Token;
  int Type;
};

parse_node ParserGetNode(lexer_state *State, int StatementEnd);
parse_node ParserGetCurls(lexer_state *State);
parse_node ParserGetParens(lexer_state *State);

#endif

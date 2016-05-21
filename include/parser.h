
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <vector>

struct parse_node {

  enum { E, T };

  std::vector<parse_node> Children;
  token Token;
  int Type;
};

struct parser {
  lexer_state &Lex;
  symtable *SymbolTable;

  parser(lexer_state &L);
  parse_node Parse();
};


#endif

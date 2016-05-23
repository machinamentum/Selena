
#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "parser.h"

struct cpp_macro {
  std::string Id;
  parse_node Expansion;
};

struct cpp_table {
  std::vector<cpp_macro> Macros;
};

void CppDefineInt(cpp_table *Table, std::string Id, int Value);
void CppResolveMacros(cpp_table *Table, parse_node *PTree);

#endif

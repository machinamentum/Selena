
#include "preprocessor.h"

void CppDefineInt(cpp_table *Table, std::string Id, int Value) {
  cpp_macro Macro;
  Macro.Id = Id;
  parse_node IntNode;
  IntNode.Type = parse_node::T;
  IntNode.Token.Type = token::INT;
  IntNode.Token.IntValue = Value;
  Macro.Expansion = IntNode;
  Table->Macros.push_back(Macro);
}

void CppResolveMacros(cpp_table *Table, parse_node *PTree) {
  if (PTree->Type == parse_node::E) {
    for (parse_node &PN : PTree->Children) {
      CppResolveMacros(Table, &PN);
    }
  } else {
    if (PTree->Token.Type == token::IDENTIFIER) {
      for (cpp_macro &Macro : Table->Macros) {
        if (PTree->Token.Id.compare(Macro.Id) == 0) {
          *PTree = Macro.Expansion;
        } else if (PTree->Token.Id.compare("__LINE__") == 0) {
          parse_node IntNode;
          IntNode.Type = parse_node::T;
          IntNode.Token.Type = token::INT;
          IntNode.Token.IntValue = PTree->Token.Line;
          IntNode.Token.Line = PTree->Token.Line;
          *PTree = IntNode;
        }
      }
    }
  }
}


#ifndef SYMBOL_H
#define SYMBOL_H

#include <string>
#include <vector>

struct symtable_entry {
  std::string Name;
  int SymbolType;
  int TypeSpecifier;
  int Qualifier;
  int Definition;
};

struct symtable {
  std::vector<symtable_entry> symbols;
  std::vector<symtable> StackedTables;

  void OpenScope();
  void CloseScope();
  int GetIndex(std::string Name);
  symtable_entry *Insert(std::string Name, int Type);
  symtable_entry *Lookup(std::string Name);
  symtable_entry *FindFirstOfType(int T);

  symtable();
};

#endif

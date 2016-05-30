#include "symbol.h"
#include "lexer.h"

int symtable::GetIndex(std::string Name) {
  for (int i = 0; i < symbols.size(); ++i) {
    if (symbols[i].Name.compare(Name) == 0) {
      return i;
    }
  }

  return 0;
}

symtable_entry *symtable::Insert(std::string Name, int Type) {
  if (GetIndex(Name) == 0) {
    symbols.push_back((symtable_entry){Name, Type});
  }

  return Lookup(Name);
}

symtable_entry *symtable::Lookup(std::string Name) {
  return &symbols[GetIndex(Name)];
}

symtable_entry *symtable::FindFirstOfType(int T) {
  for (int i = 0; i < symbols.size(); ++i) {
    if (symbols[i].SymbolType == T) {
      return &symbols[i];
    }
  }

  return &symbols[0];
}

void symtable::OpenScope() {
  StackedTables.push_back(*this);
}

void symtable::CloseScope() {
  *this = StackedTables.back();
  StackedTables.pop_back();
}

symtable::symtable() {
  symbols.reserve(256);
  Insert("", 0);
  Insert("attribute", token::ATTRIBUTE);
  Insert("const", token::CONST);
  Insert("bool", token::BOOL);
  Insert("float", token::FLOAT);
  Insert("int", token::INT);
  Insert("break", token::BREAK);
  Insert("continue", token::CONTINUE);
  Insert("do", token::DO);
  Insert("else", token::ELSE);
  Insert("for", token::FOR);
  Insert("if", token::IF);
  Insert("discard", token::DISCARD);
  Insert("return", token::RETURN);
  Insert("bvec2", token::BVEC2);
  Insert("bvec3", token::BVEC3);
  Insert("bvec4", token::BVEC4);
  Insert("ivec2", token::IVEC2);
  Insert("ivec3", token::IVEC3);
  Insert("ivec4", token::IVEC4);
  Insert("vec2", token::VEC2);
  Insert("vec3", token::VEC3);
  Insert("vec4", token::VEC4);
  Insert("mat2", token::MAT2);
  Insert("mat3", token::MAT3);
  Insert("mat4", token::MAT4);
  Insert("in", token::IN);
  Insert("out", token::OUT);
  Insert("inout", token::INOUT);
  Insert("uniform", token::UNIFORM);
  Insert("varying", token::VARYING);
  Insert("sampler2D", token::SAMPLER2D);
  Insert("samplerCube", token::SAMPLERCUBE);
  Insert("struct", token::STRUCT);
  Insert("void", token::VOID);
  Insert("while", token::WHILE);
  Insert("invariant", token::INVARIANT);
  Insert("highp", token::HIGH_PRECISION);
  Insert("mediump", token::MEDIUM_PRECISION);
  Insert("lowp", token::LOW_PRECISION);
  Insert("precision", token::PRECISION);

  Insert("true", token::BOOLCONSTANT);
  Insert("false", token::BOOLCONSTANT);

  // TODO reserved keywords
  Insert("asm", token::ASM);
  Insert("inline", token::INLINE);
}

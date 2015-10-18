
#ifndef CODEGEN_NEO_H
#define CODEGEN_NEO_H

#include "ast.h"
#include <string>
#include <vector>
#include <ostream>

struct neocode_instruction {

  enum { INLINE };

  int Type;
  int DstRegister;
  int Src1Register;
  int Src2Register;
  int SwizzleType;
  std::string ExtraData;
};

struct neocode_variable {
  int Type;
  std::string TypeName;
  int Register;
};

struct neocode_function {
  int ReturnType;
  std::string Name;
  std::vector<neocode_variable> Variables;
  std::vector<neocode_instruction> Instructions;
};

struct neocode_constant {

  enum {
    FLOAT,
    INT,
    BOOLEAN,
  };

  int Type;
  int Register;

  struct Integer {
    int X, Y, Z, W;
  };

  struct Float {
    float X, Y, Z, W;
  };

  int Bool;
};

struct neocode_program {
  std::vector<neocode_constant> Constants;
  std::vector<neocode_function> Functions;
};

neocode_program CGNeoBuildProgramInstance(ast_node *ASTNode);
void CGNeoGenerateCode(neocode_program *Program, std::ostream &os);

#endif

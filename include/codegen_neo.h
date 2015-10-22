
#ifndef CODEGEN_NEO_H
#define CODEGEN_NEO_H

#include "ast.h"
#include <string>
#include <vector>
#include <ostream>

struct neocode_register_file {
  int Vertex[8];
  int Constants[96];
  int Temp[16];
  int Output[8];

  int AllocOutput() {
    for (int i = 0; i < 8; ++i) {
      if (!Output[i]) {
        Output[i] = 1;
        return i;
      }
    }

    return -1;
  }

  int AllocVertex() {
    for (int i = 0; i < 8; ++i) {
      if (!Vertex[i]) {
        Vertex[i] = 1;
        return i;
      }
    }

    return -1;
  }

  int AllocTemp() {
    for (int i = 0; i < 16; ++i) {
      if (!Temp[i]) {
        Temp[i] = 1;
        return i + 0x10;
      }
    }

    return -1;
  }

  int AllocConstant() {
    for (int i = 0; i < 96; ++i) {
      if (!Constants[i]) {
        Constants[i] = 1;
        return i + 0x20;
      }
    }

    return -1;
  }

  void Free(int Register) {
    if (Register < 0x8)
      Vertex[Register] = 0;

    if (Register >= 0x10 && Register < 0x20)
      Temp[Register - 0x10] = 0;

    if (Register >= 0x20)
      Constants[Register - 0x20] = 0;
  }
};

struct neocode_variable {

  enum {
    OUTPUT_POSITION = 1,
    OUTPUT_QUATERNION,
    OUTPUT_COLOR,
    OUTPUT_TEXCOORD0,
    OUTPUT_TEXCOORD1,
    OUTPUT_TEXCOORD2,
    OUTPUT_VIEW
  };

  std::string Name;
  std::string TypeName;
  int Type;
  int Register;
  int RegisterType;
};

struct neocode_instruction {

  enum { INLINE, MOV, MUL };

  int Type;
  neocode_variable Dst;
  neocode_variable Src1;
  neocode_variable Src2;
  int Swizzle;
  std::string ExtraData;
};

struct neocode_program;

struct neocode_function {
  neocode_program *Program;
  int ReturnType;
  std::string Name;
  std::vector<neocode_variable> Variables;
  std::vector<neocode_instruction> Instructions;

  neocode_function(neocode_program *P) : Program(P) {}
  neocode_variable *GetVariable(std::string Name);
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
  std::vector<neocode_variable> Globals;
  neocode_register_file Registers;
};

neocode_program CGNeoBuildProgramInstance(ast_node *ASTNode);
void CGNeoGenerateCode(neocode_program *Program, std::ostream &os);

#endif

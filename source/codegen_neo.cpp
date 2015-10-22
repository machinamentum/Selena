
#include "codegen_neo.h"

#ifdef _3DS
#include <sstream>
namespace std {
template <typename T> string to_string(T Value) {
  stringstream ss;
  ss << Value;
  return ss.str();
}
};
#endif

static std::string RegisterName(neocode_variable &Var, int UseRaw = 0) {
  if (Var.Name.size() && !UseRaw)
    return Var.Name;
  if (Var.RegisterType == 0) {
    int Register = Var.Register;
    if (Register < 0x10)
      return std::string("v") + std::to_string(Register);
    if (Register < 0x20)
      return std::string("r") + std::to_string(Register - 0x10);

    return std::string("c") + std::to_string(Register - 0x20);
  } else {
    int Register = Var.Register;
    if (Register < 0x10)
      return std::string("o") + std::to_string(Register);
    if (Register < 0x20)
      return std::string("r") + std::to_string(Register - 0x10);

    return "URV";
  }
}

neocode_variable *neocode_function::GetVariable(std::string Name) {
  for (neocode_variable &V : Variables) {
    if (V.Name.compare(Name) == 0) {
      return &V;
    }
  }

  for (neocode_variable &V : Program->Globals) {
    if (V.Name.compare(Name) == 0) {
      return &V;
    }
  }
}

neocode_instruction CGNeoBuildInstruction(neocode_function *Function,
                                          ast_node *ASTNode) {
  neocode_variable DependVar;
  if (ASTNode->Children[0].Children.size()) {
    auto Depend = CGNeoBuildInstruction(Function, &ASTNode->Children[0]);
    DependVar = Depend.Dst;
    Function->Instructions.push_back(Depend);
  }

  auto GetInput = [&Function, &ASTNode, &DependVar]() {
    if (ASTNode->Children[0].Children.size()) {
      return DependVar;
    } else {
      return *Function->GetVariable(ASTNode->Children[0].Id);
    }
  };

  if (ASTNode->Type == ast_node::FUNCTION_CALL) {
    if (ASTNode->Id.compare("asm") == 0) {
      neocode_instruction In;
      In.Type = neocode_instruction::INLINE;
      In.ExtraData = ASTNode->Children[0].Id;
      return In;
    }
  }

  if (ASTNode->Type == ast_node::MULTIPLY) {
    neocode_instruction In;
    In.Type = neocode_instruction::MUL;
    In.Dst = (neocode_variable){"", "", ast_node::STRUCT,
                                Function->Program->Registers.AllocTemp(), 0};
    In.Src1 = GetInput();
    In.Src2 = *Function->GetVariable(ASTNode->Id);
    return In;
  }

  if (ASTNode->Type == ast_node::ASSIGNMENT) {
    if (ASTNode->Children[0].Children.size()) {
      Function->Instructions.back().Dst = *Function->GetVariable(ASTNode->Id);
      return neocode_instruction();
    } else {
      neocode_instruction In;
      In.Type = neocode_instruction::MOV;
      In.Dst = *Function->GetVariable(ASTNode->Id);
      In.Src1 = GetInput();
      return In;
    }
  }

  if (ASTNode->Children[0].Children.size()) {
    Function->Program->Registers.Free(DependVar.Register);
  }
}

void CGNeoBuildStatement(neocode_function *Function, ast_node *ASTNode) {
  Function->Instructions.push_back(CGNeoBuildInstruction(Function, ASTNode));
}

neocode_function CGNeoBuildFunction(neocode_program *Program,
                                    ast_node *ASTNode) {
  neocode_function Function = neocode_function(Program);
  Function.Name = ASTNode->Id;
  for (int i = 1; i < ASTNode->Children.size(); ++i) {
    CGNeoBuildStatement(&Function, &ASTNode->Children[i].Children[0]);
  }
  return Function;
}

neocode_program CGNeoBuildProgramInstance(ast_node *ASTNode) {
  neocode_program Program;
  Program.Globals.push_back((neocode_variable){
      "gl_Position", "vec4", ast_node::STRUCT, Program.Registers.AllocOutput(),
      neocode_variable::OUTPUT_POSITION});
  Program.Globals.push_back((neocode_variable){
      "gl_FrontColor", "vec4", ast_node::STRUCT,
      Program.Registers.AllocOutput(), neocode_variable::OUTPUT_COLOR});
  Program.Globals.push_back(
      (neocode_variable){"gl_Vertex", "vec4", ast_node::STRUCT,
                         Program.Registers.AllocVertex(), 0});
  Program.Globals.push_back(
      (neocode_variable){"gl_Color", "vec4", ast_node::STRUCT,
                         Program.Registers.AllocVertex(), 0});
  Program.Globals.push_back((neocode_variable){
      "gl_ModelViewProjectionMatrix", "vec4", ast_node::STRUCT,
      Program.Registers.AllocConstant(), 0});
  for (ast_node &Node : ASTNode->Children) {
    if (Node.Type == ast_node::FUNCTION)
      Program.Functions.push_back(CGNeoBuildFunction(&Program, &Node));
  }
  return Program;
}

void CGNeoGenerateInstruction(neocode_instruction *Instruction,
                              std::ostream &os) {
  switch (Instruction->Type) {
  case neocode_instruction::EMPTY:
    break;
  case neocode_instruction::INLINE:
    os << " " << Instruction->ExtraData << std::endl;
    break;

  case neocode_instruction::MOV:
    os << " "
       << "mov " << RegisterName(Instruction->Dst) << ", "
       << RegisterName(Instruction->Src1) << std::endl;
    break;

  case neocode_instruction::MUL:
    os << " "
       << "mul " << RegisterName(Instruction->Dst) << ", "
       << RegisterName(Instruction->Src1) << ", "
       << RegisterName(Instruction->Src2) << std::endl;
    break;

  default:
    os << "// Unknown instruction: " << Instruction->Type << std::endl;
    break;
  }
}

void CGNeoGenerateFunction(neocode_function *Function, std::ostream &os) {
  os << Function->Name << ":" << std::endl;
  for (neocode_instruction &Instruction : Function->Instructions) {
    CGNeoGenerateInstruction(&Instruction, os);
  }
  os << Function->Name << "_end:" << std::endl;
}

static std::string OutputName(int Register) {
  const std::string ONames[7] = {
      "position",  "quaternion", "color", "texcoord0",
      "texcoord1", "texcoord2",  "view",
  };

  return ONames[Register - 1];
}

void CGNeoGenerateCode(neocode_program *Program, std::ostream &os) {
  os << ".alias CaelinaCCVersion c95 as (0.0, 0.0, 0.0, 0.1)" << std::endl;
  for (neocode_variable &V : Program->Globals) {
    if (V.RegisterType > 0) {
      os << ".alias " << V.Name << " " << RegisterName(V, 1) << " as "
         << OutputName(V.RegisterType) << std::endl;
    } else {
      os << ".alias " << V.Name << " " << RegisterName(V, 1) << std::endl;
    }
  }
  for (neocode_function &Function : Program->Functions) {
    CGNeoGenerateFunction(&Function, os);
  }
}


#include "codegen_neo.h"

void CGNeoBuildStatement(neocode_function *Function, ast_node *ASTNode) {
  if (ASTNode->Type == ast_node::FUNCTION_CALL) {
    if (ASTNode->Id.compare("asm") == 0) {
      neocode_instruction In;
      In.Type = neocode_instruction::INLINE;
      In.ExtraData = ASTNode->Children[0].Id;
      Function->Instructions.push_back(In);
    }
  }
}

neocode_function CGNeoBuildFunction(ast_node *ASTNode) {
  neocode_function Function;
  Function.Name = ASTNode->Id;
  for (int i = 1; i < ASTNode->Children.size(); ++i) {
    CGNeoBuildStatement(&Function, &ASTNode->Children[i].Children[0]);
  }
  return Function;
}

neocode_program CGNeoBuildProgramInstance(ast_node *ASTNode) {
  neocode_program Program;
  for (ast_node &Node : ASTNode->Children) {
    if (Node.Type == ast_node::FUNCTION)
      Program.Functions.push_back(CGNeoBuildFunction(&Node));
  }
  return Program;
}

void CGNeoGenerateInstruction(neocode_instruction *Instruction,
                              std::ostream &os) {
  switch (Instruction->Type) {
  case neocode_instruction::INLINE:
    os << " " << Instruction->ExtraData << std::endl;
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

void CGNeoGenerateCode(neocode_program *Program, std::ostream &os) {
  os << ".alias CaelinaCCVersion c95 as (0.0, 0.0, 0.0, 0.1)" << std::endl;
  os << ".alias outpos o0 as position" << std::endl;
  os << ".alias outcol o1 as color" << std::endl;
  os << ".alias outtex0 o2.xyzw as texcoord0" << std::endl;
  os << ".alias outtex1 o3.xyzw as texcoord1" << std::endl;
  os << ".alias outtex2 o4.xyzw as texcoord2" << std::endl;
  for (neocode_function &Function : Program->Functions) {
    CGNeoGenerateFunction(&Function, os);
  }
}

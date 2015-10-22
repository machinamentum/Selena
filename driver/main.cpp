#include <fstream>
#include <iostream>
#include "parser.h"
#include "ast.h"
#include "codegen_neo.h"

char *SlurpFile(const char *FilePath, long *FileSize) {
  std::ifstream is = std::ifstream(FilePath);
  is.seekg(0, std::ios::end);
  long Length = is.tellg();
  is.seekg(0, std::ios::beg);
  char *Buffer = new char[Length];
  is.read(Buffer, Length);
  is.close();

  if (FileSize)
    *FileSize = Length;
  return Buffer;
}

void PrintToken(token *Token) {
  switch (Token->Type) {
  case token::FLOAT:
    printf("%f\n", Token->FloatValue);
    break;

  case token::IDENTIFIER:
    printf("%s\n", Token->Id.c_str());
    break;

  case token::END:
    printf("EOF\n");
    break;

  case token::DQSTRING:
    printf("\"%s\"\n", Token->Id.c_str());
    break;

  case token::SQSTRING:
    printf("\"%s\"\n", Token->Id.c_str());
    break;

  default:
    printf("%c\n", Token->Type);
    break;
  }
}

void PrintParseTree(parse_node *Node, int Depth) {
  for (int i = 0; i < Depth; ++i) {
    printf("   ");
  }
  if (Node->Type == parse_node::E) {
    printf("E%d\n", Depth);
    for (int i = 0; i < Node->Children.size(); ++i) {
      PrintParseTree(&Node->Children[i], Depth + 1);
    }
  } else {
    printf("T%d: ", Depth);
    PrintToken(&Node->Token);
  }
}

void PrintAST(ast_node *AST, int Depth);

void PrintASTNode(ast_node &Child, int Depth) {
  switch (Child.Type) {
  case ast_node::FUNCTION:
    printf("function:%s:%d\n", Child.Id.c_str(), Child.Children.size());
    PrintAST(&Child, Depth + 1);
    break;
  case ast_node::FUNCTION_CALL:
    printf("call:%s:%d\n", Child.Id.c_str(), Child.Children.size());
    PrintAST(&Child, Depth + 1);
    break;

  case ast_node::STRUCT:
    printf("struct:%s:%d\n", Child.Id.c_str(), Child.Children.size());
    PrintAST(&Child, Depth + 1);
    break;

  case ast_node::RETURN:
    printf("return\n");
    PrintAST(&Child, Depth + 1);
    break;

  case ast_node::ASSIGNMENT:
    printf("%s =\n", Child.Id.c_str());
    PrintAST(&Child, Depth + 1);
    break;

  case ast_node::MULTIPLY:
    printf("%s *\n", Child.Id.c_str());
    PrintAST(&Child, Depth + 1);
    break;

  case ast_node::VARIABLE:
    switch (Child.VarType) {
    case ast_node::FLOAT:
      printf("float:%s\n", Child.Id.c_str());
      break;

    case ast_node::INT:
      printf("int:%s\n", Child.Id.c_str());
      break;

    case ast_node::FLOAT_LITERAL:
      printf("float:%f\n", Child.FloatValue);
      break;

    case ast_node::INT_LITERAL:
      printf("int:%d\n", Child.IntValue);
      break;

    case ast_node::STRING_LITERAL:
      printf("string:%s\n", Child.Id.c_str());
      break;

    default:
      printf("var:%s\n", Child.Id.c_str());
      break;
    }
    break;

  default:
    printf("Node Type %d, %s\n", Child.Type, Child.Id.c_str());
    PrintAST(&Child, Depth + 1);
    break;
  }
}

void PrintAST(ast_node *AST, int Depth) {
  for (ast_node Type : AST->DefinedTypes) {
    for (int i = 0; i < Depth; ++i) {
      printf("  ");
    }
    printf("N%d ", Depth);
    PrintASTNode(Type, Depth);
  }
  for (ast_node Child : AST->Children) {
    for (int i = 0; i < Depth; ++i) {
      printf("  ");
    }
    printf("D%d ", Depth);
    PrintASTNode(Child, Depth);
  }
}

int main(int argc, char **argv) {
  bool PrintTrees = false;
  char *InputFilePath = nullptr;
  char *OutputFilePath = nullptr;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--verbose") == 0) {
      PrintTrees = true;
    } else {
      if (!InputFilePath) {
        InputFilePath = argv[i];
      } else {
        OutputFilePath = argv[i];
      }
    }
  }

  lexer_state Lexer;
  long Size;
  if (!InputFilePath) {
    printf("Error: no input file\n");
    return -1;
  }
  char *Source = SlurpFile(InputFilePath, &Size);
  LexerInit(&Lexer, Source, Source + Size);
  parse_node RootNode = ParserGetNode(&Lexer, token::END);
  if (PrintTrees)
    PrintParseTree(&RootNode, 0);
  ast_node ASTRoot = ast_node::BuildFromParseTree(nullptr, &RootNode);
  if (PrintTrees)
    PrintAST(&ASTRoot, 0);
  neocode_program Program = CGNeoBuildProgramInstance(&ASTRoot);
  if (OutputFilePath) {
    std::ofstream Fs;
    Fs.open(OutputFilePath);
    CGNeoGenerateCode(&Program, Fs);
  } else {
    CGNeoGenerateCode(&Program, std::cout);
  }
  return 0;
}

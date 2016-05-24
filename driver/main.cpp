#include "ast.h"
#include "codegen_neo.h"
#include "parser.h"
#include "preprocessor.h"
#include <cstring>
#include <fstream>
#include <iostream>

char *SlurpFile(const char *FilePath, long *FileSize) {
  std::ifstream is(FilePath);
  if (!is) return nullptr;
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

static symtable SymbolTable;

void PrintToken(token *Token) {
  if (Token->Type >= token::ATTRIBUTE && Token->Type <= token::WHILE) {
    printf("%s\n", Token->Id.c_str());
    return;
  }
  switch (Token->Type) {
  case token::ASM:
    printf("%s\n", Token->Id.c_str());
    break;
  case token::INLINE:
    printf("%s\n", Token->Id.c_str());
    break;
  case token::FLOATCONSTANT:
    printf("%f\n", Token->FloatValue);
    break;

  case token::INTCONSTANT:
    printf("%ld\n", Token->IntValue);
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

  case token::OR_OP:
    printf("||\n");
    break;

  // case token::CPPSTRING:
  //   printf("CPP: %s\n", Token->Id.c_str());
  //   break;

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
    for (size_t i = 0; i < Node->Children.size(); ++i) {
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
    printf("function:%s:%lu\n", Child.Id.c_str(), Child.Children.size());
    PrintAST(&Child, Depth + 1);
    break;
  case ast_node::FUNCTION_CALL:
    printf("call:%s:%lu\n", Child.Id.c_str(), Child.Children.size());
    PrintAST(&Child, Depth + 1);
    break;

  case ast_node::STRUCT:
    printf("struct:%s:%lu\n", Child.Id.c_str(), Child.Children.size());
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
    printf("Mul *\n");
    PrintAST(&Child, Depth + 1);
    break;

  case ast_node::DIVIDE:
    printf("Div /\n");
    PrintAST(&Child, Depth + 1);
    break;

  case ast_node::VARIABLE:
    switch (Child.VarType) {
    case ast_node::FLOAT:
      printf("float:%s:%s\n", Child.Id.c_str(),
             (Child.Modifiers & ast_node::DECLARE) ? "true" : "false");
      break;

    case ast_node::INT:
      printf("int:%s\n", Child.Id.c_str());
      break;

    case ast_node::FLOAT_LITERAL:
      printf("float:%f\n", Child.FloatValue);
      break;

    case ast_node::INT_LITERAL:
      printf("int:%ld\n", Child.IntValue);
      break;

    case ast_node::STRING_LITERAL:
      printf("string:%s\n", Child.Id.c_str());
      break;

    case ast_node::STRUCT:
      printf("struct:%s:%s\n", Child.Typename.c_str(), Child.Id.c_str());
      break;

    default:
      printf("var:%d:%s\n", Child.VarType, Child.Id.c_str());
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
  for (auto Child : AST->Children) {
    for (int i = 0; i < Depth; ++i) {
      printf("  ");
    }
    printf("D%d ", Depth);
    PrintASTNode(*Child, Depth);
  }
}

static void PrintHelp(const std::string &ExecName) {
  printf("Usage: %s <input_shader> [options]\n", ExecName.c_str());
  printf("Options:\n");
  printf("     -o <output>       | Select output file\n");
  printf("     -h,--help         | Show this help message\n");
  printf("     --verbose         | Print parse and syntax tree structures\n");

}

int main(int argc, char **argv) {
  bool PrintTrees = false;
  char *InputFilePath = nullptr;
  char *OutputFilePath = nullptr;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      PrintHelp(argv[0]);
      return 0;
    } else if (strcmp(argv[i], "--verbose") == 0) {
      PrintTrees = true;
    } else if (strcmp(argv[i], "-o") == 0) {
      OutputFilePath = argv[++i];
    } else {
      if (!InputFilePath) {
        InputFilePath = argv[i];
      }
    }
  }

  lexer_state Lexer;
  long Size;
  if (!InputFilePath) {
    printf("error: no input file\n");
    return -1;
  }
  char *Source = SlurpFile(InputFilePath, &Size);
  if (!Source) {
    printf("error: no such file or directory: \'%s\'\n", InputFilePath);
    return -1;
  }
  LexerInit(&Lexer, Source, Source + Size, &SymbolTable);
  parser Parser = parser(Lexer);
  parse_node RootNode = Parser.ParseTranslationUnit();
  if (PrintTrees)
    PrintParseTree(&RootNode, 0);

  // cpp_table CppTable;
  // CppDefineInt(&CppTable, "__FILE__", 1);
  // CppDefineInt(&CppTable, "GL_ES", 1);
  // CppDefineInt(&CppTable, "__VERSION__", 100);
  // CppResolveMacros(&CppTable, &RootNode);
  // if (PrintTrees)
  //   PrintParseTree(&RootNode, 0);
  ast_node *ASTRoot = ast_node::BuildFromParseTree(nullptr, &RootNode);
  if (PrintTrees)
    PrintAST(ASTRoot, 0);
  neocode_program Program = CGNeoBuildProgramInstance(ASTRoot);
  if (OutputFilePath) {
    std::ofstream Fs;
    Fs.open(OutputFilePath);
    CGNeoGenerateCode(&Program, Fs);
  }
  return 0;
}

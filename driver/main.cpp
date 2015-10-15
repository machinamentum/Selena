#include <fstream>
#include "parser.h"
#include "ast.h"

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

void PrintAST(ast_node *AST, int Depth) {
  for (ast_node Child : AST->Children) {
    for (int i = 0; i < Depth; ++i) {
      printf("  ");
    }
    printf("D%d ", Depth);
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

    case ast_node::VARIABLE:
      switch (Child.VarType) {
      case ast_node::FLOAT:
        printf("float:%s\n", Child.Id.c_str());
        break;

      case ast_node::INT:
        printf("int:%s\n", Child.Id.c_str());
        break;

      default:
        printf("var:%s\n", Child.Id.c_str());
        break;
      }
      break;

    default:
      printf("\n");
      PrintAST(&Child, Depth + 1);
      break;
    }
  }
}

int main(int argc, char **argv) {
  lexer_state Lexer;
  long Size;
  char *Source = SlurpFile(argv[1], &Size);
  LexerInit(&Lexer, Source, Source + Size);
  parse_node RootNode = ParserGetNode(&Lexer, token::END);
  PrintParseTree(&RootNode, 0);
  ast_node ASTRoot = ASTBuildFromParseTree(&RootNode);
  PrintAST(&ASTRoot, 0);
  return 0;
}

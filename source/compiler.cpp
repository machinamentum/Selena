#include "compiler.h"
#include "ast.h"
#include "codegen_shbin.h"
#include "parser.h"
#include <cstring>
#include <cstdlib>

static void (*UserErrorHandler)(const char *Msg) = nullptr;

static void ErrorCallback(const std::string &ErrMsg,
                          const std::string &OffendingLine, int LineNumber,
                          int LineOffset) {
  std::string Msg = "error:" + std::to_string(LineNumber) + ":" + std::to_string(LineOffset) + ErrMsg + "\n" + OffendingLine + "\n";
  if (UserErrorHandler) UserErrorHandler(Msg.c_str());
}

extern "C" {

void SelenaSetErrorHandler(void (*ErrorFunc)(const char *)) {
  UserErrorHandler = ErrorFunc;
}

char *SelenaCompileShaderSource(const char *Src, int *BinSize) {
  symtable SymbolTable;
  lexer_state Lexer;
  LexerInit(&Lexer, (char *)Src, (char *)Src + strlen(Src) + 1, &SymbolTable);
  parser Parser = parser(Lexer);
  Parser.ErrorFunc = ErrorCallback;
  parse_node RootNode = Parser.ParseTranslationUnit();

  ast_node ASTRoot = ast::BuildTranslationUnit(RootNode, &SymbolTable);
  neocode_program Program = CGNeoBuildProgramInstance(&ASTRoot, &SymbolTable);
  std::stringstream ss;
  CGShbinGenerateCode(&Program, ss);
  char *Shbin = (char *)malloc(ss.str().length() + 1);
  memcpy(Shbin, ss.str().c_str(), ss.str().length());
  *BinSize = ss.str().length();
  return Shbin;
}

}

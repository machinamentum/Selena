#include "parser.h"

parser::parser(lexer_state &L) : Lex(L) {
    SymbolTable = Lex.Table;
}

parse_node parser::Parse() {
    return parse_node();
}

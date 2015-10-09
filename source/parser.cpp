
#include "parser.h"

parse_node ParserGetParens(lexer_state *State) {
  parse_node Node = ParserGetNode(State, ')');
  return Node;
}

parse_node ParserGetCurls(lexer_state *State) {
  parse_node Node = ParserGetNode(State, '}');
  return Node;
}

parse_node ParserGetNode(lexer_state *State, int StatementEnd) {
  parse_node NodeParent;
  NodeParent.Type = parse_node::E;

  token Token = LexerGetToken(State);
  while (Token.Type != StatementEnd) {
    parse_node Child;
    Child.Type = parse_node::T;
    Child.Token = Token;
    NodeParent.Children.push_back(Child);
    if (Token.Type == '(') {
      NodeParent.Children.push_back(ParserGetParens(State));
      Child.Type = parse_node::T;
      Child.Token = { ')' };
      NodeParent.Children.push_back(Child);
    }
    else if (Token.Type == '{') {
      NodeParent.Children.push_back(ParserGetCurls(State));
      Child.Type = parse_node::T;
      Child.Token = { '}' };
      NodeParent.Children.push_back(Child);
    }

    Token = LexerGetToken(State);
  }

  return NodeParent;
}
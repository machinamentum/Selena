
#include "parser.h"

void ParserCreateStatmentBranches(parse_node *Node, int StatementEnd) {
  if (Node->Children.size() <= 0)
    return;
  parse_node StatementNode;
  StatementNode.Type = parse_node::E;
  std::vector<parse_node> ChildrenClone =
      std::vector<parse_node>(Node->Children);
  Node->Children.clear();
  for (int i = 0; i < ChildrenClone.size(); ++i) {
    StatementNode.Children.push_back(ChildrenClone[i]);
    if (ChildrenClone[i].Token.Type == StatementEnd) {
      Node->Children.push_back(StatementNode);
      StatementNode = parse_node();
    }
  }
  if (StatementNode.Children.size())
    Node->Children.push_back(StatementNode);
}

parse_node ParserGetParens(lexer_state *State) {
  parse_node Node = ParserGetNode(State, ')');
  ParserCreateStatmentBranches(&Node, ',');
  return Node;
}

parse_node ParserGetCurls(lexer_state *State) {
  parse_node Node = ParserGetNode(State, '}');
  ParserCreateStatmentBranches(&Node, ';');
  return Node;
}

parse_node ParserGetNode(lexer_state *State, int StatementEnd) {
  parse_node NodeParent;
  NodeParent.Type = parse_node::E;

  token Token = LexerGetToken(State);
  while (Token.Type != StatementEnd && Token.Type != token::END) {
    parse_node Child;
    Child.Type = parse_node::T;
    Child.Token = Token;
    NodeParent.Children.push_back(Child);
    if (Token.Type == '(') {
      NodeParent.Children.push_back(ParserGetParens(State));
    } else if (Token.Type == '{') {
      NodeParent.Children.push_back(ParserGetCurls(State));
    }

    Token = LexerGetToken(State);
  }
  parse_node Child;
  Child.Type = parse_node::T;
  Child.Token = Token;
  NodeParent.Children.push_back(Child);

  return NodeParent;
}


#include "ast.h"

ast_node ASTBuildFloat(parse_node *Node) {
  ast_node ASTNode;
  ASTNode.Type = ast_node::FLOAT;
  ASTNode.Id = Node->Children[1].Token.Id;
  return ASTNode;
}
ast_node ASTBuildInt(parse_node *Node) {
  ast_node ASTNode;
  ASTNode.Type = ast_node::INT;
  ASTNode.Id = Node->Children[1].Token.Id;
  return ASTNode;
}

ast_node ASTBuildVariable(parse_node *Node) {}
ast_node ASTBuildFunction(parse_node *Node) {}

ast_node ASTBuildStruct(parse_node *Node) {
  ast_node ASTNode;
  ASTNode.Type = ast_node::STRUCT;
  ASTNode.Id = Node->Children[1].Token.Id;

  for (int i = 3; i < Node->Children.size(); ++i) {
    ast_node ChildNode = ASTBuildFromParseTree(&Node->Children[i]);
    if (ChildNode.Type == ast_node::NONE && ChildNode.Children.size() == 0)
      continue;

    for (int i = 0; i < ChildNode.Children.size(); ++i) {
      ASTNode.Children.push_back(ChildNode.Children[i]);
    }
  }

  return ASTNode;
}

ast_node ASTBuildFromIdentifier(parse_node *Node, parse_node *PNode) {
  token *Token = &Node->Token;
  std::string Id = Token->Id;
  if (Id.compare("struct") == 0) {
    return ASTBuildStruct(PNode);
  } else if (Id.compare("float") == 0) {
    return ASTBuildFloat(PNode);
  } else if (Id.compare("int") == 0) {
    return ASTBuildInt(PNode);
  }
  return ast_node();
}

ast_node ASTBuildFromParseTree(parse_node *PNode) {
  ast_node ASTNode;
  ASTNode.Type = ast_node::NONE;
  if (PNode->Type == parse_node::E) {
    for (parse_node &PN : PNode->Children) {
      if (PN.Type == parse_node::E) {
        ast_node ChildNode = ASTBuildFromParseTree(&PN);
        if (ChildNode.Type == ast_node::NONE && ChildNode.Children.size() == 0)
          continue;

        for (int i = 0; i < ChildNode.Children.size(); ++i) {
          ASTNode.Children.push_back(ChildNode.Children[i]);
        }

      } else {
        ASTNode.Children.push_back(ASTBuildFromIdentifier(&PN, PNode));
        break;
      }
    }
  } else {
    // If a token ends up here, it's generally a bug
  }

  return ASTNode;
}

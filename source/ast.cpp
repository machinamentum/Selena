
#include "ast.h"

int ASTGetTypeFromString(std::string Typename) {
  if (Typename.compare("struct") == 0)
    return ast_node::STRUCT;
  if (Typename.compare("float") == 0)
    return ast_node::FLOAT;
  if (Typename.compare("int") == 0)
    return ast_node::INT;
  if (Typename.compare("void") == 0)
    return ast_node::VOID;
  if (Typename.compare("return") == 0)
    return ast_node::RETURN;
  return ast_node::NONE;
}

ast_node ASTBuildFunction(parse_node *Node) {
  ast_node ASTNode;
  ASTNode.Type = ast_node::FUNCTION;
  ASTNode.VarType = ASTGetTypeFromString(Node->Children[0].Token.Id);
  ASTNode.Id = Node->Children[1].Token.Id;
  ASTNode.Children.push_back(ASTBuildFromParseTree(&Node->Children[3]));
  if (Node->Children[5].Token.Type == '{') {
    for (parse_node &Child : Node->Children[6].Children) {
      ASTNode.Children.push_back(ASTBuildStatement(&Child));
    }
  }
  return ASTNode;
}

ast_node ASTBuildFunctionCall(parse_node *Node) {
  ast_node ASTNode;
  ASTNode.Type = ast_node::FUNCTION_CALL;
  ASTNode.Id = Node->Children[0].Token.Id;
  ast_node Params = ASTBuildFromParseTree(&Node->Children[2]);
  for (ast_node &Param : Params.Children) {
    ASTNode.Children.push_back(Param);
  }
  return ASTNode;
}

ast_node ASTBuildVariable(parse_node *Node) {
  ast_node ASTNode;
  ASTNode.Type = ast_node::VARIABLE;
  ASTNode.VarType = ast_node::NONE;
  ASTNode.Id = Node->Children[0].Token.Id;
  return ASTNode;
}

ast_node ASTBuildStatement(parse_node *Node) {
  ast_node ASTNode;
  int Type = ASTGetTypeFromString(Node->Children[0].Token.Id);
  if (Type == ast_node::NONE) {
    if (Node->Children[0].Token.Type == token::IDENTIFIER &&
        Node->Children[1].Token.Type == '=') {
      ASTNode.Type = ast_node::ASSIGNMENT;
      ASTNode.Id = Node->Children[0].Token.Id;
      ASTNode.Children.push_back(
          ASTBuildFromParseTree(&Node->Children[2]).Children[0]);
    } else {
      ASTNode.Children.push_back(
          ASTBuildFromIdentifier(&Node->Children[0], Node));
    }
  }

  return ASTNode;
}

ast_node ASTBuildReturn(parse_node *Node) {
  ast_node ASTNode;
  ASTNode.Type = ast_node::RETURN;
  ASTNode.Children.push_back(ASTBuildStatement(&Node->Children[1]));
  return ASTNode;
}

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

ast_node ASTBuildConstantPrimitive(parse_node *Node) {
  ast_node ASTNode;
  ASTNode.Type = ast_node::VARIABLE;
  ASTNode.VarType = ast_node::FLOAT;
  if (Node->Token.Type == token::FLOAT) {
    ASTNode.VarType = ast_node::FLOAT_LITERAL;
    ASTNode.FloatValue = Node->Token.FloatValue;
  } else if (Node->Token.Type == token::INT) {
    ASTNode.VarType = ast_node::INT_LITERAL;
    ASTNode.FloatValue = Node->Token.IntValue;
  }
  return ASTNode;
}

ast_node ASTBuildFromIdentifier(parse_node *Node, parse_node *PNode) {
  token *Token = &Node->Token;
  std::string Id = Token->Id;
  int Type = ASTGetTypeFromString(Id);
  if (Type != ast_node::NONE) {
    if (PNode->Children[2].Token.Type == '(') {
      return ASTBuildFunction(PNode);
    } else if (Type == ast_node::STRUCT) {
      return ASTBuildStruct(PNode);
    } else if (Type == ast_node::RETURN) {
      return ASTBuildReturn(PNode);
    } else {
      return ASTBuildVariable(PNode);
    }
  } else {
    if (PNode->Children[1].Token.Type == '(') {
      return ASTBuildFunctionCall(PNode);
    } else if (Token->Type == token::DQSTRING) {
      ast_node StringNode;
      StringNode.Type = ast_node::VARIABLE;
      StringNode.VarType = ast_node::STRING_LITERAL;
      StringNode.Id = Id;
      return StringNode;
    } else if (Node->Token.Type == token::FLOAT ||
               Node->Token.Type == token::INT) {
      return ASTBuildConstantPrimitive(Node);
    } else if (PNode->Children[1].Token.Type == ';') {
      return ASTBuildVariable(PNode);
    }
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

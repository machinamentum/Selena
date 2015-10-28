
#include "ast.h"

ast_node *ast_node::LookupType(std::string Typename) {
  for (ast_node &Type : Children) {
    if (Type.Type == ast_node::STRUCT && Type.Id.compare(Typename) == 0) {
      return &Type;
    }
  }

  if (Parent)
    return Parent->LookupType(Typename);
  return nullptr;
}

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

ast_node ast_node::BuildFunction(ast_node *Parent, parse_node *Node) {
  ast_node ASTNode = ast_node(Parent);
  ASTNode.Type = ast_node::FUNCTION;
  int ParamsIndex = -1;
  for (size_t i = 1; i < Node->Children.size(); ++i) {
    if (Node->Children[i].Token.Type == token::IDENTIFIER &&
        Node->Children[i + 1].Token.Type == '(') {
      ParamsIndex = i;
      break;
    }
  }

  for (int i = 0; i < ParamsIndex; ++i) {
    int Type = ASTGetTypeFromString(Node->Children[i].Token.Id);
    if (Type != ast_node::NONE) {
      ASTNode.VarType = Type;
    } else if (Node->Children[i].Token.Id.compare("inline") == 0) {
      ASTNode.Modifiers |= ast_node::INLINE;
    } else {
      // do error
    }
  }
  ASTNode.Id = Node->Children[ParamsIndex].Token.Id;
  ASTNode.PushChild(
      BuildFromParseTree(&ASTNode, &Node->Children[ParamsIndex + 2]));
  if (Node->Children[ParamsIndex + 4].Token.Type == '{') {
    for (parse_node &Child : Node->Children[ParamsIndex + 5].Children) {
      ASTNode.PushChild(BuildStatement(&ASTNode, &Child));
    }
  }
  return ASTNode;
}

ast_node ast_node::BuildFunctionCall(ast_node *Parent, parse_node *Node) {
  ast_node ASTNode = ast_node(Parent);
  ASTNode.Type = ast_node::FUNCTION_CALL;
  ASTNode.Id = Node->Children[0].Token.Id;
  ast_node Params = BuildFromParseTree(&ASTNode, &Node->Children[2]);
  for (ast_node &Param : Params.Children) {
    ASTNode.PushChild(Param);
  }
  return ASTNode;
}

ast_node ast_node::BuildVariable(ast_node *Parent, parse_node *Node) {
  ast_node ASTNode = ast_node(Parent);
  ASTNode.Type = ast_node::VARIABLE;
  ASTNode.Modifiers = 0;
  if (Node->Type == parse_node::T) {
    ASTNode.Id = Node->Token.Id;
    return ASTNode;
  }
  int Type = ASTGetTypeFromString(Node->Children[0].Token.Id);
  if (Type == ast_node::NONE &&
      ASTNode.LookupType(Node->Children[0].Token.Id)) {
    ASTNode.VarType = ast_node::STRUCT;
    ASTNode.Typename = Node->Children[0].Token.Id;
    ASTNode.Id = Node->Children[1].Token.Id;
  } else if (Type != ast_node::NONE) {
    ASTNode.VarType = Type;
    ASTNode.Id = Node->Children[1].Token.Id;
    ASTNode.Modifiers |= ast_node::DECLARE;
  } else {
    ASTNode.Id = Node->Children[0].Token.Id;
  }
  return ASTNode;
}

ast_node ast_node::BuildStatement(ast_node *Parent, parse_node *Node) {
  return BuildFromParseTree(Parent, Node);
}

ast_node ast_node::BuildReturn(ast_node *Parent, parse_node *Node) {
  ast_node ASTNode = ast_node(Parent);
  ASTNode.Type = ast_node::RETURN;
  ASTNode.PushChild(BuildFromParseTree(&ASTNode, &Node->Children[1]));
  return ASTNode;
}

ast_node ast_node::BuildStruct(ast_node *Parent, parse_node *Node) {
  ast_node ASTNode = ast_node(Parent);
  ASTNode.Type = ast_node::STRUCT;
  ASTNode.Id = Node->Children[1].Token.Id;

  for (size_t i = 3; i < Node->Children.size(); ++i) {
    ast_node ChildNode = BuildFromParseTree(&ASTNode, &Node->Children[i]);
    if (ChildNode.Type == ast_node::NONE && ChildNode.Children.size() == 0)
      continue;

    for (size_t i = 0; i < ChildNode.Children.size(); ++i) {
      ASTNode.PushChild(ChildNode.Children[i]);
    }
  }

  return ASTNode;
}

ast_node ast_node::BuildConstantPrimitive(ast_node *Parent, parse_node *Node) {
  ast_node ASTNode = ast_node(Parent);
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

ast_node ast_node::BuildFromIdentifier(ast_node *ASTParent, parse_node *PNode) {
  parse_node *Node = &PNode->Children[0];
  token *Token = &Node->Token;
  std::string Id = Token->Id;
  int Type = ASTGetTypeFromString(Id);
  if (PNode->Children[1].Token.Type == '=') {
    ast_node MultNode = ast_node(ASTParent);
    MultNode.Type = ast_node::ASSIGNMENT;
    MultNode.PushChild(
        BuildFromParseTree(&MultNode, &PNode->Children[0]).Children[0]);
    MultNode.PushChild(
        BuildFromParseTree(&MultNode, &PNode->Children[2]).Children[0]);
    return MultNode;
  } else if (PNode->Children[1].Token.Type == '*') {
    ast_node MultNode = ast_node(ASTParent);
    MultNode.Type = ast_node::MULTIPLY;
    MultNode.PushChild(
        BuildFromParseTree(&MultNode, &PNode->Children[0]).Children[0]);
    MultNode.PushChild(
        BuildFromParseTree(&MultNode, &PNode->Children[2]).Children[0]);
    return MultNode;
  } else if (PNode->Children[1].Token.Type == '/') {
    ast_node MultNode = ast_node(ASTParent);
    MultNode.Type = ast_node::DIVIDE;
    MultNode.PushChild(
        BuildFromParseTree(&MultNode, &PNode->Children[0]).Children[0]);
    MultNode.PushChild(
        BuildFromParseTree(&MultNode, &PNode->Children[2]).Children[0]);
    return MultNode;
  } else if (Type != ast_node::NONE) {
    for (size_t i = 1; i < PNode->Children.size() - 1; ++i) {
      if (PNode->Children[i].Token.Type == token::IDENTIFIER &&
          PNode->Children[i + 1].Token.Type == '(') {
        return BuildFunction(ASTParent, PNode);
      }
    }
    if (Type == ast_node::STRUCT) {
      return BuildStruct(ASTParent, PNode);
    } else if (Type == ast_node::RETURN) {
      return BuildReturn(ASTParent, PNode);
    } else {
      return BuildVariable(ASTParent, PNode);
    }
  } else {
    if (PNode->Children[0].Token.Id.compare("inline") == 0) {
      for (size_t i = 1; i < PNode->Children.size() - 1; ++i) {
        if (PNode->Children[i].Token.Type == token::IDENTIFIER &&
            PNode->Children[i + 1].Token.Type == '(') {
          return BuildFunction(ASTParent, PNode);
        }
      }
    }
    if (PNode->Children[1].Token.Type == '(') {
      return BuildFunctionCall(ASTParent, PNode);
    } else if (Token->Type == token::DQSTRING) {
      ast_node StringNode = ast_node(ASTParent);
      StringNode.Type = ast_node::VARIABLE;
      StringNode.VarType = ast_node::STRING_LITERAL;
      StringNode.Id = Id;
      return StringNode;
    } else if (Token->Type == token::FLOAT || Token->Type == token::INT) {
      return BuildConstantPrimitive(ASTParent, Node);
    } else {
      return BuildVariable(ASTParent, PNode);
    }
  }
  return ast_node(ASTParent);
}

ast_node ast_node::BuildFromParseTree(ast_node *Parent, parse_node *PNode) {
  ast_node ASTNode = ast_node(Parent);
  ASTNode.Type = ast_node::NONE;
  if (PNode->Type == parse_node::E) {
    for (parse_node &PN : PNode->Children) {
      if (PNode->Children.size() == 3 &&
          PNode->Children[1].Type == parse_node::T) {
        ast_node ASTTemp = BuildFromIdentifier(&ASTNode, PNode);
        ASTNode.PushChild(ASTTemp);
        break;
      } else {

        if (PN.Type == parse_node::E) {
          ast_node ChildNode = BuildFromParseTree(&ASTNode, &PN);
          if (ChildNode.Type == ast_node::NONE &&
              ChildNode.Children.size() == 0)
            continue;

          for (size_t i = 0; i < ChildNode.Children.size(); ++i) {
            ASTNode.PushChild(ChildNode.Children[i]);
          }

        } else {
          ast_node ASTTemp = BuildFromIdentifier(&ASTNode, PNode);
          ASTNode.PushChild(ASTTemp);
          break;
        }
      }
    }
  } else {
    // If a token ends up here, it's generally a bug
  }

  return ASTNode;
}

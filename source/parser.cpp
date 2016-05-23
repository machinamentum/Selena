#include "parser.h"
#include <cstdlib>
#include <functional>

void PrintToken(token *Token);
static void error(const char *S, token &T) {
    printf("Error:%d:%d: %s\n", T.Line, T.Offset, S);
    PrintToken(&T);
    exit(-1);
}

parser::parser(lexer_state &L) : Lex(L) {
    SymbolTable = Lex.Table;
}

void parser::Match(int T) {
    if (T == Token.Type) {
        Token = Lex.GetToken();
    } else {
        printf("Expected %d:%c\n", T, T);
        token N = Lex.PeekToken();
        printf("Next token "); PrintToken(&N);
        error("Unexpected token", Token);
    }
}

parse_node parser::ParseTypeSpecifier() {

}

parse_node parser::ParseFullySpecifiedType() {
    parse_node N;
    // TODO
    if (Token.Type == token::VOID) {
        N.Children.push_back(parse_node(Token));
        Match(token::VOID);
        return N;
    }
    error("Not correct type", Token);
    return parse_node();
}

parse_node parser::ParseParameterDeclaration() {

}

parse_node parser::ParseFunctionHeaderWithParameters() {

}

parse_node parser::ParseFunctionHeader() {
    parse_node N;
    N.Append(ParseFullySpecifiedType());
    N.Children.push_back(Token);
    Match(token::IDENTIFIER);
    N.Children.push_back(parse_node(token::LEFT_PAREN));
    Match(token::LEFT_PAREN);
    return N;
}

parse_node parser::ParseFunctionDeclarator() {
    parse_node N = ParseFunctionHeader();
    if (Token.Type != token::RIGHT_PAREN) {
        N.Children.push_back(ParseParameterDeclaration());
    } else {
        N.Children.push_back(parse_node());
    }
    return N;
}

parse_node parser::ParseFunctionPrototype() {
    parse_node N;
    N.Append(ParseFunctionDeclarator());
    Match(token::RIGHT_PAREN);
    N.Children.push_back(parse_node(token::RIGHT_PAREN));
    return N;
}

bool parser::IsAssignmentOp(int T) {
    switch (T) {
        case token::EQUAL:
        case token::MUL_ASSIGN:
        case token::DIV_ASSIGN:
        case token::ADD_ASSIGN:
        case token::SUB_ASSIGN:
        case token::MOD_ASSIGN:
        case token::LEFT_ASSIGN:
        case token::RIGHT_ASSIGN:
        case token::AND_ASSIGN:
        case token::XOR_ASSIGN:
        case token::OR_ASSIGN:
            return true;
        default:
            return false;
    }
}

parse_node parser::ParseAssignmentOperator() {
    switch (Token.Type) {
        case token::EQUAL:
        case token::MUL_ASSIGN:
        case token::DIV_ASSIGN:
        case token::ADD_ASSIGN:
        case token::SUB_ASSIGN: {
            parse_node N = parse_node(Token);
            Match(Token.Type);
            return N;
        }

        case token::MOD_ASSIGN:
        case token::LEFT_ASSIGN:
        case token::RIGHT_ASSIGN:
        case token::AND_ASSIGN:
        case token::XOR_ASSIGN:
        case token::OR_ASSIGN:
            error("Operator is reserved", Token);
            return parse_node();
        default:
            error("Unexpected token", Token);
            return parse_node();
    }
}

parse_node parser::ParseFunctionCall() {
    // TODO
    return parse_node();
}

parse_node parser::ParseIntegerExpression() {
    // TODO
    return parse_node();
}

parse_node parser::ParseOperatorExpression(ParseFuncPtr R, std::vector<int> TokenTypes) {
    #define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))
    auto IsOfType = [](int T, std::vector<int> &v) -> bool {
        for (int &i : v) {
            if (i == T) return true;
        }
        return false;
    };
    std::function<parse_node (parse_node &)> ParseExtL = [&](parse_node &P) -> parse_node {
        parse_node N;
        if (IsOfType(Token.Type, TokenTypes)) {
            N.Children.push_back(P);
            N.Children.push_back(parse_node(Token));
            Match(Token.Type);
            N.Children.push_back(CALL_MEMBER_FN(*this, R)());
            parse_node C = ParseExtL(N);
            if (!C.Empty()) {
                return C;
            }
        }
        return N;
    };
    parse_node RP = CALL_MEMBER_FN(*this, R)();
    parse_node LP = ParseExtL(RP);
    if (!LP.Empty()) {
        return LP;
    }
    return RP;
    #undef CALL_MEMBER_FN
}

parse_node parser::ParseMultiplicativeExpression() {
    return ParseOperatorExpression(&parser::ParseUnaryExpression, {token::STAR, token::SLASH, token::PERCENT});
}

parse_node parser::ParseAdditiveExpression() {
    return ParseOperatorExpression(&parser::ParseMultiplicativeExpression, {token::PLUS, token::DASH});
}

parse_node parser::ParseShiftExpression() {
    return ParseOperatorExpression(&parser::ParseAdditiveExpression, {token::LEFT_OP, token::RIGHT_OP});
}

parse_node parser::ParseRelationalExpression() {
    return ParseOperatorExpression(&parser::ParseShiftExpression, {token::LEFT_ANGLE, token::RIGHT_ANGLE, token::LE_OP, token::GE_OP});
}

parse_node parser::ParseEqualityExpression() {
    return ParseOperatorExpression(&parser::ParseRelationalExpression, {token::EQ_OP, token::NE_OP});
}

parse_node parser::ParseAndExpression() {
    return ParseOperatorExpression(&parser::ParseEqualityExpression, {token::AMPERSAND});
}

parse_node parser::ParseExclusiveOrExpression() {
    return ParseOperatorExpression(&parser::ParseAndExpression, {token::CARET});
}

parse_node parser::ParseInclusiveOrExpression() {
    return ParseOperatorExpression(&parser::ParseExclusiveOrExpression, {token::VERTICLE_BAR});
}

parse_node parser::ParseLogicalAndExpression() {
    return ParseOperatorExpression(&parser::ParseInclusiveOrExpression, {token::AND_OP});
}

parse_node parser::ParseLogicalXOrExpression() {
    return ParseOperatorExpression(&parser::ParseLogicalAndExpression, {token::XOR_OP});
}

parse_node parser::ParseLogicalOrExpression() {
    return ParseOperatorExpression(&parser::ParseLogicalXOrExpression, {token::OR_OP});
}

parse_node parser::ParsePrimaryExpression() {
    parse_node N;
    if (Token.Type == token::LEFT_PAREN) {
        Match(token::LEFT_PAREN);
        N.Children.push_back(ParseExpression());
        Match(token::RIGHT_PAREN);
        return N;
    }
    switch (Token.Type) {
        case token::INTCONSTANT:
        case token::FLOATCONSTANT:
        case token::BOOLCONSTANT:
        case token::IDENTIFIER:
            N = parse_node(Token);
            Match(Token.Type);
            return N;

        default: {
            error("Unexpected token", Token);
            return parse_node();
        }
    }
}

parse_node parser::ParsePostfixExpression() {
    std::function<parse_node (parse_node &)> ParseExtPostfix = [&](parse_node &P) -> parse_node {
        parse_node N;
        if (Token.Type == token::LEFT_BRACKET) {
            N.Children.push_back(P);
            Match(token::LEFT_BRACKET);
            N.Children.push_back(ParseIntegerExpression());
            Match(token::RIGHT_BRACKET);
        } else if (Token.Type == token::DEC_OP || Token.Type == token::INC_OP) {
            N.Children.push_back(parse_node(Token));
            Match(Token.Type);
        } else if (Token.Type == token::DOT) {
            N.Children.push_back(parse_node(Token));
            Match(token::DOT);
            N.Children.push_back(parse_node(Token));
            Match(token::FIELD_SELECTION);
        }
        return N;
    };
    parse_node Main;
    if (Lex.PeekToken().Type == token::LEFT_PAREN) {
        Main = ParseFunctionCall();
    } else {
        Main = ParsePrimaryExpression();
    }
    parse_node Ext = ParseExtPostfix(Main);
    if (!Ext.Empty()) {
        return Ext;
    }
    return Main;
}

parse_node parser::ParseUnaryExpression() {
    parse_node N;
    switch (Token.Type) {
        case token::INC_OP:
        case token::DEC_OP:
        case token::PLUS:
        case token::DASH:
        case token::BANG:
            N.Children.push_back(parse_node(Token));
            Match(Token.Type);
            N.Children.push_back(ParseUnaryExpression());
            return N;
        case token::TILDE:
            error("Operator reserved", Token);
    }
    N.Children.push_back(ParsePostfixExpression());
    return N;
}

parse_node parser::ParseConditionalExpression() {
    parse_node N;
    parse_node L = ParseLogicalOrExpression();
    if (Token.Type != token::QUESTION) {
        return L;
    }
    Match(token::QUESTION);
    N.Children.push_back(ParseExpression());
    N.Children.push_back(parse_node(Token));
    Match(token::COLON);
    N.Children.push_back(ParseAssignmentExpression());
    return N;
}

parse_node parser::ParseAssignmentExpression() {
    lexer_state S(Lex);
    token Tok(Token);
    parse_node U = ParseUnaryExpression();
    if (!IsAssignmentOp(Token.Type)) {
        Lex = S;
        Token = Tok;
        parse_node Cond = ParseConditionalExpression();
        if (!IsAssignmentOp(Token.Type)) return Cond;
        U = Cond;
    }
    parse_node N;
    N.Children.push_back(U);

    N.Children.push_back(ParseAssignmentOperator());
    N.Children.push_back(ParseAssignmentExpression());
    return N;
}

parse_node parser::ParseExpression() {
    parse_node N;
    expr_reset:
    N.Append(ParseAssignmentExpression());
    if (Token.Type == token::COMMA) {
        N.Children.push_back(parse_node(Token));
        Match(token::COMMA);
        goto expr_reset;
    }
    return N;
}

parse_node parser::ParseExpressionStatement() {
    parse_node N;
    if (Token.Type != token::SEMICOLON) {
        N.Append(ParseExpression());
    }
    Match(token::SEMICOLON);
    return N;
}

parse_node parser::ParseSimpleStatement() {
    // TODO other simple statements
    return ParseExpressionStatement();
}

parse_node parser::ParseCompoundStatementWithScope() {
    parse_node N;
    N.Children.push_back(Token);
    Match(token::LEFT_BRACE);
    if (Token.Type != token::RIGHT_BRACE) {
        N.Children.push_back(ParseStatementList());
    }
    N.Children.push_back(Token);
    Match(token::RIGHT_BRACE);
    return N;
}

parse_node parser::ParseStatementNoNewScope() {
    if (Token.Type == token::LEFT_BRACE) {
        return ParseCompoundStatementWithScope();
    }
    return ParseSimpleStatement();
}

parse_node parser::ParseStatementList() {
    parse_node N;
    while (Token.Type != token::RIGHT_BRACE) {
        N.Children.push_back(ParseStatementNoNewScope());
    }
    return N;
}

parse_node parser::ParseCompoundStatementNoNewScope() {
    parse_node N;
    N.Children.push_back(Token);
    Match(token::LEFT_BRACE);
    if (Token.Type != token::RIGHT_BRACE) {
        N.Children.push_back(ParseStatementList());
    } else {
        N.Children.push_back(parse_node());
    }
    N.Children.push_back(Token);
    Match(token::RIGHT_BRACE);
    return N;
}

parse_node parser::ParseFunctionDefinition() {
    parse_node N;
    N.Append(ParseFunctionPrototype());
    N.Append(ParseCompoundStatementNoNewScope());
    return N;
}

parse_node parser::Parse() {
    Token = Lex.GetToken();
    parse_node N;
    while (Token.Type != token::END) {
        N.Append(ParseFunctionDefinition());
    }

    return N;
}

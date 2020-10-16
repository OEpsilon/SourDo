#pragma once

#include "Datatypes/Token.h"

namespace srdo {
    class Node;
    class ExpressionNode;

    class Parser
    {
    public:
        Parser(const std::vector<Token>& tokenList);

        void parse();
        const std::shared_ptr<Node>& getAST();
    private:
        std::shared_ptr<Node> ast;
        const std::vector<Token>& tokenList;
        const Token* currentTok;
        int index;
    private:
        typedef std::shared_ptr<ExpressionNode> (Parser::*ParseExprFunc)(std::shared_ptr<ExpressionNode>);

        enum class ExprPrec 
        {
            None = 0,
            OrExpr,
            XorExpr,
            AndExpr,
            NotExpr,
            CompExpr,
            BitOrExpr,
            BitXorExpr,
            BitAndExpr,
            AdditiveExpr,
            Factor,
            Sign,
            BitNotExpr,
            Power,
            Primary,
        };
        
        struct ParseExprRule 
        {
            ParseExprFunc prefix;
            ParseExprFunc infix;
            ExprPrec precedence;
        };
    private:  
        void advance();

        std::shared_ptr<Node> module();
        std::shared_ptr<Node> topLevelDecl();
        std::shared_ptr<Node> funcDecl();
        std::shared_ptr<Node> paramList();
        std::shared_ptr<Node> parameter();
        std::shared_ptr<Node> stmtList();
        std::shared_ptr<Node> statement();
        std::shared_ptr<Node> exprStmt();
        std::shared_ptr<Node> varDecl();
        std::shared_ptr<Node> ifStmt();
        std::shared_ptr<ExpressionNode> expression(ExprPrec precedence);

        ParseExprRule& getRule(const TType& type);

        std::shared_ptr<ExpressionNode> unaryOp(std::shared_ptr<ExpressionNode> previousNode);
        std::shared_ptr<ExpressionNode> binaryOp(std::shared_ptr<ExpressionNode> previousNode);
    };
}

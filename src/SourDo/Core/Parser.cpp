#include "Parser.h"

#include "Datatypes/Node.h"

namespace srdo {
    Parser::Parser(const std::vector<Token>& tokenList)
        : tokenList(tokenList), index(0)
    {
        currentTok = &(tokenList[index]);
    }

    void Parser::advance()
    {
        index++;
        currentTok = (index < tokenList.size())? &(tokenList[index]) : currentTok;
    }
    
    std::shared_ptr<Node> Parser::module()
    {
        return topLevelDecl();
    }

    std::shared_ptr<Node> Parser::topLevelDecl()
    {

    }

    std::shared_ptr<Node> Parser::funcDecl()
    {
        
    }

    std::shared_ptr<Node> Parser::paramList()
    {
        
    }

    std::shared_ptr<Node> Parser::parameter()
    {
        
    }

    std::shared_ptr<Node> Parser::stmtList()
    {
        
    }

    std::shared_ptr<Node> Parser::statement()
    {
        
    }

    std::shared_ptr<Node> Parser::exprStmt()
    {
        
    }

    std::shared_ptr<Node> Parser::varDecl()
    {
        
    }

    std::shared_ptr<Node> Parser::ifStmt()
    {
        if(currentTok->type == TType::Keyword && currentTok->value == "");
    }

    std::shared_ptr<ExpressionNode> Parser::expression(ExprPrec precedence)
    {
        
    }

    Parser::ParseExprRule& Parser::getRule(const TType& type)
    {
        static std::unordered_map<TType, ParseExprRule> rules = 
        {
            //                  Prefix                  Infix                   Precedence
            {TType::Unknown,    {nullptr,               nullptr,                ExprPrec::None          }},
            // Arithmetic Operators
            {TType::Plus,       {&Parser::unaryOp,      &Parser::binaryOp,      ExprPrec::AdditiveExpr  }},
            {TType::Minus,      {&Parser::unaryOp,      &Parser::binaryOp,      ExprPrec::AdditiveExpr  }}
        };
        
        if(rules.find(type) == rules.end())
        {
            return rules[TType::Unknown];
        }
        return rules[type];
    }

    std::shared_ptr<ExpressionNode> Parser::unaryOp(std::shared_ptr<ExpressionNode> previousNode)
    {
        Token opTok = *currentTok;
        advance();

        std::shared_ptr<ExpressionNode> operand;
        switch(currentTok->type)
        {
            case TType::Plus:

            case TType::Minus:
            case TType::Not:
        }

        return std::make_shared<UnaryOpNode>(opTok, operand);
    }

    std::shared_ptr<ExpressionNode> Parser::binaryOp(std::shared_ptr<ExpressionNode> previousNode)
    {
        
    }
}

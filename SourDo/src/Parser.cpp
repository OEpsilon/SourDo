#include "Parser.hpp"

namespace SourDo
{
    ParseResult Parser::parse_tokens(const std::vector<Token>& tokens)
    {
        this->tokens = tokens;
        position = 0;
        current_token = tokens[position];
        std::shared_ptr<Node> ast = expression(ExprPrecedence::AddExpr);
        return {ast, error};
    }
    
    Token Parser::advance()
    {
        position++;
        if(position < tokens.size())
        {
            current_token = tokens[position];
        }
        return current_token;
    }

    std::shared_ptr<ExpressionNode> Parser::expression(ExprPrecedence precedence)
    {
        ParseExprFunc prefix_func = get_rule(current_token.type).prefix;
        if(prefix_func == nullptr)
        {
            error = Error("Expected an expression", current_token.position);
            return nullptr;
        }
        std::shared_ptr<ExpressionNode> previous_operand = std::invoke(prefix_func, this, nullptr);
        if(error)
        {
            return nullptr;
        }

        while(precedence <= get_rule(current_token.type).precedence)
        {
            ParseExprFunc infix_func = get_rule(current_token.type).infix;
            previous_operand = std::invoke(infix_func, this, previous_operand);
            if(error)
            {
                return nullptr;
            }
        }
        return previous_operand;
    }

    std::shared_ptr<ExpressionNode> Parser::binary_op_left(std::shared_ptr<ExpressionNode> previous)
    {
        Token op_token = current_token;
        advance();
        ExprPrecedence precedence = static_cast<ExprPrecedence>(
                static_cast<int>(get_rule(op_token.type).precedence) + 1);
        std::shared_ptr<ExpressionNode> right = expression(precedence);
        return std::make_shared<BinaryOpNode>(previous, op_token, right);
    }

    std::shared_ptr<ExpressionNode> Parser::factor(std::shared_ptr<ExpressionNode> previous)
    {
        Token tok = current_token;
        if(tok.type == Token::Type::INT || tok.type == Token::Type::FLOAT)
        {
            advance();
            return std::make_shared<LiteralNode>(tok);
        }
        error = Error("Expected an expression", tok.position);
        return nullptr;
    }

    Parser::ParseExprRule& Parser::get_rule(const Token::Type& type)
    {
        static std::unordered_map<Token::Type, ParseExprRule> rules =
        {
            //                      Prefix              Infix                       Precedence
            {Token::Type::NONE,     {nullptr,           nullptr,                    ExprPrecedence::None    }},
            {Token::Type::INT,      {&Parser::factor,   nullptr,                    ExprPrecedence::Factor  }},
            {Token::Type::FLOAT,    {&Parser::factor,   nullptr,                    ExprPrecedence::Factor  }},
            {Token::Type::PLUS,     {nullptr,           &Parser::binary_op_left,    ExprPrecedence::AddExpr }},
            {Token::Type::MINUS,    {nullptr,           &Parser::binary_op_left,    ExprPrecedence::AddExpr }},
            {Token::Type::TK_EOF,   {nullptr,           nullptr,                    ExprPrecedence::None    }},
        };

        return rules[type];
    }

} // namespace SourDo


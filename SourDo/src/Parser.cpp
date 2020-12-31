#include "Parser.hpp"

namespace SourDo
{
    std::shared_ptr<Node> Parser::parse_tokens(const std::vector<Token>& tokens)
    {
        this->tokens = tokens;
        position = 0;
        std::cout << tokens.size();
        current_token = tokens[position];
        std::shared_ptr<Node> ast = expression(ExprPrecedence::AddExpr);
        return ast;
    }
    
    Token Parser::advance()
    {
        position++;
        if(position < tokens.size())
        {
            current_token = tokens[position];
        }
        std::cout << current_token << "\n";
        return current_token;
    }

    std::shared_ptr<ExpressionNode> Parser::expression(ExprPrecedence precedence)
    {
        ParseExprFunc prefix_func = get_rule(current_token.type).prefix;
        std::shared_ptr<ExpressionNode> previous_operand = std::invoke(prefix_func, this, nullptr);

        while(precedence <= get_rule(current_token.type).precedence)
        {
            ParseExprFunc infix_func = get_rule(current_token.type).infix;
            previous_operand = std::invoke(infix_func, this, previous_operand);
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
        Token literal_tok = current_token;
        advance();
        return std::make_shared<LiteralNode>(literal_tok);
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


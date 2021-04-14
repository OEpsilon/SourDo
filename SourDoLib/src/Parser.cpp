#include "Interpreter.hpp"

#include <unordered_map>
#include <sstream>

namespace sourdo
{
    ParserReturn Parser::parse_tokens(const std::vector<Token>& tokens)
    {
        this->tokens = tokens;
        position = 0;
        current_token = tokens[position];
        std::shared_ptr<Node> ast = statement();
        
        if(!error && current_token.type != Token::Type::TK_EOF)
        {
            return {nullptr, "Expected an operator"};
        }

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

    std::shared_ptr<Node> Parser::statement()
    {
        if(current_token == Token(Token::Type::KEYWORD, "var"))
        {
            advance();
            if(current_token.type != Token::Type::IDENTIFIER)
            {
                error = "Expected an identifier";
                return nullptr;
            }
            Token name_tok = current_token;
            advance();
            if(current_token.type != Token::Type::ASSIGN)
            {
                return std::make_shared<VarDeclarationNode>(name_tok, std::make_shared<NullValueNode>());
            }
            advance();
            std::shared_ptr<ExpressionNode> expr = expression(ExprPrecedence::ADD_EXPR);
            if(error)
            {
                return nullptr;
            }
            return std::make_shared<VarDeclarationNode>(name_tok, expr);
        }
        // Looking ahead at the next token probably isn't the best idea but it works for now.
        else if(current_token.type == Token::Type::IDENTIFIER && tokens[position + 1].type == Token::Type::ASSIGN)
        {
            Token name_tok = current_token;
            advance();
            advance();
            std::shared_ptr<ExpressionNode> expr = expression(ExprPrecedence::ADD_EXPR);
            if(error)
            {
                return nullptr;
            }
            return std::make_shared<VarAssignmentNode>(name_tok, expr);
        }

        return expression(ExprPrecedence::ADD_EXPR);
    }

    std::shared_ptr<ExpressionNode> Parser::expression(ExprPrecedence precedence)
    {
        if(current_token.type == Token::Type::LPAREN)
        {
            advance();
            std::shared_ptr<ExpressionNode> expr = expression(ExprPrecedence::ADD_EXPR);
            if(error)
            {
                return nullptr;
            }
            if(current_token.type != Token::Type::RPAREN)
            {
                error = "Expected a ')'";
                return nullptr;
            }
            return expr;
        }

        ParseExprFunc prefix_func = get_rule(current_token.type).prefix;
        if(prefix_func == nullptr)
        {
            error = "Expected an expression";
            return nullptr;
        }
        std::shared_ptr<ExpressionNode> previous_operand = std::invoke(prefix_func, this, nullptr);
        if(error)
        {
            return nullptr;
        }

        if(get_rule(current_token.type).infix == nullptr)
        {
            return previous_operand;
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

    std::shared_ptr<ExpressionNode> Parser::binary_op_right(std::shared_ptr<ExpressionNode> previous)
    {
        Token op_token = current_token;
        advance();
        ExprPrecedence precedence = static_cast<ExprPrecedence>(
                static_cast<int>(get_rule(op_token.type).precedence));
        std::shared_ptr<ExpressionNode> right = expression(precedence);
        return std::make_shared<BinaryOpNode>(previous, op_token, right);
    }

    std::shared_ptr<ExpressionNode> Parser::sign(std::shared_ptr<ExpressionNode> previous)
    {
        Token op_token = current_token;
        advance();
        std::shared_ptr<ExpressionNode> operand = expression(ExprPrecedence::POWER);
        return std::make_shared<UnaryOpNode>(op_token, operand);
    }

    std::shared_ptr<ExpressionNode> Parser::factor(std::shared_ptr<ExpressionNode> previous)
    {
        Token tok = current_token;
        if(tok.type == Token::Type::IDENTIFIER)
        {
            advance();
            return std::make_shared<VarAccessNode>(tok);
        }
        else if(tok.type == Token::Type::INT_LITERAL)
        {
            advance();
            return std::make_shared<IntValueNode>(tok);
        }
        else if(tok.type == Token::Type::FLOAT_LITERAL)
        {
            advance();
            return std::make_shared<FloatValueNode>(tok);
        }
        error = "Expected an expression";
        return nullptr;
    }

    Parser::ParseExprRule& Parser::get_rule(const Token::Type& type)
    {
        static std::unordered_map<Token::Type, ParseExprRule> rules =
        {
            //                              Prefix              Infix                       Precedence
            {Token::Type::NONE,             {nullptr,           nullptr,                    ExprPrecedence::NONE        }},
            {Token::Type::IDENTIFIER,       {&Parser::factor,   nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::INT_LITERAL,      {&Parser::factor,   nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::FLOAT_LITERAL,    {&Parser::factor,   nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::ADD,              {&Parser::sign,     &Parser::binary_op_left,    ExprPrecedence::ADD_EXPR    }},
            {Token::Type::SUB,              {&Parser::sign,     &Parser::binary_op_left,    ExprPrecedence::ADD_EXPR    }},
            {Token::Type::MUL,              {nullptr,           &Parser::binary_op_left,    ExprPrecedence::MUL_EXPR    }},
            {Token::Type::DIV,              {nullptr,           &Parser::binary_op_left,    ExprPrecedence::MUL_EXPR    }},
            {Token::Type::POW,              {nullptr,           &Parser::binary_op_right,   ExprPrecedence::POWER       }},
            {Token::Type::TK_EOF,           {nullptr,           nullptr,                    ExprPrecedence::NONE        }},
        };

        return rules[type];
    }
} // namespace sourdo

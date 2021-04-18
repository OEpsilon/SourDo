#include "Interpreter.hpp"

#include <unordered_map>
#include <sstream>

#include <iostream>

namespace sourdo
{
    ParserReturn Parser::parse_tokens(const std::vector<Token>& tokens)
    {
        this->tokens = tokens;
        position = 0;
        current_token = tokens[position];
        std::shared_ptr<Node> ast = statement_list();
        
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

    std::shared_ptr<StatementListNode> Parser::statement_list()
    {
        std::vector<std::shared_ptr<Node>> statements;

        while(current_token.type == Token::Type::NEW_LINE)
        {
            advance();
        }

        while(current_token != Token(Token::Type::KEYWORD, "end") &&
                current_token != Token(Token::Type::KEYWORD, "elif") &&
                current_token != Token(Token::Type::KEYWORD, "else") &&
                current_token.type != Token::Type::TK_EOF)
        {
            std::shared_ptr<Node> stmt = statement();
            if(error)
            {
                return nullptr;
            }
            statements.emplace_back(stmt);

            uint32_t new_line_count = 0;
            while(current_token.type == Token::Type::NEW_LINE)
            {
                advance();
                new_line_count++;
            }
            if(new_line_count == 0)
            {
                break;
            }
        }
        return std::make_shared<StatementListNode>(statements);
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
            std::shared_ptr<ExpressionNode> expr = expression(ExprPrecedence::LOGIC_OR);
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
            std::shared_ptr<ExpressionNode> expr = expression(ExprPrecedence::LOGIC_OR);
            if(error)
            {
                return nullptr;
            }
            return std::make_shared<VarAssignmentNode>(name_tok, expr);
        }
        else if(current_token == Token(Token::Type::KEYWORD, "if"))
        {
            std::vector<IfNode::IfBlock> cases;
            std::shared_ptr<StatementListNode> else_case;
            advance();
            
            auto if_expr_case = [&]() -> void
            {
                std::shared_ptr<ExpressionNode> condition = expression(ExprPrecedence::LOGIC_OR);
                if(error)
                {
                    return;
                }
                
                if(current_token != Token(Token::Type::KEYWORD, "then"))
                {
                    error = "Expected 'then'";
                    return;
                }
                advance();

                std::shared_ptr<StatementListNode> statements = statement_list();
                if(error)
                {
                    return;
                }

                cases.emplace_back(condition, statements);
            };

            if_expr_case();
            if(error)
            {
                return nullptr;
            }

            while(current_token == Token(Token::Type::KEYWORD, "elif"))
            {
                advance();

                if_expr_case();
                if(error)
                {
                    return nullptr;
                }
            }

            if(current_token == Token(Token::Type::KEYWORD, "else"))
            {
                advance();

                else_case = statement_list();
                if(error)
                {
                    return nullptr;
                }
            }

            if(current_token != Token(Token::Type::KEYWORD, "end"))
            {
                error = "Expected 'end'";
                return nullptr;
            }
            advance();


            return std::make_shared<IfNode>(cases, else_case);
        }

        return expression(ExprPrecedence::LOGIC_OR);
    }

    std::shared_ptr<ExpressionNode> Parser::expression(ExprPrecedence precedence)
    {
        if(current_token.type == Token::Type::LPAREN)
        {
            advance();
            std::shared_ptr<ExpressionNode> expr = expression(ExprPrecedence::LOGIC_OR);
            if(error)
            {
                return nullptr;
            }
            if(current_token.type != Token::Type::RPAREN)
            {
                error = "Expected a ')'";
                return nullptr;
            }
            advance();
            return expr;
        }

        ParseExprFunc prefix_func = get_rule(current_token.type).prefix;
        if(prefix_func == nullptr)
        {
            error = "Expected an expression";
            return nullptr;
        }
        std::shared_ptr<ExpressionNode> previous_operand = (this->*prefix_func)(nullptr);
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
            previous_operand = (this->*infix_func)(previous_operand);
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

    std::shared_ptr<ExpressionNode> Parser::logic_not(std::shared_ptr<ExpressionNode> previous)
    {
        Token op_token = current_token;
        advance();
        std::shared_ptr<ExpressionNode> operand = expression(ExprPrecedence::COMPARISON);
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
        else if(tok.type == Token::Type::NUMBER_LITERAL)
        {
            advance();
            return std::make_shared<NumberValueNode>(tok);
        }
        else if(tok.type == Token::Type::BOOL_LITERAL)
        {
            advance();
            return std::make_shared<BoolValueNode>(tok);
        }
        else if(tok.type == Token::Type::NULL_LITERAL)
        {
            advance();
            return std::make_shared<NullValueNode>();
        }
        error = "Expected an expression";
        return nullptr;
    }

    Parser::ParseExprRule& Parser::get_rule(const Token::Type& type)
    {
        static std::unordered_map<Token::Type, ParseExprRule> rules =
        {
            //                              Prefix                  Infix                       Precedence
            {Token::Type::NONE,             {nullptr,               nullptr,                    ExprPrecedence::NONE        }},
            {Token::Type::IDENTIFIER,       {&Parser::factor,       nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::NUMBER_LITERAL,   {&Parser::factor,       nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::BOOL_LITERAL,     {&Parser::factor,       nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::NULL_LITERAL,     {&Parser::factor,       nullptr,                    ExprPrecedence::FACTOR      }},

            {Token::Type::ADD,              {&Parser::sign,         &Parser::binary_op_left,    ExprPrecedence::ADD_EXPR    }},
            {Token::Type::SUB,              {&Parser::sign,         &Parser::binary_op_left,    ExprPrecedence::ADD_EXPR    }},
            {Token::Type::MUL,              {nullptr,               &Parser::binary_op_left,    ExprPrecedence::MUL_EXPR    }},
            {Token::Type::DIV,              {nullptr,               &Parser::binary_op_left,    ExprPrecedence::MUL_EXPR    }},
            {Token::Type::POW,              {nullptr,               &Parser::binary_op_right,   ExprPrecedence::POWER       }},
            
            {Token::Type::LESS_THAN,        {nullptr,               &Parser::binary_op_left,    ExprPrecedence::COMPARISON  }},
            {Token::Type::GREATER_THAN,     {nullptr,               &Parser::binary_op_left,    ExprPrecedence::COMPARISON  }},
            {Token::Type::LESS_EQUAL,       {nullptr,               &Parser::binary_op_left,    ExprPrecedence::COMPARISON  }},
            {Token::Type::GREATER_EQUAL,    {nullptr,               &Parser::binary_op_left,    ExprPrecedence::COMPARISON  }},
            {Token::Type::EQUAL,            {nullptr,               &Parser::binary_op_left,    ExprPrecedence::COMPARISON  }},
            {Token::Type::NOT_EQUAL,        {nullptr,               &Parser::binary_op_left,    ExprPrecedence::COMPARISON  }},

            {Token::Type::LOGIC_OR,         {nullptr,               &Parser::binary_op_left,    ExprPrecedence::LOGIC_OR    }},
            {Token::Type::LOGIC_AND,        {nullptr,               &Parser::binary_op_left,    ExprPrecedence::LOGIC_AND   }},
            {Token::Type::LOGIC_NOT,        {&Parser::logic_not,    nullptr,                    ExprPrecedence::LOGIC_NOT   }},
            
            {Token::Type::TK_EOF,           {nullptr,               nullptr,                    ExprPrecedence::NONE        }},
        };

        return rules[type];
    }
} // namespace sourdo

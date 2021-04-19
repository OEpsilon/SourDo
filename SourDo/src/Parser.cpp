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
            std::stringstream ss;
            ss << current_token.position << "Expected an operator";
            return {nullptr, ss.str()};
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
        Position saved_position = current_token.position;
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
        return std::make_shared<StatementListNode>(statements, saved_position);
    }

    std::shared_ptr<Node> Parser::statement()
    {
        if(current_token == Token(Token::Type::KEYWORD, "var"))
        {
            Position saved_position = current_token.position;
            advance();
            if(current_token.type != Token::Type::IDENTIFIER)
            {
                std::stringstream ss;
                ss << current_token.position << "Expected an identifier";
                error = ss.str();
                return nullptr;
            }
            Token name_tok = current_token;
            advance();
            if(current_token.type != Token::Type::ASSIGN)
            {
                return std::make_shared<VarDeclarationNode>(name_tok, nullptr, saved_position);
            }
            advance();
            std::shared_ptr<ExpressionNode> expr = expression(ExprPrecedence::LOGIC_OR);
            if(error)
            {
                return nullptr;
            }
            return std::make_shared<VarDeclarationNode>(name_tok, expr, saved_position);
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
            Position saved_position = current_token.position;
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
                    std::stringstream ss;
                    ss << current_token.position << "Expected 'then'";
                    error = ss.str();
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
                std::stringstream ss;
                ss << current_token.position << "Expected 'end'";
                error = ss.str();
                return nullptr;
            }
            advance();


            return std::make_shared<IfNode>(cases, else_case, saved_position);
        }

        return expression(ExprPrecedence::LOGIC_OR);
    }

    std::shared_ptr<ExpressionNode> Parser::expression(ExprPrecedence precedence, bool multiline_mode)
    {
        if(current_token.type == Token::Type::LPAREN)
        {
            advance();

            std::shared_ptr<ExpressionNode> expr = expression(ExprPrecedence::LOGIC_OR, true);
            if(error)
            {
                return nullptr;
            }
            if(current_token.type != Token::Type::RPAREN)
            {
                std::stringstream ss;
                ss << current_token.position << "Expected a ')'";
                error = ss.str();
                return nullptr;
            }
            advance();
            return expr;
        }

        #define IGNORE_NEW_LINE_IF_MULTILINE()                              \
        if(multiline_mode)                                                  \
        {                                                                   \
            while(current_token.type == Token::Type::NEW_LINE)              \
            {                                                               \
                advance();                                                  \
            }                                                               \
        }

        IGNORE_NEW_LINE_IF_MULTILINE();

        ParseExprFunc prefix_func = get_rule(current_token.type).prefix;
        if(prefix_func == nullptr)
        {
            std::stringstream ss;
            ss << current_token.position << "Expected an expression";
            error = ss.str();
            return nullptr;
        }
        std::shared_ptr<ExpressionNode> previous_operand = (this->*prefix_func)(nullptr, multiline_mode);
        if(error)
        {
            return nullptr;
        }

        IGNORE_NEW_LINE_IF_MULTILINE();

        if(get_rule(current_token.type).infix == nullptr)
        {
            return previous_operand;
        }

        while(precedence <= get_rule(current_token.type).precedence)
        {
            IGNORE_NEW_LINE_IF_MULTILINE();

            ParseExprFunc infix_func = get_rule(current_token.type).infix;
            previous_operand = (this->*infix_func)(previous_operand, multiline_mode);
            if(error)
            {
                return nullptr;
            }
        }
        IGNORE_NEW_LINE_IF_MULTILINE();
        #undef IGNORE_NEW_LINE_IF_MULTILINE

        return previous_operand;
    }

    std::shared_ptr<ExpressionNode> Parser::binary_op_left(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        Token op_token = current_token;
        advance();
        ExprPrecedence precedence = static_cast<ExprPrecedence>(
                static_cast<int>(get_rule(op_token.type).precedence) + 1);
        std::shared_ptr<ExpressionNode> right = expression(precedence, multiline_mode);
        return std::make_shared<BinaryOpNode>(previous, op_token, right);
    }

    std::shared_ptr<ExpressionNode> Parser::binary_op_right(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        Token op_token = current_token;
        advance();
        ExprPrecedence precedence = static_cast<ExprPrecedence>(
                static_cast<int>(get_rule(op_token.type).precedence));
        std::shared_ptr<ExpressionNode> right = expression(precedence, multiline_mode);
        return std::make_shared<BinaryOpNode>(previous, op_token, right);
    }

    std::shared_ptr<ExpressionNode> Parser::sign(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        Token op_token = current_token;
        advance();
        std::shared_ptr<ExpressionNode> operand = expression(ExprPrecedence::POWER, multiline_mode);
        return std::make_shared<UnaryOpNode>(op_token, operand);
    }

    std::shared_ptr<ExpressionNode> Parser::logic_not(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        Token op_token = current_token;
        advance();
        std::shared_ptr<ExpressionNode> operand = expression(ExprPrecedence::COMPARISON, multiline_mode);
        return std::make_shared<UnaryOpNode>(op_token, operand);
    }

    std::shared_ptr<ExpressionNode> Parser::factor(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
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
            return std::make_shared<NullValueNode>(tok);
        }
        std::stringstream ss;
        ss << current_token.position << "Expected an expression";
        error = ss.str();
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

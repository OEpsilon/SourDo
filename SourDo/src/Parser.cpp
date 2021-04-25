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
            std::shared_ptr<ExpressionNode> expr = expression();
            if(error)
            {
                return nullptr;
            }
            return std::make_shared<VarDeclarationNode>(name_tok, expr, saved_position);
        }
        else if(current_token == Token(Token::Type::KEYWORD, "func"))
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
            Token name = current_token;
            advance();

            if(current_token.type != Token::Type::LPAREN)
            {
                std::stringstream ss;
                ss << current_token.position << "Expected a '('";
                error = ss.str();
                return nullptr;
            }
            advance();
            std::vector<Token> paramaters;
            if(current_token.type != Token::Type::RPAREN)
            {
                if(current_token.type != Token::Type::IDENTIFIER)
                {
                    std::stringstream ss;
                    ss << current_token.position << "Expected an identifier";
                    error = ss.str();
                    return nullptr;
                }
                paramaters.push_back(current_token);
                advance();

                while(current_token.type == Token::Type::COMMA)
                {
                    advance();
                    if(current_token.type != Token::Type::IDENTIFIER)
                    {
                        std::stringstream ss;
                        ss << current_token.position << "Expected an identifier";
                        error = ss.str();
                        return nullptr;
                    }
                    paramaters.push_back(current_token);
                    advance();
                }
                
                if(current_token.type != Token::Type::RPAREN)
                {
                    std::stringstream ss;
                    ss << current_token.position << "Expected a ')'";
                    error = ss.str();
                    return nullptr;
                }
            }
            advance();

            std::shared_ptr<StatementListNode> statements = statement_list();
            if(error)
            {
                return nullptr;
            }
            if(current_token != Token(Token::Type::KEYWORD, "end"))
            {
                std::stringstream ss;
                ss << current_token.position << "Expected 'end'";
                error = ss.str();
                return nullptr;
            }
            advance();
            return std::make_shared<FuncDeclarationNode>(name, paramaters, statements, saved_position);
        }
        else if(current_token == Token(Token::Type::KEYWORD, "return"))
        {
            Position saved_position = current_token.position;
            advance();
            std::shared_ptr<ExpressionNode> return_value = expression();
            if(error)
            {
                return nullptr;
            }
            return std::make_shared<ReturnNode>(return_value, saved_position);
        }
        else if(current_token == Token(Token::Type::KEYWORD, "if"))
        {
            Position saved_position = current_token.position;
            std::vector<IfNode::IfBlock> cases;
            std::shared_ptr<StatementListNode> else_case;
            advance();
            
            auto if_expr_case = [&]() -> void
            {
                std::shared_ptr<ExpressionNode> condition = expression();
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
        else if(current_token.type == Token::Type::IDENTIFIER)
        {
            auto do_assignment = [&](VarAssignmentNode::Operation op) -> std::shared_ptr<VarAssignmentNode>
            {
                Token name_tok = current_token;
                advance();
                Position saved_position = current_token.position;
                advance();
                std::shared_ptr<ExpressionNode> new_value = expression();
                if(error)
                {
                    return nullptr;
                }
                return std::make_shared<VarAssignmentNode>(name_tok, op, new_value, saved_position);
            };

            switch(tokens[position + 1].type)
            {
                case Token::Type::ASSIGN:
                {
                    return do_assignment(VarAssignmentNode::Operation::NONE);
                    break;
                }
                case Token::Type::ASSIGN_ADD:
                {
                    return do_assignment(VarAssignmentNode::Operation::ADD);
                    break;
                }
                case Token::Type::ASSIGN_SUB:
                {
                    return do_assignment(VarAssignmentNode::Operation::SUB);
                    break;
                }
                case Token::Type::ASSIGN_MUL:
                {
                    return do_assignment(VarAssignmentNode::Operation::MUL);
                    break;
                }
                case Token::Type::ASSIGN_DIV:
                {
                    return do_assignment(VarAssignmentNode::Operation::DIV);
                    break;
                }
                default:
                    break;
            }
            // If the next token is not an assignment, move on to expressions.
        }
        return expression();
    }

    std::shared_ptr<ExpressionNode> Parser::expression(bool multiline_mode)
    {
        return expression_with_precedence(ExprPrecedence::LOGIC_OR, multiline_mode);
    }

    std::shared_ptr<ExpressionNode> Parser::expression_with_precedence(ExprPrecedence precedence, bool multiline_mode)
    {
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

    std::shared_ptr<ExpressionNode> Parser::grouping(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        advance();
        std::shared_ptr<ExpressionNode> grouped = expression(true);
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
        return grouped;
    }

    std::shared_ptr<ExpressionNode> Parser::binary_op_left(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        Token op_token = current_token;
        advance();
        ExprPrecedence precedence = static_cast<ExprPrecedence>(
                static_cast<int>(get_rule(op_token.type).precedence) + 1);
        std::shared_ptr<ExpressionNode> right = expression_with_precedence(precedence, multiline_mode);
        return std::make_shared<BinaryOpNode>(previous, op_token, right);
    }

    std::shared_ptr<ExpressionNode> Parser::binary_op_right(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        Token op_token = current_token;
        advance();
        ExprPrecedence precedence = static_cast<ExprPrecedence>(
                static_cast<int>(get_rule(op_token.type).precedence));
        std::shared_ptr<ExpressionNode> right = expression_with_precedence(precedence, multiline_mode);
        return std::make_shared<BinaryOpNode>(previous, op_token, right);
    }

    std::shared_ptr<ExpressionNode> Parser::unary_op(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        Token op_token = current_token;
        advance();
        std::shared_ptr<ExpressionNode> operand;
        
        switch(op_token.type)
        {
            case Token::Type::ADD:
            case Token::Type::SUB:
            {
                operand = expression_with_precedence(ExprPrecedence::SIGN, multiline_mode);
                break;
            }
            case Token::Type::LOGIC_NOT:
            {
                operand = expression_with_precedence(ExprPrecedence::LOGIC_NOT, multiline_mode);
                break;
            }
            default:
            {
                return nullptr;
            }
        }
        return std::make_shared<UnaryOpNode>(op_token, operand);
    }

    std::shared_ptr<ExpressionNode> Parser::call(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        advance();
        std::vector<std::shared_ptr<ExpressionNode>> arguments;
        if(current_token.type != Token::Type::RPAREN)
        {
            std::shared_ptr<ExpressionNode> arg = expression();
            if(error)
            {
                return nullptr;
            }
            arguments.push_back(arg);

            while(current_token.type == Token::Type::COMMA)
            {
                advance();
                std::shared_ptr<ExpressionNode> arg = expression();
                if(error)
                {
                    return nullptr;
                }
                arguments.push_back(arg);
            }

            if(current_token.type != Token::Type::RPAREN)
            {
                std::stringstream ss;
                ss << current_token.position << "Expected a ')'";
                error = ss.str();
                return nullptr;
            }
        }
        advance();
        return std::make_shared<CallNode>(previous, arguments);
    }

    std::shared_ptr<ExpressionNode> Parser::factor(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        Token tok = current_token;
        switch(tok.type)
        {
            case Token::Type::NUMBER_LITERAL:
            {
                advance();
                return std::make_shared<NumberNode>(tok);
                break;
            }
            case Token::Type::STRING_LITERAL:
            {
                advance();
                return std::make_shared<StringNode>(tok);
                break;
            }
            case Token::Type::BOOL_LITERAL:
            {
                advance();
                return std::make_shared<BoolNode>(tok);
                break;
            }
            case Token::Type::NULL_LITERAL:
            {
                advance();
                return std::make_shared<NullNode>(tok);
                break;
            }
            case Token::Type::IDENTIFIER:
            {
                advance();
                return std::make_shared<IdentifierNode>(tok);
                break;
            }
            default:
                break;
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
            {Token::Type::NUMBER_LITERAL,   {&Parser::factor,       nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::STRING_LITERAL,   {&Parser::factor,       nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::BOOL_LITERAL,     {&Parser::factor,       nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::NULL_LITERAL,     {&Parser::factor,       nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::IDENTIFIER,       {&Parser::factor,       nullptr,                    ExprPrecedence::FACTOR      }},

            {Token::Type::ADD,              {&Parser::unary_op,     &Parser::binary_op_left,    ExprPrecedence::ADD_EXPR    }},
            {Token::Type::SUB,              {&Parser::unary_op,     &Parser::binary_op_left,    ExprPrecedence::ADD_EXPR    }},
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
            {Token::Type::LOGIC_NOT,        {&Parser::unary_op,     nullptr,                    ExprPrecedence::NONE        }},

            {Token::Type::LPAREN,           {&Parser::grouping,     &Parser::call,              ExprPrecedence::CALL        }},
            
            {Token::Type::TK_EOF,           {nullptr,               nullptr,                    ExprPrecedence::NONE        }},
        };

        return rules[type];
    }
} // namespace sourdo

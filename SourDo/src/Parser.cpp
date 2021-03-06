#include "Interpreter.hpp"

#include <unordered_map>
#include <sstream>
#include <map>

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

    std::vector<std::shared_ptr<ExpressionNode>> Parser::get_arguments()
    {
        std::vector<std::shared_ptr<ExpressionNode>> arguments;
        if(current_token.type != Token::Type::RPAREN)
        {
            std::shared_ptr<ExpressionNode> arg = expression(true);
            if(error)
            {
                return {};
            }
            arguments.push_back(arg);

            while(current_token.type == Token::Type::COMMA)
            {
                advance();
                std::shared_ptr<ExpressionNode> arg = expression(true);
                if(error)
                {
                    return {};
                }
                arguments.push_back(arg);
            }

            if(current_token.type != Token::Type::RPAREN)
            {
                std::stringstream ss;
                ss << current_token.position << "Expected a ')'";
                error = ss.str();
                return {};
            }
        }
        advance();
        return arguments;
    }

    std::vector<std::string> Parser::get_parameters(std::vector<std::string>& parameters)
    {
        if(current_token.type != Token::Type::LPAREN)
        {
            std::stringstream ss;
            ss << current_token.position << "Expected a '('";
            error = ss.str();
            return {};
        }
        advance();
        if(current_token.type != Token::Type::RPAREN)
        {
            if(current_token.type != Token::Type::IDENTIFIER)
            {
                std::stringstream ss;
                ss << current_token.position << "Expected an identifier";
                error = ss.str();
                return {};
            }
            parameters.push_back(current_token.value);
            advance();

            while(current_token.type == Token::Type::COMMA)
            {
                advance();
                if(current_token.type != Token::Type::IDENTIFIER)
                {
                    std::stringstream ss;
                    ss << current_token.position << "Expected an identifier";
                    error = ss.str();
                    return {};
                }
                parameters.push_back(current_token.value);
                advance();
            }
            
            if(current_token.type != Token::Type::RPAREN)
            {
                std::stringstream ss;
                ss << current_token.position << "Expected a ')'";
                error = ss.str();
                return {};
            }
        }
        advance();
        return parameters;
    }

    std::shared_ptr<StatementListNode> Parser::statement_list()
    {
        Position saved_position = current_token.position;
        std::vector<std::shared_ptr<Node>> statements;

        while(current_token.type == Token::Type::NEW_LINE)
        {
            advance();
        }

        while(current_token.type != Token::Type::END &&
                current_token.type != Token::Type::ELIF &&
            current_token.type != Token::Type::ELSE &&
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

    std::shared_ptr<ClassNode> Parser::class_def()
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
        Token class_name = current_token;
        std::optional<Token> super_name;
        advance();
        if(current_token.type == Token::Type::EXTENDS)
        {
            advance();
            if(current_token.type != Token::Type::IDENTIFIER)
            {
                std::stringstream ss;
                ss << current_token.position << "Expected an identifier";
                error = ss.str();
                return nullptr;
            }
            super_name = current_token;
            advance();
        }

        while(current_token.type == Token::Type::NEW_LINE)
        {
            advance();
        }

        std::unordered_map<std::string, ClassNode::Property> properties;
        bool has_new_func = false;
        std::unordered_map<std::string, ClassNode::Property> methods;
        std::unordered_map<std::string, ClassNode::Setter> setters;
        std::unordered_map<std::string, ClassNode::Getter> getters;

        while(current_token.type != Token::Type::END && 
                current_token.type != Token::Type::TK_EOF)
        {
            bool is_private = false;
            if(current_token.type == Token::Type::PRIVATE)
            {
                is_private = true;
                advance();
            }

            #define IDENTIFIER_CHECK(identifier_name)                       \
            if(current_token.type != Token::Type::IDENTIFIER)               \
            {                                                               \
                std::stringstream ss;                                       \
                ss << current_token.position << "Expected an identifier";   \
                error = ss.str();                                           \
                return nullptr;                                             \
            }                                                               \
            Token identifier_name = current_token;                          \
            advance()
            
            #define CHECK_IDENTIFIER_EXISTS(identifier_name)                \
            if(properties.find(identifier_name) != properties.end()         \
                    || methods.find(identifier_name) != methods.end()       \
                    || setters.find(identifier_name) != setters.end()       \
                    || getters.find(identifier_name) != getters.end())      \
            {                                                               \
                std::stringstream ss;                                       \
                ss << current_token.position << "'" << identifier_name      \
                        << "' is already defined in class '"                \
                        << class_name.value << "'";                         \
                error = ss.str();                                           \
                return nullptr;                                             \
            }

            switch(current_token.type)
            {
                case Token::Type::CONST:
                case Token::Type::VAR:
                {
                    bool readonly = current_token.type == Token::Type::CONST;
                    Position saved_position = current_token.position;
                    advance();
                    IDENTIFIER_CHECK(name_tok);
                    if(name_tok.value == "new")
                    {
                        std::stringstream ss;
                        ss << current_token.position << "'new' must be declared as a method";
                        error = ss.str();
                        return nullptr;
                    }

                    if(current_token.type != Token::Type::ASSIGN)
                    {
                        if(readonly)
                        {
                            std::stringstream ss;
                            ss << current_token.position << "A property declared constant must be assigned a value when declared";
                            error = ss.str();
                            return nullptr;
                        }
                        properties[name_tok.value] = ClassNode::Property(nullptr, false, is_private);
                        break;
                    }
                    advance();
                    std::shared_ptr<ExpressionNode> expr = expression();
                    if(error)
                    {
                        return nullptr;
                    }
                    properties[name_tok.value] = ClassNode::Property(nullptr, readonly, is_private);
                    break;
                }
                case Token::Type::FUNC:
                {
                    Position saved_position = current_token.position;

                    std::vector<std::string> parameters;
                    std::shared_ptr<StatementListNode> statements;
                    
                    advance();
                    IDENTIFIER_CHECK(name);
                    CHECK_IDENTIFIER_EXISTS(name.value);
                    has_new_func = has_new_func || name.value == "new";
                    if(name.value == "new" && is_private)
                    {
                        std::stringstream ss;
                        ss << saved_position << "The 'new' method cannot be declared private";
                        error = ss.str();
                        return nullptr;
                    }
                    
                    get_parameters(parameters);
                    if(error)
                    {
                        return nullptr;
                    }
                    if(parameters.size() == 0)
                    {
                        std::stringstream ss;
                        ss << saved_position << "Function declared in a class must have at least 1 parameter";
                        error = ss.str();
                        return nullptr;
                    }

                    statements = statement_list();
                    if(error)
                    {
                        return nullptr;
                    }
                    if(current_token.type != Token::Type::END)
                    {
                        std::stringstream ss;
                        ss << current_token.position << "Expected 'end'";
                        error = ss.str();
                        return nullptr;
                    }
                    advance();
                    
                    methods[name.value] = ClassNode::Property(std::make_shared<FuncNode>(parameters, statements, saved_position), true, is_private);
                    break;
                }
                case Token::Type::SET:
                {
                    Position saved_position = current_token.position;
                    advance();

                    IDENTIFIER_CHECK(name);
                    if(properties.find(name.value) != properties.end()
                            || methods.find(name.value) != methods.end()
                            || setters.find(name.value) != setters.end())
                    {
                        std::stringstream ss;
                        ss << current_token.position << "'" << name.value
                                << "' is already defined in class '"
                                << class_name.value << "'";
                        error = ss.str();
                        return nullptr;
                    }

                    if(name.value == "new")
                    {
                        std::stringstream ss;
                        ss << current_token.position << "'new' must be declared as a method";
                        error = ss.str();
                        return nullptr;
                    }
                    
                    if(current_token.type != Token::Type::LPAREN)
                    {
                        std::stringstream ss;
                        ss << current_token.position << "Expected a '('";
                        error = ss.str();
                        return nullptr;
                    }
                    advance();

                    IDENTIFIER_CHECK(self_name);

                    if(current_token.type != Token::Type::COMMA)
                    {
                        std::stringstream ss;
                        ss << current_token.position << "Expected a ','";
                        error = ss.str();
                        return nullptr;
                    }
                    advance();

                    IDENTIFIER_CHECK(new_value_name);

                    if(current_token.type != Token::Type::RPAREN)
                    {
                        std::stringstream ss;
                        ss << current_token.position << "Expected a ')'";
                        error = ss.str();
                        return nullptr;
                    }
                    advance();

                    std::shared_ptr<StatementListNode> statements = statement_list();
                    if(error)
                    {
                        return nullptr;
                    }

                    if(current_token.type != Token::Type::END)
                    {
                        std::stringstream ss;
                        ss << current_token.position << "Expected 'end'";
                        error = ss.str();
                        return nullptr;
                    }
                    advance();

                    setters[name.value] = ClassNode::Setter(self_name, new_value_name, statements, is_private, saved_position);
                    break;
                }
                case Token::Type::GET:
                {
                    Position saved_position = current_token.position;
                    advance();

                    IDENTIFIER_CHECK(name);
                    if(properties.find(name.value) != properties.end()
                            || methods.find(name.value) != methods.end()
                            || getters.find(name.value) != getters.end())
                    {
                        std::stringstream ss;
                        ss << current_token.position << "'" << name.value
                                << "' is already defined in class '"
                                << class_name.value << "'";
                        error = ss.str();
                        return nullptr;
                    }

                    if(name.value == "new")
                    {
                        std::stringstream ss;
                        ss << current_token.position << "'new' must be declared as a method";
                        error = ss.str();
                        return nullptr;
                    }

                    if(current_token.type != Token::Type::LPAREN)
                    {
                        std::stringstream ss;
                        ss << current_token.position << "Expected a '('";
                        error = ss.str();
                        return nullptr;
                    }
                    advance();

                    IDENTIFIER_CHECK(self_name);

                    if(current_token.type != Token::Type::RPAREN)
                    {
                        std::stringstream ss;
                        ss << current_token.position << "Expected a ')'";
                        error = ss.str();
                        return nullptr;
                    }
                    advance();

                    std::shared_ptr<StatementListNode> statements = statement_list();
                    if(error)
                    {
                        return nullptr;
                    }

                    if(current_token.type != Token::Type::END)
                    {
                        std::stringstream ss;
                        ss << current_token.position << "Expected 'end'";
                        error = ss.str();
                        return nullptr;
                    }
                    advance();

                    getters[name.value] = ClassNode::Getter(self_name, statements, is_private, saved_position);
                    #undef IDENTIFIER_CHECK
                    #undef CHECK_IDENTIFIER_EXISTS
                    break;
                }
                default:
                    if(is_private)
                    {
                        std::stringstream ss;
                        ss << current_token.position << "Expected a declaration";
                        error = ss.str();
                        return nullptr;
                    }
                    break;
            }

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

        if(!has_new_func)
        {
            methods["new"] = ClassNode::Property(std::make_shared<FuncNode>(std::vector<std::string>(), 
                    std::make_shared<StatementListNode>(
                        std::vector<std::shared_ptr<Node>>(), Position()
                    ),
                    Position()
                ),
                true,
                false
            );
        }

        if(current_token.type != Token::Type::END)
        {
            std::stringstream ss;
            ss << current_token.position << "Expected 'end'";
            error = ss.str();
            return nullptr;
        }
        advance();

        return std::make_shared<ClassNode>(class_name, super_name, properties, methods, setters, getters, saved_position);
    }

    std::shared_ptr<Node> Parser::statement()
    {
        switch(current_token.type)
        {
            case Token::Type::VAR:
            case Token::Type::CONST:
            {
                return var_declaration();
                break;
            }
            case Token::Type::CLASS:
            {
                return class_def();
                break;
            }
            case Token::Type::FUNC:
            {
                // Don't give an error yet as this may be a lambda function and thus have no name.
                if(tokens[position + 1].type != Token::Type::IDENTIFIER)
                {
                    return nullptr;
                }
                return func_declaration();
            }
            case Token::Type::RETURN:
            {
                Position saved_position = current_token.position;
                advance();
                std::shared_ptr<ExpressionNode> return_value = expression();
                if(error)
                {
                    return nullptr;
                }
                return std::make_shared<ReturnNode>(return_value, saved_position);
                break;
            }
            case Token::Type::BREAK:
            {
                Position saved_position = current_token.position;
                advance();
                return std::make_shared<BreakNode>(saved_position);
                break;
            }
            case Token::Type::CONTINUE:
            {
                Position saved_position = current_token.position;
                advance();
                return std::make_shared<ContinueNode>(saved_position);
                break;
            }
            case Token::Type::IF:
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
                    
                    if(current_token.type != Token::Type::THEN)
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

                while(current_token.type == Token::Type::ELIF)
                {
                    advance();

                    if_expr_case();
                    if(error)
                    {
                        return nullptr;
                    }
                }

                if(current_token.type == Token::Type::ELSE)
                {
                    advance();

                    else_case = statement_list();
                    if(error)
                    {
                        return nullptr;
                    }
                }

                if(current_token.type != Token::Type::END)
                {
                    std::stringstream ss;
                    ss << current_token.position << "Expected 'end'";
                    error = ss.str();
                    return nullptr;
                }
                advance();

                return std::make_shared<IfNode>(cases, else_case, saved_position);
                break;
            }
            case Token::Type::FOR:
            {
                Position saved_position = current_token.position;
                advance();

                std::shared_ptr<Node> initializer;
                if(current_token.type == Token::Type::VAR)
                {
                    initializer = var_declaration();
                }
                else
                {
                    initializer = expression(true, true);
                }
                
                if(error)
                {
                    return nullptr;
                }

                if(current_token.type != Token::Type::COMMA)
                {
                    std::stringstream ss;
                    ss << current_token << "Expected a ','";
                    error = ss.str();
                    return nullptr;
                }
                advance();
                std::shared_ptr<ExpressionNode> condition = expression(true);

                if(current_token.type != Token::Type::COMMA)
                {
                    std::stringstream ss;
                    ss << current_token << "Expected a ','";
                    error = ss.str();
                    return nullptr;
                }
                advance();
                
                std::shared_ptr<Node> increment = expression(true, true);
                if(error)
                {
                    return nullptr;
                }
                if(current_token.type != Token::Type::DO)
                {
                    std::stringstream ss;
                    ss << current_token.position << "Expected 'do'";
                    error = ss.str();
                }
                advance();

                std::shared_ptr<StatementListNode> statements = statement_list();
                if(error)
                {
                    return nullptr;
                }
                if(current_token.type != Token::Type::END)
                {
                    std::stringstream ss;
                    ss << current_token.position << "Expected 'end'";
                    error = ss.str();
                }
                advance();
                return std::make_shared<ForNode>(initializer, condition, increment, statements, saved_position);
            }
            case Token::Type::WHILE:
            {
                Position saved_position = current_token.position;
                advance();

                std::shared_ptr<ExpressionNode> condition = expression(true);
                if(error)
                {
                    return nullptr;
                }

                if(current_token.type != Token::Type::DO)
                {
                    std::stringstream ss;
                    ss << current_token << "Expected 'do'";
                    error = ss.str();
                    return nullptr;
                }
                advance();
                std::shared_ptr<StatementListNode> statements = statement_list();
                if(error)
                {
                    return nullptr;
                }

                if(current_token.type != Token::Type::END)
                {
                    std::stringstream ss;
                    ss << current_token.position << "Expected 'end'";
                    error = ss.str();
                    return nullptr;
                }
                advance();
                return std::make_shared<WhileNode>(condition, statements, saved_position);
            }
            case Token::Type::LOOP:
            {
                Position saved_position = current_token.position;
                advance();

                std::shared_ptr<StatementListNode> statements = statement_list();
                if(error)
                {
                    return nullptr;
                }

                if(current_token.type != Token::Type::END)
                {
                    std::stringstream ss;
                    ss << current_token.position << "Expected 'end'";
                    error = ss.str();
                    return nullptr;
                }
                advance();
                return std::make_shared<LoopNode>(statements, saved_position);
            }
            default:
            {
                break;
            }
        }
        return expression(false, true);
    }

    std::shared_ptr<VarDeclarationNode> Parser::var_declaration()
    {
        Position saved_position = current_token.position;
        bool readonly = current_token.type == Token::Type::CONST;
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
            if(readonly)
            {
                std::stringstream ss;
                ss << current_token.position << "A variable declared constant must be assigned a value when declared";
                error = ss.str();
                return nullptr;
            }
            return std::make_shared<VarDeclarationNode>(name_tok, nullptr, false, saved_position);
        }
        advance();
        std::shared_ptr<ExpressionNode> expr = expression();
        if(error)
        {
            return nullptr;
        }
        return std::make_shared<VarDeclarationNode>(name_tok, expr, readonly, saved_position);
    }

    std::shared_ptr<Node> Parser::func_declaration()
    {
        Position saved_position = current_token.position;

        std::vector<std::string> parameters;
        std::shared_ptr<StatementListNode> statements;
        
        advance();
        Token name = current_token;
        advance();
        
        get_parameters(parameters);
        if(error)
        {
            return nullptr;
        }

        statements = statement_list();
        if(error)
        {
            return nullptr;
        }
        if(current_token.type != Token::Type::END)
        {
            std::stringstream ss;
            ss << current_token.position << "Expected 'end'";
            error = ss.str();
            return nullptr;
        }
        advance();
        
        return std::make_shared<VarDeclarationNode>(name, std::make_shared<FuncNode>(parameters, statements, saved_position), true, saved_position);
    }

    std::shared_ptr<ExpressionNode> Parser::expression(bool multiline_mode, bool allow_assignment)
    {
        return expression_with_precedence(allow_assignment ? ExprPrecedence::ASSIGNMENT : ExprPrecedence::LOGIC_OR, multiline_mode);
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

    std::shared_ptr<ExpressionNode> Parser::assignment(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        if(previous->type != Node::Type::IDENTIFIER_NODE && previous->type != Node::Type::INDEX_NODE)
        {
            std::stringstream ss;
            ss << current_token.position << "Cannot assign to an expression";
            error = ss.str();
            return nullptr;
        }

        auto do_assignment = [&](AssignmentNode::Operation op) -> std::shared_ptr<ExpressionNode>
        {
            Position saved_position = current_token.position;
            advance();
            std::shared_ptr<ExpressionNode> new_value = expression(false, false);
            if(error)
            {
                return nullptr;
            }
            return std::make_shared<AssignmentNode>(previous, op, new_value, saved_position);
        };

        switch(current_token.type)
        {
            case Token::Type::ASSIGN:
                return do_assignment(AssignmentNode::Operation::NONE);
                break;
            case Token::Type::ASSIGN_ADD:
                return do_assignment(AssignmentNode::Operation::ADD);
                break;
            case Token::Type::ASSIGN_SUB:
                return do_assignment(AssignmentNode::Operation::SUB);
                break;
            case Token::Type::ASSIGN_MUL:
                return do_assignment(AssignmentNode::Operation::MUL);
                break;
            case Token::Type::ASSIGN_DIV:
                return do_assignment(AssignmentNode::Operation::DIV);
                break;
            default:
                break;
        }
        return nullptr;
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
            case Token::Type::NOT:
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

    std::shared_ptr<ExpressionNode> Parser::is_expr(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        Position saved_position = current_token.position;
        advance();
        bool invert = false;
        if(current_token.type == Token::Type::NOT)
        {
            invert = true;
            advance();
        }

        if(current_token.type != Token::Type::IDENTIFIER
            && current_token.type != Token::Type::BUILTIN_TYPE)
        {
            std::stringstream ss;
            ss << current_token.position << "Expected an identifier";
            error = ss.str();
            return nullptr;
        }
        Token right_operand = current_token;
        advance();

        return std::make_shared<IsNode>(previous, invert, right_operand, saved_position);
    }

    std::shared_ptr<ExpressionNode> Parser::call(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        advance();
        std::vector<std::shared_ptr<ExpressionNode>> arguments = get_arguments();
        if(error)
        {
            return nullptr;
        }
        return std::make_shared<CallNode>(previous, arguments);
    }

    std::shared_ptr<ExpressionNode> Parser::index(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        Token tok = current_token.type;

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
        if(tok.type == Token::Type::COLON)
        {
            if(current_token.type != Token::Type::LPAREN)
            {
                std::stringstream ss;
                ss << current_token.position << "Member that was accessed with ':' must be called";
                error = ss.str();
                return nullptr;
            }
            advance();
            std::vector<std::shared_ptr<ExpressionNode>> arguments = get_arguments();
            if(error)
            {
                return nullptr;
            }
            return std::make_shared<IndexCallNode>(previous, std::make_shared<StringNode>(name_tok), arguments);
        }
        return std::make_shared<IndexNode>(previous, std::make_shared<StringNode>(name_tok));
    }

    std::shared_ptr<ExpressionNode> Parser::subscript(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        advance();
        std::shared_ptr<ExpressionNode> expr = expression(true);
        if(error)
        {
            return nullptr;
        }
        if(current_token.type != Token::Type::RBRACKET)
        {
            std::stringstream ss;
            ss << current_token.position << "Expected a ']'";
            error = ss.str();
            return nullptr;
        }
        advance();
        return std::make_shared<IndexNode>(previous, expr);
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
            case Token::Type::BOOL_TRUE:
            {
                advance();
                return std::make_shared<BoolNode>(tok);
                break;
            }
            case Token::Type::BOOL_FALSE:
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

    std::shared_ptr<ExpressionNode> Parser::lambda_function(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        Position saved_position = current_token.position;
        advance();
        std::vector<std::string> parameters;
        get_parameters(parameters);
        std::shared_ptr<StatementListNode> statements = statement_list();
        if(error)
        {
            return nullptr;
        }
        if(current_token.type != Token::Type::END)
        {
            std::stringstream ss;
            ss << current_token.position << "Expected 'end'";
            error = ss.str();
            return nullptr;
        }
        advance();
        return std::make_shared<FuncNode>(parameters, statements, saved_position);
    }

    std::shared_ptr<ExpressionNode> Parser::object_literal(std::shared_ptr<ExpressionNode> previous, bool multiline_mode)
    {
        Position saved_position;
        std::map<std::shared_ptr<ExpressionNode>, std::shared_ptr<ExpressionNode>> keys;
        do
        {
            advance();
            while(current_token.type == Token::Type::NEW_LINE)
            {
                advance();
            }

            if(current_token.type == Token::Type::RBRACE)
            {
                break;
            }
            std::shared_ptr<ExpressionNode> key = expression();
            if(error)
            {
                return nullptr;
            }
            if(current_token.type != Token::Type::ASSIGN)
            {
                std::stringstream ss;
                ss << current_token.position << "Expected a '='";
                error = ss.str();
                return nullptr;
            }
            advance();
            std::shared_ptr<ExpressionNode> value = expression();
            if(error)
            {
                return nullptr;
            }
            keys[key] = value;
        }
        while(current_token.type == Token::Type::COMMA);

        while(current_token.type == Token::Type::NEW_LINE)
        {
            advance();
        }

        if(current_token.type != Token::Type::RBRACE)
        {
            std::stringstream ss;
            ss << current_token.position << "Expected a '}'";
            error = ss.str();
            return nullptr;
        }
        advance();
        return std::make_shared<TableNode>(std::move(keys), saved_position);
    }

    Parser::ParseExprRule& Parser::get_rule(const Token::Type& type)
    {
        static std::unordered_map<Token::Type, ParseExprRule> rules =
        {
            //                              Prefix                      Infix                       Precedence
            {Token::Type::NONE,             {nullptr,                   nullptr,                    ExprPrecedence::NONE        }},
            {Token::Type::NUMBER_LITERAL,   {&Parser::factor,           nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::STRING_LITERAL,   {&Parser::factor,           nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::BOOL_TRUE,        {&Parser::factor,           nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::BOOL_FALSE,       {&Parser::factor,           nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::NULL_LITERAL,     {&Parser::factor,           nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::IDENTIFIER,       {&Parser::factor,           nullptr,                    ExprPrecedence::FACTOR      }},
            {Token::Type::FUNC,             {&Parser::lambda_function,  nullptr,                    ExprPrecedence::FACTOR      }},

            {Token::Type::ADD,              {&Parser::unary_op,         &Parser::binary_op_left,    ExprPrecedence::ADD_EXPR    }},
            {Token::Type::SUB,              {&Parser::unary_op,         &Parser::binary_op_left,    ExprPrecedence::ADD_EXPR    }},
            {Token::Type::MUL,              {nullptr,                   &Parser::binary_op_left,    ExprPrecedence::MUL_EXPR    }},
            {Token::Type::DIV,              {nullptr,                   &Parser::binary_op_left,    ExprPrecedence::MUL_EXPR    }},
            {Token::Type::MOD,              {nullptr,                   &Parser::binary_op_left,    ExprPrecedence::MUL_EXPR    }},
            {Token::Type::POW,              {nullptr,                   &Parser::binary_op_right,   ExprPrecedence::POWER       }},
            {Token::Type::ISA,              {nullptr,                   &Parser::is_expr,           ExprPrecedence::POWER       }},
            
            {Token::Type::LESS_THAN,        {nullptr,                   &Parser::binary_op_left,    ExprPrecedence::COMPARISON  }},
            {Token::Type::GREATER_THAN,     {nullptr,                   &Parser::binary_op_left,    ExprPrecedence::COMPARISON  }},
            {Token::Type::LESS_EQUAL,       {nullptr,                   &Parser::binary_op_left,    ExprPrecedence::COMPARISON  }},
            {Token::Type::GREATER_EQUAL,    {nullptr,                   &Parser::binary_op_left,    ExprPrecedence::COMPARISON  }},
            {Token::Type::EQUAL,            {nullptr,                   &Parser::binary_op_left,    ExprPrecedence::COMPARISON  }},
            {Token::Type::NOT_EQUAL,        {nullptr,                   &Parser::binary_op_left,    ExprPrecedence::COMPARISON  }},

            {Token::Type::OR,               {nullptr,                   &Parser::binary_op_left,    ExprPrecedence::LOGIC_OR    }},
            {Token::Type::AND,              {nullptr,                   &Parser::binary_op_left,    ExprPrecedence::LOGIC_AND   }},
            {Token::Type::NOT,              {&Parser::unary_op,         nullptr,                    ExprPrecedence::NONE        }},

            {Token::Type::ASSIGN,           {nullptr,                   &Parser::assignment,        ExprPrecedence::ASSIGNMENT  }},
            {Token::Type::ASSIGN_ADD,       {nullptr,                   &Parser::assignment,        ExprPrecedence::ASSIGNMENT  }},
            {Token::Type::ASSIGN_SUB,       {nullptr,                   &Parser::assignment,        ExprPrecedence::ASSIGNMENT  }},
            {Token::Type::ASSIGN_MUL,       {nullptr,                   &Parser::assignment,        ExprPrecedence::ASSIGNMENT  }},
            {Token::Type::ASSIGN_DIV,       {nullptr,                   &Parser::assignment,        ExprPrecedence::ASSIGNMENT  }},

            {Token::Type::LPAREN,           {&Parser::grouping,         &Parser::call,              ExprPrecedence::CALL        }},
            {Token::Type::DOT,              {nullptr,                   &Parser::index,             ExprPrecedence::INDEX       }},
            {Token::Type::COLON,            {nullptr,                   &Parser::index,             ExprPrecedence::INDEX       }},
            {Token::Type::LBRACKET,         {nullptr,                   &Parser::subscript,         ExprPrecedence::SUBSCRIPT   }},
            {Token::Type::LBRACE,           {&Parser::object_literal,   nullptr,                    ExprPrecedence::NONE        }},
            
            {Token::Type::TK_EOF,           {nullptr,               nullptr,                    ExprPrecedence::NONE        }},
        };

        return rules[type];
    }
} // namespace sourdo

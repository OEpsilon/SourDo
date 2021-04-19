#pragma once

#include <vector>
#include <optional>  

#include "Datatypes/Token.hpp"
#include "Datatypes/Nodes.hpp"
#include "Datatypes/Value.hpp"

struct sourdo_Data;

namespace sourdo
{
    struct TokenizerReturn
    {
        std::vector<Token> tokens;
        std::optional<std::string> error_message;
    };

    struct ParserReturn
    {
        std::shared_ptr<Node> ast;
        std::optional<std::string> error_message;
    };

    struct VisitorReturn
    {
        Value result;
        std::optional<std::string> error_message;
    };

    TokenizerReturn tokenize_string(const std::string& string, const std::string& file_name);
    VisitorReturn visit_ast(sourdo_Data* data, std::shared_ptr<Node>);

    class Parser
    {
    public:
        ParserReturn parse_tokens(const std::vector<Token>& tokens);
    private:
        typedef std::shared_ptr<ExpressionNode> (Parser::*ParseExprFunc)(std::shared_ptr<ExpressionNode>, bool);

        enum class ExprPrecedence
        {
            NONE            = 0,
            LOGIC_OR        = 1,
            LOGIC_AND       = 2,
            LOGIC_NOT       = 3,
            COMPARISON      = 4,
            ADD_EXPR        = 5,
            MUL_EXPR        = 6,
            POWER           = 7,
            FACTOR          = 8,
        };

        struct ParseExprRule 
        {
            ParseExprFunc prefix;
            ParseExprFunc infix;
            ExprPrecedence precedence;
        };

        std::optional<std::string> error;
        std::vector<Token> tokens;
        Token current_token;
        uint32_t position;

        Token advance();

        std::shared_ptr<StatementListNode> statement_list();

        std::shared_ptr<Node> statement();

        std::shared_ptr<ExpressionNode> expression(ExprPrecedence precedence, bool multiline_mode = false);

        std::shared_ptr<ExpressionNode> binary_op_left(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);
        std::shared_ptr<ExpressionNode> binary_op_right(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);

        std::shared_ptr<ExpressionNode> sign(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);
        std::shared_ptr<ExpressionNode> logic_not(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);

        std::shared_ptr<ExpressionNode> factor(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);

        ParseExprRule& get_rule(const Token::Type& type);
    };
} // namespace sourdo

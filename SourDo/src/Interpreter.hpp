#pragma once

#include "SourDo/SourDo.hpp"

#include <vector>
#include <optional>  

#include "Datatypes/Token.hpp"
#include "Datatypes/Nodes.hpp"
#include "Datatypes/Value.hpp"

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
        bool is_function_return = false;
        bool is_breaking = false;
        bool is_continuing = false;
        Position break_position;
    };

    TokenizerReturn tokenize_string(const std::string& string, const std::string& file_name);
    VisitorReturn visit_ast(Data::Impl* data, std::shared_ptr<Node> ast);

    class Parser
    {
    public:
        ParserReturn parse_tokens(const std::vector<Token>& tokens);
    private:
        typedef std::shared_ptr<ExpressionNode> (Parser::*ParseExprFunc)(std::shared_ptr<ExpressionNode>, bool);

        enum class ExprPrecedence
        {
            NONE = 0,
            ASSIGNMENT,
            LOGIC_OR,
            LOGIC_AND,
            LOGIC_NOT,
            COMPARISON,
            ADD_EXPR,
            MUL_EXPR,
            SIGN,
            POWER,
            IS_EXPR,
            CALL,
            INDEX,
            SUBSCRIPT,
            FACTOR,
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
        std::vector<std::shared_ptr<ExpressionNode>> get_arguments();
        std::vector<std::string> get_parameters(std::vector<std::string>& parameters);

        std::shared_ptr<StatementListNode> statement_list();

        std::shared_ptr<Node> statement();
        std::shared_ptr<Node> var_declaration();

        std::shared_ptr<ExpressionNode> expression(bool multiline_mode = false, bool allow_assignment = false);
        std::shared_ptr<ExpressionNode> expression_with_precedence(ExprPrecedence precedence, bool multiline_mode = false);

        std::shared_ptr<ExpressionNode> assignment(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);

        std::shared_ptr<ExpressionNode> grouping(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);

        std::shared_ptr<ExpressionNode> binary_op_left(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);
        std::shared_ptr<ExpressionNode> binary_op_right(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);

        std::shared_ptr<ExpressionNode> unary_op(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);

        std::shared_ptr<ExpressionNode> is_expr(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);
        std::shared_ptr<ExpressionNode> call(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);
        std::shared_ptr<ExpressionNode> index(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);
        std::shared_ptr<ExpressionNode> subscript(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);

        std::shared_ptr<ExpressionNode> factor(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);

        std::shared_ptr<ExpressionNode> lambda_function(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);

        std::shared_ptr<ExpressionNode> object_literal(std::shared_ptr<ExpressionNode> previous, bool multiline_mode);

        ParseExprRule& get_rule(const Token::Type& type);
    };
} // namespace sourdo

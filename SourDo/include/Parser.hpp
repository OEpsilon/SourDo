#pragma once

#include "DataTypes/Nodes.hpp"
#include "Errors/Error.hpp"

#include <unordered_map>
#include <vector>

namespace SourDo {
    struct ParseResult
    {
        std::shared_ptr<Node> result;
        std::optional<Error> error;
    };

    class Parser
    {
    public:
        ParseResult parse_tokens(const std::vector<Token>& tokens);
    private:
        typedef std::shared_ptr<ExpressionNode> (Parser::*ParseExprFunc)(std::shared_ptr<ExpressionNode>);

        enum class ExprPrecedence
        {
            None        = 0,
            AddExpr     = 1,
            Factor      = 2,
        };

        struct ParseExprRule 
        {
            ParseExprFunc prefix;
            ParseExprFunc infix;
            ExprPrecedence precedence;
        };

        std::optional<Error> error;
        std::vector<Token> tokens;
        Token current_token;
        uint32_t position;

        Token advance();

        std::shared_ptr<ExpressionNode> expression(ExprPrecedence precedence);

        std::shared_ptr<ExpressionNode> binary_op_left(std::shared_ptr<ExpressionNode> previous);

        std::shared_ptr<ExpressionNode> factor(std::shared_ptr<ExpressionNode> previous);

        ParseExprRule& get_rule(const Token::Type& type);
    };
} // namespace SourDo


#pragma once

#include "Token.hpp"

#include <memory>
#include <sstream>

namespace SourDo {
    class Node 
    {
    public:
        enum class Type
        {
            None = 0,
            BinaryOpNode,
            LiteralNode,
        };
    public:
        Type type = Type::None;
    public:
        virtual std::string to_string() = 0;
    protected:
        Node() = default;
    };

    class ExpressionNode : public Node
    {
    protected:
        ExpressionNode() = default;
    };

    class BinaryOpNode : public ExpressionNode
    {
    public:
        BinaryOpNode(std::shared_ptr<ExpressionNode> left_operand, 
                const Token& op_token, std::shared_ptr<ExpressionNode> right_operand)
            : left_operand(left_operand), op_token(op_token), right_operand(right_operand)
    {
        type = Type::BinaryOpNode;
    }
    public:
        std::shared_ptr<ExpressionNode> left_operand;
        Token op_token;
        std::shared_ptr<ExpressionNode> right_operand;
    public:
        std::string to_string() final
        {
            std::stringstream ss;
            ss << "(" << left_operand->to_string() << ", " << op_token << ", " << right_operand->to_string() << ")";
            return ss.str();
        }
    };

    class LiteralNode : public ExpressionNode
    {
    public:
        LiteralNode(const Token& token)
            : token(token)
        {
            type = Type::LiteralNode;
        }

    public:
        Token token;
    public:
        std::string to_string() final
        {
            std::stringstream ss;
            ss << token;
            return ss.str();
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Node>& node)
    {
        os << node->to_string();
        return os;
    }
} // namespace SourDo

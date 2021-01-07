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
            BINARY_OP_NODE,
            UNARY_OP_NODE,
            LITERAL_NODE,
        };
        Type type = Type::None;

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
            type = Type::BINARY_OP_NODE;
        }

        std::shared_ptr<ExpressionNode> left_operand;
        Token op_token;
        std::shared_ptr<ExpressionNode> right_operand;

        std::string to_string() final
        {
            std::stringstream ss;
            ss << "(" << left_operand->to_string() 
                    << ", " << op_token << ", " 
                    << right_operand->to_string() << ")";
            return ss.str();
        }
    };

    class UnaryOpNode : public ExpressionNode
    {
    public:
        UnaryOpNode(const Token& op_token, std::shared_ptr<ExpressionNode> operand)
            : op_token(op_token), operand(operand)
        {
            type = Type::UNARY_OP_NODE;
        }

        Token op_token;
        std::shared_ptr<ExpressionNode> operand;

        std::string to_string() final
        {
            std::stringstream ss;
            ss << "(" << op_token << ", " << operand->to_string() << ")";
            return ss.str();
        }
    };

    class LiteralNode : public ExpressionNode
    {
    public:
        LiteralNode(const Token& token)
            : token(token)
        {
            type = Type::LITERAL_NODE;
        }

        Token token;
        
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

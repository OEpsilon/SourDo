#pragma once

#include <string>
#include <memory>
#include <sstream>

#include "Token.hpp"

namespace sourdo
{
    struct Node 
    {
        virtual ~Node() = default;
        
        enum class Type
        {
            None = 0,
            BINARY_OP_NODE,
            UNARY_OP_NODE,
            LITERAL_NODE,
        };
        Type type = Type::None;

        virtual std::string to_string() = 0;
    };

    struct ExpressionNode : public Node
    {
        virtual ~ExpressionNode() = default;
    protected:
        ExpressionNode() = default;
    };

    struct BinaryOpNode : public ExpressionNode
    {
        BinaryOpNode(std::shared_ptr<ExpressionNode> left_operand, 
                const Token& op_token, std::shared_ptr<ExpressionNode> right_operand)
            : left_operand(left_operand), op_token(op_token), right_operand(right_operand)
        {
            type = Type::BINARY_OP_NODE;
        }

        virtual ~BinaryOpNode() = default;

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

    struct UnaryOpNode : public ExpressionNode
    {
        UnaryOpNode(const Token& op_token, std::shared_ptr<ExpressionNode> operand)
            : op_token(op_token), operand(operand)
        {
            type = Type::UNARY_OP_NODE;
        }

        virtual ~UnaryOpNode() = default;

        Token op_token;
        std::shared_ptr<ExpressionNode> operand;

        std::string to_string() final
        {
            std::stringstream ss;
            ss << "(" << op_token << ", " << operand->to_string() << ")";
            return ss.str();
        }
    };

    struct LiteralNode : public ExpressionNode
    {
        LiteralNode(const Token& token)
            : token(token)
        {
            type = Type::LITERAL_NODE;
        }

        virtual ~LiteralNode() = default;

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
} // namespace sourdo

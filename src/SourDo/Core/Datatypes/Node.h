#pragma once

#include "Token.h"

namespace srdo {
    class Node;

    class Node
    {
    public:
        enum class Type
        {
            None = 0,
            Literal,
            UnaryOp,
            BinaryOp,
            Print,
        };
        Type type = Type::None;

        virtual ~Node()
        {
        }
    protected:
        Node()
        {
        }
    };

    class ExpressionNode : public Node
    {
    public:
        virtual ~ExpressionNode()
        {
        }
    protected:
        ExpressionNode()
        {
        }
    };

    class LiteralNode : public ExpressionNode
    {
    public:
        LiteralNode(const Token& value)
            : value(value)
        {
            type = Type::Literal;
        }

        Token value;
    };

    class BinaryOpNode : public ExpressionNode
    {
    public:
        BinaryOpNode(std::shared_ptr<ExpressionNode> leftOp, const Token& opTok,
                    std::shared_ptr<ExpressionNode> rightOp)
                : leftOp(leftOp), opTok(opTok), rightOp(rightOp)
        {
            type = Type::BinaryOp;
        }

        std::shared_ptr<ExpressionNode> leftOp;
        Token opTok;
        std::shared_ptr<ExpressionNode> rightOp;
    };

    class UnaryOpNode : public ExpressionNode
    {
    public:
        UnaryOpNode(Token opTok, std::shared_ptr<ExpressionNode> operand)
            : opTok(opTok), operand(operand)
        {
            type = Type::UnaryOp;
        }

        Token opTok;
        std::shared_ptr<ExpressionNode> operand;
    };

    class PrintNode: public Node
    {
    public:
        PrintNode(std::shared_ptr<ExpressionNode> expression)
            : expression(expression)
        {
            type = Type::Print;
        }
        
        std::shared_ptr<ExpressionNode> expression;
    };
}

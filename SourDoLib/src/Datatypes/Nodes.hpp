#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <optional>

#include "Token.hpp"

namespace sourdo
{
    struct Node;
    struct StatementListNode;
    struct IfNode;
    
    struct VarDeclarationNode;
    struct VarAssignmentNode;
    struct VarAccessNode;
    
    struct ExpressionNode;
    struct BinaryOpNode;
    struct UnaryOpNode;
    struct NumberValueNode;
    struct BoolValueNode;
    struct NullValueNode;

    struct Node 
    {
        virtual ~Node() = default;
        
        enum class Type
        {
            NONE = 0,
            STATEMENT_LIST_NODE,
            IF_NODE,

            VAR_DECLARATION_NODE,
            VAR_ASSIGNMENT_NODE,
            VAR_ACCESS_NODE,

            BINARY_OP_NODE,
            UNARY_OP_NODE,
            NUMBER_VALUE_NODE,
            BOOL_VALUE_NODE,
            NULL_VALUE_NODE,
        };
        Type type = Type::NONE;

        virtual std::string to_string() = 0;
    };

    struct StatementListNode : public Node
    {
        StatementListNode(std::vector<std::shared_ptr<Node>> statements)
            : statements(statements)
        {
            type = Type::STATEMENT_LIST_NODE;
        }

        std::vector<std::shared_ptr<Node>> statements;

        virtual ~StatementListNode() = default;

        std::string to_string() final
        {
            std::stringstream ss;
            ss << "(";
            int i = 0;
            for(auto& statement : statements)
            {
                ss << statement->to_string();
                if(i < statements.size() - 1)
                {
                    ss << ", ";
                }
                i++;
            }
            ss << ")";
            return ss.str();
        }
    };

    struct IfNode : public Node
    {
        struct IfBlock
        {
            IfBlock(std::shared_ptr<ExpressionNode> condition, std::shared_ptr<StatementListNode> statements)
                : condition(condition), statements(statements)
            {
            }
            
            std::shared_ptr<ExpressionNode> condition;
            std::shared_ptr<StatementListNode> statements;
        };

        IfNode(std::vector<IfBlock> cases, std::shared_ptr<StatementListNode> else_case)
            : cases(cases), else_case(else_case)
        {
            type = Type::IF_NODE;
        }

        virtual ~IfNode() = default;
        
        std::vector<IfBlock> cases;
        std::shared_ptr<StatementListNode> else_case;

        std::string to_string() final
        {
            // I'll deal with this later
            std::stringstream ss;
            return ss.str();
        }
    };

    struct VarDeclarationNode : public Node
    {
        VarDeclarationNode(const Token& name_tok, std::shared_ptr<ExpressionNode> initializer)
            : name_tok(name_tok), initializer(initializer)
        {
            type = Type::VAR_DECLARATION_NODE;
        }
        Token name_tok;
        std::shared_ptr<ExpressionNode> initializer;

        std::string to_string() final
        {
            std::stringstream ss;
            ss << "(var " << name_tok << ", " << initializer << ")";
            return ss.str();
        }
    };

    struct VarAssignmentNode : public Node
    {
        VarAssignmentNode(const Token& name_tok, std::shared_ptr<ExpressionNode> new_value)
            : name_tok(name_tok), new_value(new_value)
        {
            type = Type::VAR_ASSIGNMENT_NODE;
        }
        Token name_tok;
        std::shared_ptr<ExpressionNode> new_value;

        std::string to_string() final
        {
            std::stringstream ss;
            ss << "(" << name_tok << ", " << new_value << ")";
            return ss.str();
        }
    };

    struct ExpressionNode : public Node
    {
        virtual ~ExpressionNode() = default;
    protected:
        ExpressionNode() = default;
    };

    struct VarAccessNode : public ExpressionNode
    {
        VarAccessNode(const Token& name_tok)
            : name_tok(name_tok)
        {
            type = Type::VAR_ACCESS_NODE;
        }

        virtual ~VarAccessNode() = default;
        
        Token name_tok;

        std::string to_string() final
        {
            std::stringstream ss;
            ss << name_tok;
            return ss.str();
        }
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

    struct NumberValueNode : public ExpressionNode
    {
        NumberValueNode(const Token& value)
            : value(value)
        {
            type = Type::NUMBER_VALUE_NODE;
        }

        virtual ~NumberValueNode() = default;

        Token value;
        
        std::string to_string() final
        {
            std::stringstream ss;
            ss << value;
            return ss.str();
        }
    };

    struct BoolValueNode : public ExpressionNode
    {
        BoolValueNode(const Token& value)
            : value(value)
        {
            type = Type::BOOL_VALUE_NODE;
        }

        virtual ~BoolValueNode() = default;

        Token value;
        
        std::string to_string() final
        {
            std::stringstream ss;
            ss << value;
            return ss.str();
        }
    };

    struct NullValueNode : public ExpressionNode
    {
        NullValueNode()
        {
            type = Type::NULL_VALUE_NODE;
        }

        virtual ~NullValueNode() = default;
        
        std::string to_string() final
        {
            std::stringstream ss;
            ss << "null";
            return ss.str();
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Node>& node)
    {
        os << node->to_string();
        return os;
    }
} // namespace sourdo

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
    struct FuncDeclarationNode;
    struct ReturnNode;
    
    struct ExpressionNode;
    struct BinaryOpNode;
    struct UnaryOpNode;
    struct CallNode;

    struct NumberNode;
    struct StringNode;
    struct BoolNode;
    struct NullNode;
    struct IdentifierNode;

    struct Node 
    {
        enum class Type
        {
            NONE = 0,
            STATEMENT_LIST_NODE,
            IF_NODE,

            VAR_DECLARATION_NODE,
            VAR_ASSIGNMENT_NODE,
            FUNC_DECLARATION_NODE,
            RETURN_NODE,
            
            BINARY_OP_NODE,
            UNARY_OP_NODE,
            CALL_NODE,

            NUMBER_NODE,
            STRING_NODE,
            BOOL_NODE,
            NULL_NODE,
            IDENTIFIER_NODE,
        };

        Node(const Position& position)
            : position(position)
        {

        }

        virtual ~Node() = default;

        Type type = Type::NONE;
        Position position;

        virtual std::string to_string() = 0;
    };

    struct StatementListNode : public Node
    {
        StatementListNode(const std::vector<std::shared_ptr<Node>>& statements, const Position& position)
            : Node(position), statements(statements)
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

        IfNode(const std::vector<IfBlock>& cases, std::shared_ptr<StatementListNode> else_case, const Position& position)
            : Node(position), cases(cases), else_case(else_case)
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
        VarDeclarationNode(const Token& name_tok, std::shared_ptr<ExpressionNode> initializer, const Position& position)
            : Node(position), name_tok(name_tok), initializer(initializer)
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

    struct FuncDeclarationNode : public Node
    {
        FuncDeclarationNode(const Token& name, const std::vector<Token>& parameters, std::shared_ptr<StatementListNode> statements, const Position& position)
            : Node(position), name(name), parameters(parameters), statements(statements)
        {
            type = Type::FUNC_DECLARATION_NODE;
        }

        virtual ~FuncDeclarationNode() = default;

        Token name;
        std::vector<Token> parameters;
        std::shared_ptr<StatementListNode> statements;

        std::string to_string() final
        {
            // implement later
            std::stringstream ss;
            return ss.str();
        }
    };

    struct ExpressionNode : public Node
    {
        virtual ~ExpressionNode() = default;
    protected:
        ExpressionNode(const Position& position)
            : Node(position)
        {
        }
    };

    struct VarAssignmentNode : public ExpressionNode
    {
        enum class Operation
        {
            NONE,
            ADD, 
            SUB, 
            MUL, 
            DIV,
        };
        
        VarAssignmentNode(Token name_tok, Operation op, std::shared_ptr<ExpressionNode> new_value, const Position& position)
            : ExpressionNode(position), name_tok(name_tok), op(op), new_value(new_value)
        {
            type = Type::VAR_ASSIGNMENT_NODE;
        }
        Token name_tok;
        Operation op;
        std::shared_ptr<ExpressionNode> new_value;

        std::string to_string() final
        {
            std::stringstream ss;
            ss << "(" << name_tok << ", " << new_value << ")";
            return ss.str();
        }
    };

    struct ReturnNode : public Node
    {
        ReturnNode(std::shared_ptr<ExpressionNode> return_value, const Position& position)
            : Node(position), return_value(return_value)
        {
            type = Type::RETURN_NODE;
        }

        std::shared_ptr<ExpressionNode> return_value;

        std::string to_string() final
        {
            std::stringstream ss;
            ss << "return " << return_value->to_string();
            return ss.str();
        }

        virtual ~ReturnNode() = default;
    };

    struct BinaryOpNode : public ExpressionNode
    {
        BinaryOpNode(std::shared_ptr<ExpressionNode> left_operand, 
                const Token& op_token, std::shared_ptr<ExpressionNode> right_operand)
            : ExpressionNode(op_token.position), left_operand(left_operand), op_token(op_token), right_operand(right_operand)
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
            : ExpressionNode(op_token.position), op_token(op_token), operand(operand)
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

    struct CallNode : public ExpressionNode
    {
        CallNode(std::shared_ptr<ExpressionNode> callee, const std::vector<std::shared_ptr<ExpressionNode>>& arguments)
            : ExpressionNode(callee->position), callee(callee), arguments(arguments)
        {
            type = Type::CALL_NODE;
        }

        std::shared_ptr<ExpressionNode> callee; 
        std::vector<std::shared_ptr<ExpressionNode>> arguments;

        std::string to_string() final
        {
            std::stringstream ss;
            ss << "(" << callee->to_string() << ", ";
            for(int i = 0; i < arguments.size(); i++)
            {
                ss << arguments[i]->to_string();
                if(i <= arguments.size() - 1)
                {
                    ss << ", ";
                }
            }
            ss << ")";
            return ss.str();
        }
    };

    struct NumberNode : public ExpressionNode
    {
        NumberNode(const Token& value)
            : ExpressionNode(value.position), value(value)
        {
            type = Type::NUMBER_NODE;
        }

        virtual ~NumberNode() = default;

        Token value;
        
        std::string to_string() final
        {
            std::stringstream ss;
            ss << value;
            return ss.str();
        }
    };

    struct StringNode : public ExpressionNode
    {
        StringNode(const Token& value)
            : ExpressionNode(value.position), value(value)
        {
            type = Type::STRING_NODE;
        }

        virtual ~StringNode() = default;

        Token value;
        
        std::string to_string() final
        {
            std::stringstream ss;
            ss << value;
            return ss.str();
        }
    };

    struct BoolNode : public ExpressionNode
    {
        BoolNode(const Token& value)
            : ExpressionNode(value.position), value(value)
        {
            type = Type::BOOL_NODE;
        }

        virtual ~BoolNode() = default;

        Token value;
        
        std::string to_string() final
        {
            std::stringstream ss;
            ss << value;
            return ss.str();
        }
    };

    struct NullNode : public ExpressionNode
    {
        NullNode(const Token& value)
            : ExpressionNode(value.position), value(value)
        {
            type = Type::NULL_NODE;
        }

        virtual ~NullNode() = default;

        Token value;
        
        std::string to_string() final
        {
            std::stringstream ss;
            ss << "null";
            return ss.str();
        }
    };

    struct IdentifierNode : public ExpressionNode
    {
        IdentifierNode(const Token& name_tok)
            : ExpressionNode(name_tok.position), name_tok(name_tok)
        {
            type = Type::IDENTIFIER_NODE;
        }

        virtual ~IdentifierNode() = default;
        
        Token name_tok;

        std::string to_string() final
        {
            std::stringstream ss;
            ss << name_tok;
            return ss.str();
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Node>& node)
    {
        os << node->to_string();
        return os;
    }
} // namespace sourdo

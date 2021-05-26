#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <optional>
#include <map>

#include "Token.hpp"

namespace sourdo
{
    struct Node;
    struct StatementListNode;
    struct IfNode;
    struct ForNode;
    struct WhileNode;
    struct LoopNode;
    
    struct VarDeclarationNode;
    struct AssignmentNode;
    struct FuncNode;
    struct ReturnNode;
    struct BreakNode;
    struct ContinueNode;
    
    struct ExpressionNode;
    struct BinaryOpNode;
    struct UnaryOpNode;
    struct IsNode;
    struct CallNode;
    struct IndexNode;
    struct IndexCallNode;

    struct NumberNode;
    struct StringNode;
    struct BoolNode;
    struct NullNode;
    struct IdentifierNode;
    struct TableNode;

    struct Node 
    {
        enum class Type
        {
            NONE = 0,
            STATEMENT_LIST_NODE,
            IF_NODE,
            FOR_NODE,
            WHILE_NODE,
            LOOP_NODE,

            VAR_DECLARATION_NODE,
            ASSIGNMENT_NODE,
            FUNC_NODE,
            RETURN_NODE,
            BREAK_NODE,
            CONTINUE_NODE,
            
            BINARY_OP_NODE,
            UNARY_OP_NODE,
            IS_NODE,
            CALL_NODE,
            INDEX_NODE,
            INDEX_CALL_NODE,

            NUMBER_NODE,
            STRING_NODE,
            BOOL_NODE,
            NULL_NODE,
            IDENTIFIER_NODE,
            TABLE_NODE,
        };

        Node(const Position& position)
            : position(position)
        {

        }

        virtual ~Node() = default;

        Type type = Type::NONE;
        Position position;

        virtual std::string to_string(uint32_t indent_level) = 0;
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

    struct StatementListNode : public Node
    {
        StatementListNode(const std::vector<std::shared_ptr<Node>>& statements, const Position& position)
            : Node(position), statements(statements)
        {
            type = Type::STATEMENT_LIST_NODE;
        }

        std::vector<std::shared_ptr<Node>> statements;

        virtual ~StatementListNode() = default;

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << "(";
            for(int i = 0; i < statements.size(); i++)
            {
                for(int i = 0; i < indent_level; i++)
                {
                    ss << "\t";
                }
                ss << statements[i]->to_string(indent_level + 1);
                if(i < statements.size() - 1)
                {
                    ss << "\n";
                }
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

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            for(int i = 0; i < cases.size(); i++)
            {
                if(i == 0)
                {
                    ss << "if(";
                }
                else
                {
                    ss << "\nelif(";
                }
                ss << cases[i].condition->to_string(0) << "\n";
                ss << cases[i].statements->to_string(indent_level + 1) << "\n)";
            }
            if(else_case)
            {
                ss << "\nelse(\n";
                ss << else_case->to_string(indent_level + 1) << "\n)";
            }
            return ss.str();
        }
    };

    struct ForNode : public Node
    {
        ForNode(std::shared_ptr<Node> initializer, std::shared_ptr<ExpressionNode> condition, std::shared_ptr<Node> increment, 
                std::shared_ptr<StatementListNode> statements, const Position& position)
            : Node(position), initializer(initializer), condition(condition), increment(increment), statements(statements)
        {
            type = Type::FOR_NODE;
        }

        virtual ~ForNode() = default;

        std::shared_ptr<Node> initializer; 
        std::shared_ptr<ExpressionNode> condition; 
        std::shared_ptr<Node> increment;
        std::shared_ptr<StatementListNode> statements;

        std::string to_string(uint32_t indent_level) final
        {
            // I'll deal with this later
            std::stringstream ss;
            return ss.str();
        }
    };

    struct WhileNode : public Node
    {
        WhileNode(std::shared_ptr<ExpressionNode> condition, std::shared_ptr<StatementListNode> statements, const Position& position)
            : Node(position), condition(condition), statements(statements)
        {
            type = Type::WHILE_NODE;
        }

        std::shared_ptr<ExpressionNode> condition;
        std::shared_ptr<StatementListNode> statements;

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << "while(" << condition->to_string(0) << "\n";
            ss << statements->to_string(indent_level + 1) << "\n)";
            return ss.str();
        }

        virtual ~WhileNode() = default;
    };

    struct LoopNode : public Node
    {
        LoopNode(std::shared_ptr<StatementListNode> statements, const Position& position)
            : Node(position), statements(statements)
        {
            type = Type::LOOP_NODE;
        }

        std::shared_ptr<StatementListNode> statements;

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << "loop(" << "\n";
            ss << statements->to_string(indent_level + 1) << "\n";
            for(int i = 0; i < indent_level; i++)
            {
                ss << "\t";
            }

            ss << ")";
            return ss.str();
        }

        virtual ~LoopNode() = default;
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

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << "(var " << name_tok << ", " << initializer << ")";
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

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << "return " << return_value->to_string(0);
            return ss.str();
        }

        virtual ~ReturnNode() = default;
    };

    struct BreakNode : public Node
    {
        BreakNode(const Position& position)
            : Node(position)
        {
            type = Type::BREAK_NODE;
        }

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << "break";
            return ss.str();
        }

        virtual ~BreakNode() = default;
    };

    struct ContinueNode : public Node
    {
        ContinueNode(const Position& position)
            : Node(position)
        {
            type = Type::CONTINUE_NODE;
        }

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << "continue";
            return ss.str();
        }

        virtual ~ContinueNode() = default;
    };

    struct AssignmentNode : public ExpressionNode
    {
        enum class Operation
        {
            NONE,
            ADD, 
            SUB, 
            MUL, 
            DIV,
        };
        
        AssignmentNode(std::shared_ptr<ExpressionNode> assignee, Operation op, std::shared_ptr<ExpressionNode> new_value, const Position& position)
            : ExpressionNode(position), assignee(assignee), op(op), new_value(new_value)
        {
            type = Type::ASSIGNMENT_NODE;
        }
        std::shared_ptr<ExpressionNode> assignee;
        Operation op;
        std::shared_ptr<ExpressionNode> new_value;

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << "(" << assignee->to_string(0) << ", " << new_value << ")";
            return ss.str();
        }
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

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << "(" << left_operand->to_string(0) 
                    << ", " << op_token << ", " 
                    << right_operand->to_string(0) << ")";
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

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << "(" << op_token << ", " << operand->to_string(0) << ")";
            return ss.str();
        }
    };

    struct IsNode : public ExpressionNode
    {
        IsNode(std::shared_ptr<ExpressionNode> left_operand, bool invert, const Token& right_operand, 
                const Position& position)
            : ExpressionNode(position), left_operand(left_operand), invert(invert), right_operand(right_operand)
        {
            type = Type::IS_NODE;
        }
        
        std::shared_ptr<ExpressionNode> left_operand;
        bool invert;
        Token right_operand;

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << "(" << left_operand->to_string(0) 
                    << ", is, " 
                    << right_operand << ")";
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

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << "(" << callee->to_string(0) << ", ";
            for(int i = 0; i < arguments.size(); i++)
            {
                ss << arguments[i]->to_string(0);
                if(i <= arguments.size() - 1)
                {
                    ss << ", ";
                }
            }
            ss << ")";
            return ss.str();
        }
    };

    struct IndexNode : public ExpressionNode
    {
        IndexNode(std::shared_ptr<ExpressionNode> base, std::shared_ptr<ExpressionNode> attribute)
            : ExpressionNode(base->position), base(base), attribute(attribute)
        {
            type = Type::INDEX_NODE;
        }

        std::shared_ptr<ExpressionNode> base; 
        std::shared_ptr<ExpressionNode> attribute;

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << "(" << base->to_string(0) << ", " << attribute->to_string(0) << ")";
            return ss.str();
        }
    };

    struct IndexCallNode : public ExpressionNode
    {
        IndexCallNode(std::shared_ptr<ExpressionNode> base, std::shared_ptr<ExpressionNode> callee, 
                const std::vector<std::shared_ptr<ExpressionNode>>& arguments)
            : ExpressionNode(base->position), base(base), callee(callee), arguments(arguments)
        {
            type = Type::INDEX_CALL_NODE;
        }

        std::shared_ptr<ExpressionNode> base; 
        std::shared_ptr<ExpressionNode> callee;
        std::vector<std::shared_ptr<ExpressionNode>> arguments;

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << "(" << base->to_string(0) << ", " << callee->to_string(0) << ", ";
            
            for(int i = 0; i < arguments.size(); i++)
            {
                ss << arguments[i]->to_string(0);
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
        
        std::string to_string(uint32_t indent_level) final
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
        
        std::string to_string(uint32_t indent_level) final
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
        
        std::string to_string(uint32_t indent_level) final
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
        
        std::string to_string(uint32_t indent_level) final
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

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            ss << name_tok;
            return ss.str();
        }
    };

    struct FuncNode : public ExpressionNode
    {
        FuncNode(const std::vector<std::string>& parameters, std::shared_ptr<StatementListNode> statements, const Position& position)
            : ExpressionNode(position), parameters(parameters), statements(statements)
        {
            type = Type::FUNC_NODE;
        }

        virtual ~FuncNode() = default;

        std::vector<std::string> parameters;
        std::shared_ptr<StatementListNode> statements;

        std::string to_string(uint32_t indent_level) final
        {
            // implement later
            std::stringstream ss;
            return ss.str();
        }
    };

    struct TableNode : public ExpressionNode
    {
        TableNode(const std::map<std::shared_ptr<ExpressionNode>, std::shared_ptr<ExpressionNode>>& keys, 
                const Position& position)
            : ExpressionNode(position), keys(keys)
        {
            type = Type::TABLE_NODE;
        }

        std::map<std::shared_ptr<ExpressionNode>, std::shared_ptr<ExpressionNode>> keys;

        std::string to_string(uint32_t indent_level) final
        {
            std::stringstream ss;
            return ss.str();
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Node>& node)
    {
        os << node->to_string(0);
        return os;
    }
} // namespace sourdo

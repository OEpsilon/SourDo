#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <optional>
#include <map>
#include <unordered_map>

#include "Token.hpp"

namespace sourdo
{
    struct Node;
    struct StatementListNode;
    struct ClassNode;
    
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
            STATEMENT_LIST_NODE,
            CLASS_NODE,
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

        virtual ~Node() = default;

        Type type;
        Position position;
    protected:
        Node(const Position& position)
            : position(position)
        {

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

    struct StatementListNode : public Node
    {
        StatementListNode(const std::vector<std::shared_ptr<Node>>& statements, const Position& position)
            : Node(position), statements(statements)
        {
            type = Type::STATEMENT_LIST_NODE;
        }

        std::vector<std::shared_ptr<Node>> statements;

        virtual ~StatementListNode() = default;
    };

    struct ClassNode : public Node
    {
        struct Property
        {
            Property() = default;

            Property(std::shared_ptr<ExpressionNode> initial_value, bool readonly, bool is_private)
                : initial_value(initial_value), readonly(readonly), is_private(is_private)
            {
            }

            std::shared_ptr<ExpressionNode> initial_value;
            bool readonly = false;
            bool is_private = false;
        };

        struct Setter
        {
            Setter() = default;

            Setter(const Token& self_name, const Token& new_value_name,
                    std::shared_ptr<StatementListNode> statements,
                    bool is_private, const Position& position)
                : self_name(self_name), new_value_name(new_value_name), 
                    statements(statements), is_private(is_private), position(position)
            {
            }

            Token self_name;
            Token new_value_name;
            std::shared_ptr<StatementListNode> statements;
            Position position;
            bool is_private = false;
        };

        struct Getter
        {
            Getter() = default;
            
            Getter(const Token& self_name, std::shared_ptr<StatementListNode> statements,
                    bool is_private, const Position& position)
                : self_name(self_name), statements(statements), is_private(is_private), position(position)
            {
            }

            Token self_name;
            std::shared_ptr<StatementListNode> statements;
            Position position;
            bool is_private = false;
        };

        ClassNode(const Token& class_name, const std::optional<Token>& super_name,
                const std::unordered_map<std::string, Property>& properties, 
                const std::unordered_map<std::string, Property>& methods,
                const std::unordered_map<std::string, Setter>& setters, 
                const std::unordered_map<std::string, Getter>& getters, 
                const Position& position)
            : Node(position), class_name(class_name), super_name(super_name),
                properties(std::move(properties)), methods(std::move(methods)),
                setters(std::move(setters)), getters(std::move(getters))
        {
            type = Type::CLASS_NODE;
        }

        Token class_name;
        std::optional<Token> super_name;
        std::unordered_map<std::string, Property> properties;
        std::unordered_map<std::string, Property> methods;
        std::unordered_map<std::string, Setter> setters;
        std::unordered_map<std::string, Getter> getters;
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

        virtual ~LoopNode() = default;
    };

    struct VarDeclarationNode : public Node
    {
        VarDeclarationNode(const Token& name_tok, std::shared_ptr<ExpressionNode> initializer, 
                bool readonly, const Position& position)
            : Node(position), name_tok(name_tok), initializer(initializer), readonly(readonly)
        {
            type = Type::VAR_DECLARATION_NODE;
        }
        Token name_tok;
        std::shared_ptr<ExpressionNode> initializer;
        bool readonly;
    };

    struct ReturnNode : public Node
    {
        ReturnNode(std::shared_ptr<ExpressionNode> return_value, const Position& position)
            : Node(position), return_value(return_value)
        {
            type = Type::RETURN_NODE;
        }

        std::shared_ptr<ExpressionNode> return_value;

        virtual ~ReturnNode() = default;
    };

    struct BreakNode : public Node
    {
        BreakNode(const Position& position)
            : Node(position)
        {
            type = Type::BREAK_NODE;
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
    };

} // namespace sourdo

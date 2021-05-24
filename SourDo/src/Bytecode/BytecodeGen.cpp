#include "BytecodeGen.hpp"

#include "../SourDoData.hpp"

#include <exception>

namespace sourdo
{
    BytecodeGenerator::Result BytecodeGenerator::generate_bytecode(std::shared_ptr<Node> ast)
    {
        Bytecode bytecode;
        visit_node(ast, bytecode);
        return {std::move(bytecode), error};
    }

    uint64_t BytecodeGenerator::push_constant(const Value& val, Bytecode& bytecode)
    {
        auto it = std::find(bytecode.constants.begin(), bytecode.constants.end(), val);
        if(it != bytecode.constants.end())
        {
            return it - bytecode.constants.begin();
        }
        uint64_t const_position = bytecode.constants.size();
        bytecode.constants.emplace_back(val);
        return const_position;
    }
    
    void BytecodeGenerator::visit_node(std::shared_ptr<Node> node, Bytecode& bytecode)
    {
        switch(node->type)
        {
            case Node::Type::STATEMENT_LIST_NODE:
                visit_statement_list_node(std::static_pointer_cast<StatementListNode>(node), bytecode);
                break;
            case Node::Type::VAR_DECLARATION_NODE:
                visit_var_declaration_node(std::static_pointer_cast<VarDeclarationNode>(node), bytecode);
                break;
            case Node::Type::ASSIGNMENT_NODE:
                visit_assignment_node(std::static_pointer_cast<AssignmentNode>(node), bytecode);
                break;
            case Node::Type::CALL_NODE:
                visit_call_node(std::static_pointer_cast<CallNode>(node), bytecode);
                break;
            case Node::Type::BINARY_OP_NODE:
                visit_binary_op_node(std::static_pointer_cast<BinaryOpNode>(node), bytecode);
                break;
            case Node::Type::NUMBER_NODE:
                visit_number_node(std::static_pointer_cast<NumberNode>(node), bytecode);
                break;
            case Node::Type::STRING_NODE:
                visit_string_node(std::static_pointer_cast<StringNode>(node), bytecode);
                break;
            case Node::Type::BOOL_NODE:
                visit_bool_node(std::static_pointer_cast<BoolNode>(node), bytecode);
                break;
            case Node::Type::NULL_NODE:
                visit_null_node(std::static_pointer_cast<NullNode>(node), bytecode);
                break;
            case Node::Type::IDENTIFIER_NODE:
                visit_identifier_node(std::static_pointer_cast<IdentifierNode>(node), bytecode);
                break;
            default:
                throw std::runtime_error("A node is not implemented yet");
                break;
        }
    }

    void BytecodeGenerator::visit_statement_list_node(std::shared_ptr<StatementListNode> node, Bytecode& bytecode)
    {
        static const std::array<Node::Type, 7> expression_types = 
        {
            Node::Type::CALL_NODE,
            Node::Type::BINARY_OP_NODE,
            Node::Type::NUMBER_NODE,
            Node::Type::STRING_NODE,
            Node::Type::BOOL_NODE,
            Node::Type::NULL_NODE,
            Node::Type::IDENTIFIER_NODE,
        };

        for(auto& stmt : node->statements)
        {
            visit_node(stmt, bytecode);
            if(error)
            {
                return;
            }
            if(std::find(expression_types.begin(), expression_types.end(), stmt->type) != expression_types.end())
            {
                bytecode.instructions.emplace_back(OP_POP);
            }
        }
    }

    void BytecodeGenerator::visit_var_declaration_node(std::shared_ptr<VarDeclarationNode> node, Bytecode& bytecode)
    {
        uint64_t var_name = push_constant(node->name_tok.value, bytecode);
        if(node->initializer)
        {
            visit_node(node->initializer, bytecode);
            if(error)
            {
                return;
            }
        }
        else
        {
            bytecode.instructions.emplace_back(OP_PUSH_NULL);
        }
        bytecode.instructions.emplace_back(OP_SYM_CREATE, var_name);
    }

    void BytecodeGenerator::visit_assignment_node(std::shared_ptr<AssignmentNode> node, Bytecode& bytecode)
    {
        uint64_t sym_name = push_constant(
                std::static_pointer_cast<IdentifierNode>(node->assignee)->name_tok.value, bytecode);
        if(node->op != AssignmentNode::Operation::NONE)
        {
            visit_node(node->assignee, bytecode);
        }

        visit_node(node->new_value, bytecode);

        switch(node->op)
        {
            case AssignmentNode::Operation::ADD:
                bytecode.instructions.emplace_back(OP_ADD);
                break;
            case AssignmentNode::Operation::SUB:
                bytecode.instructions.emplace_back(OP_SUB);
                break;
            case AssignmentNode::Operation::MUL:
                bytecode.instructions.emplace_back(OP_MUL);
                break;
            case AssignmentNode::Operation::DIV:
                bytecode.instructions.emplace_back(OP_DIV);
                break;
            default:
                break;
        }
        bytecode.instructions.emplace_back(OP_SYM_SET, sym_name);
    }

    void BytecodeGenerator::visit_call_node(std::shared_ptr<CallNode> node, Bytecode& bytecode)
    {
        visit_node(node->callee, bytecode);
        if(error) return;

        for(auto& arg : node->arguments)
        {
            visit_node(arg, bytecode);
            if(error) return;
        }
        bytecode.instructions.emplace_back(Opcode::OP_CALL, node->arguments.size());
    }

    void BytecodeGenerator::visit_binary_op_node(std::shared_ptr<BinaryOpNode> node, Bytecode& bytecode)
    {
        visit_node(node->left_operand, bytecode);
        if(error) return;
        
        visit_node(node->right_operand, bytecode);
        if(error) return;

        switch(node->op_token.type)
        {
            case Token::Type::ADD:
                bytecode.instructions.emplace_back(OP_ADD);
                break;
            case Token::Type::SUB:
                bytecode.instructions.emplace_back(OP_SUB);
                break;
            case Token::Type::MUL:
                bytecode.instructions.emplace_back(OP_MUL);
                break;
            case Token::Type::DIV:
                bytecode.instructions.emplace_back(OP_DIV);
                break;
            case Token::Type::MOD:
                bytecode.instructions.emplace_back(OP_MOD);
                break;
            case Token::Type::POW:
                bytecode.instructions.emplace_back(OP_POW);
                break;
            default:
                break;
        }
    }

    void BytecodeGenerator::visit_number_node(std::shared_ptr<NumberNode> node, Bytecode& bytecode)
    {
        uint64_t constant = push_constant(std::stod(node->value.value), bytecode);
        bytecode.instructions.emplace_back(Opcode::OP_PUSH_NUMBER, constant);
    }

    void BytecodeGenerator::visit_string_node(std::shared_ptr<StringNode> node, Bytecode& bytecode)
    {
        uint64_t constant = push_constant(node->value.value, bytecode);
        bytecode.instructions.emplace_back(Opcode::OP_PUSH_STRING, constant);
    }

    void BytecodeGenerator::visit_bool_node(std::shared_ptr<BoolNode> node, Bytecode& bytecode)
    {
        uint64_t constant = push_constant(node->value.type == Token::Type::BOOL_TRUE ? true : false, bytecode);
        bytecode.instructions.emplace_back(Opcode::OP_PUSH_BOOL, constant);
    }

    void BytecodeGenerator::visit_null_node(std::shared_ptr<NullNode> node, Bytecode& bytecode)
    {
        bytecode.instructions.emplace_back(Opcode::OP_PUSH_NULL);
    }

    void BytecodeGenerator::visit_identifier_node(std::shared_ptr<IdentifierNode> node, Bytecode& bytecode)
    {
        uint64_t constant = push_constant(node->name_tok.value, bytecode);
        bytecode.instructions.emplace_back(Opcode::OP_SYM_GET, constant);
    }

} // namespace sourdo

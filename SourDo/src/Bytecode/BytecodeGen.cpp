#include "BytecodeGen.hpp"

#include <exception>

namespace sourdo
{
    Bytecode BytecodeGenerator::generate_bytecode(std::shared_ptr<Node> ast)
    {
        Bytecode bytecode;
        visit_node(ast, bytecode);
        return std::move(bytecode);
    }

    uint64_t BytecodeGenerator::push_constant(const Value& val, Bytecode& bytecode)
    {
        auto it = std::find(bytecode.constants.begin(), bytecode.constants.end(), val);
        if(it != bytecode.constants.end())
        {
            return it - bytecode.constants.begin();
        }
        uint64_t const_position = bytecode.constants.size();
        bytecode.constants.push_back(val);
        return const_position;
    }
    
    void BytecodeGenerator::visit_node(std::shared_ptr<Node> node, Bytecode& bytecode)
    {
        switch(node->type)
        {
            case Node::Type::STATEMENT_LIST_NODE:
                visit_statement_list_node(std::static_pointer_cast<StatementListNode>(node), bytecode);
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
        for(auto& stmt : node->statements)
        {
            visit_node(stmt, bytecode);
            bytecode.instructions.push_back(OP_POP);
        }
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

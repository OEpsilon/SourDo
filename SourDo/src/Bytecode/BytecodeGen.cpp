#include "BytecodeGen.hpp"

#include "../SourDoData.hpp"
#include "../Datatypes/Function.hpp"

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

    void BytecodeGenerator::fix_control_flows(uint64_t continue_spot, uint64_t break_spot, Bytecode& bytecode)
    {
        for(auto& brk : breaks)
        {
            bytecode.instructions[brk].operand = break_spot;
        }
        for(auto& cont : continues)
        {
            bytecode.instructions[cont].operand = continue_spot;
        }
    }
    
    void BytecodeGenerator::visit_node(std::shared_ptr<Node> node, Bytecode& bytecode)
    {
        switch(node->type)
        {
            case Node::Type::STATEMENT_LIST_NODE:
                visit_statement_list_node(std::static_pointer_cast<StatementListNode>(node), bytecode);
                break;
            case Node::Type::IF_NODE:
                visit_if_node(std::static_pointer_cast<IfNode>(node), bytecode);
                break;
            case Node::Type::FOR_NODE:
                visit_for_node(std::static_pointer_cast<ForNode>(node), bytecode);
                break;
            case Node::Type::WHILE_NODE:
                visit_while_node(std::static_pointer_cast<WhileNode>(node), bytecode);
                break;
            case Node::Type::LOOP_NODE:
                visit_loop_node(std::static_pointer_cast<LoopNode>(node), bytecode);
                break;
            case Node::Type::VAR_DECLARATION_NODE:
                visit_var_declaration_node(std::static_pointer_cast<VarDeclarationNode>(node), bytecode);
                break;
            case Node::Type::ASSIGNMENT_NODE:
                visit_assignment_node(std::static_pointer_cast<AssignmentNode>(node), bytecode);
                break;
            case Node::Type::FUNC_NODE:
                visit_func_node(std::static_pointer_cast<FuncNode>(node), bytecode);
                break;
            case Node::Type::RETURN_NODE:
                visit_return_node(std::static_pointer_cast<ReturnNode>(node), bytecode);
                break;
            case Node::Type::BREAK_NODE:
                visit_break_node(std::static_pointer_cast<BreakNode>(node), bytecode);
                break;
            case Node::Type::CONTINUE_NODE:
                visit_continue_node(std::static_pointer_cast<ContinueNode>(node), bytecode);
                break;
            case Node::Type::CALL_NODE:
                visit_call_node(std::static_pointer_cast<CallNode>(node), bytecode);
                break;
            case Node::Type::BINARY_OP_NODE:
                visit_binary_op_node(std::static_pointer_cast<BinaryOpNode>(node), bytecode);
                break;
            case Node::Type::UNARY_OP_NODE:
                visit_unary_op_node(std::static_pointer_cast<UnaryOpNode>(node), bytecode);
                break;
            case Node::Type::INDEX_NODE:
                visit_index_node(std::static_pointer_cast<IndexNode>(node), bytecode);
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
            case Node::Type::TABLE_NODE:
                visit_table_node(std::static_pointer_cast<TableNode>(node), bytecode);
                break;
            default:
                throw std::runtime_error("A node is not implemented yet");
                break;
        }
    }

    void BytecodeGenerator::visit_statement_list_node(std::shared_ptr<StatementListNode> node, Bytecode& bytecode)
    {
        static const std::array<Node::Type, 9> expression_types = 
        {
            Node::Type::FUNC_NODE,
            Node::Type::CALL_NODE,
            Node::Type::BINARY_OP_NODE,
            Node::Type::NUMBER_NODE,
            Node::Type::STRING_NODE,
            Node::Type::BOOL_NODE,
            Node::Type::NULL_NODE,
            Node::Type::IDENTIFIER_NODE,
            Node::Type::TABLE_NODE,
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

    void BytecodeGenerator::visit_if_node(std::shared_ptr<IfNode> node, Bytecode& bytecode)
    {
        std::vector<uint64_t> jumps;
        jumps.reserve(node->cases.size());
        for(auto& if_case : node->cases)
        {
            visit_node(if_case.condition, bytecode);
            if(error) return;
            
            uint64_t jump_position = bytecode.instructions.size();
            bytecode.instructions.emplace_back(OP_NJMP);
            bytecode.instructions.emplace_back(OP_PUSH_SCOPE);
            visit_node(if_case.statements, bytecode);
            if(error) return;

            bytecode.instructions.emplace_back(OP_POP_SCOPE);

            jumps.emplace_back(bytecode.instructions.size());
            bytecode.instructions.emplace_back(OP_JMP);
            bytecode.instructions[jump_position].operand = bytecode.instructions.size();
        }

        if(node->else_case)
        {
            bytecode.instructions.emplace_back(OP_PUSH_SCOPE);
            visit_node(node->else_case, bytecode);
            if(error) return;

            bytecode.instructions.emplace_back(OP_POP_SCOPE);
        }

        for(auto& jmp : jumps) 
        {
            bytecode.instructions[jmp].operand = bytecode.instructions.size();
        }
    }

    void BytecodeGenerator::visit_for_node(std::shared_ptr<ForNode> node, Bytecode& bytecode)
    {
        bytecode.instructions.emplace_back(OP_PUSH_SCOPE);

        visit_node(node->initializer, bytecode);
        if(error) return;
        
        uint64_t start_position = bytecode.instructions.size();
        bytecode.instructions.emplace_back(OP_PUSH_SCOPE);
        visit_node(node->condition, bytecode);
        if(error) return;

        uint64_t jump_position = bytecode.instructions.size();
        bytecode.instructions.emplace_back(OP_NJMP);

        bool saved_in_loop = in_loop;
        in_loop = true;
        visit_node(node->statements, bytecode);
        if(error) return;

        in_loop = saved_in_loop;

        bytecode.instructions.emplace_back(OP_POP_SCOPE);

        visit_node(node->increment, bytecode);
        if(error) return;

        bytecode.instructions.emplace_back(OP_JMP, start_position);

        bytecode.instructions[jump_position].operand = bytecode.instructions.size();
        fix_control_flows(start_position, bytecode.instructions.size(), bytecode);
        bytecode.instructions.emplace_back(OP_POP_SCOPE);
    }
    
    void BytecodeGenerator::visit_while_node(std::shared_ptr<WhileNode> node, Bytecode& bytecode)
    {
        bytecode.instructions.emplace_back(OP_PUSH_SCOPE);

        uint64_t start_position = bytecode.instructions.size();
        visit_node(node->condition, bytecode);
        if(error) return;

        uint64_t jump_position = bytecode.instructions.size();
        bytecode.instructions.emplace_back(OP_NJMP);

        bool saved_in_loop = in_loop;
        in_loop = true;
        visit_node(node->statements, bytecode);
        if(error) return;

        in_loop = saved_in_loop;
        
        bytecode.instructions.emplace_back(OP_JMP, start_position);

        bytecode.instructions[jump_position].operand = bytecode.instructions.size();
        fix_control_flows(start_position, bytecode.instructions.size(), bytecode);

        bytecode.instructions.emplace_back(OP_POP_SCOPE);
    }

    void BytecodeGenerator::visit_loop_node(std::shared_ptr<LoopNode> node, Bytecode& bytecode)
    {
        bytecode.instructions.emplace_back(OP_PUSH_SCOPE);
        uint64_t start_position = bytecode.instructions.size();
        bool saved_in_loop = in_loop;
        in_loop = true;
        visit_node(node->statements, bytecode);
        if(error) return;

        in_loop = saved_in_loop;

        bytecode.instructions.emplace_back(OP_JMP, start_position);
        fix_control_flows(start_position, bytecode.instructions.size(), bytecode);

        bytecode.instructions.emplace_back(OP_POP_SCOPE);
    }

    void BytecodeGenerator::visit_var_declaration_node(std::shared_ptr<VarDeclarationNode> node, Bytecode& bytecode)
    {
        uint64_t var_name = push_constant(node->name_tok.value, bytecode);
        if(node->initializer)
        {
            visit_node(node->initializer, bytecode);
            if(error) return;
        }
        else
        {
            bytecode.instructions.emplace_back(OP_PUSH_NULL);
        }
        bytecode.instructions.emplace_back(OP_SYM_CREATE, var_name);
    }

    void BytecodeGenerator::visit_assignment_node(std::shared_ptr<AssignmentNode> node, Bytecode& bytecode)
    {
        if(node->assignee->type == Node::Type::IDENTIFIER_NODE)
        {
            uint64_t sym_name = push_constant(
                    std::static_pointer_cast<IdentifierNode>(node->assignee)->name_tok.value, bytecode);
            if(node->op != AssignmentNode::Operation::NONE)
            {
                visit_node(node->assignee, bytecode);
                if(error) return;
            }
            visit_node(node->new_value, bytecode);
            if(error) return;

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
            return;
        }
        auto index_node = std::static_pointer_cast<IndexNode>(node->assignee);
        visit_node(index_node->base, bytecode);
        if(error) return;

        visit_node(index_node->attribute, bytecode);
        if(error) return;

        if(node->op != AssignmentNode::Operation::NONE)
        {
            bytecode.instructions.emplace_back(OP_STACK_GET_TOP, 2);
            bytecode.instructions.emplace_back(OP_STACK_GET_TOP, 2);
            bytecode.instructions.emplace_back(OP_VAL_GET);
        }
        visit_node(node->new_value, bytecode);
        if(error) return;

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
        bytecode.instructions.emplace_back(OP_VAL_SET);
    }

    void BytecodeGenerator::visit_func_node(std::shared_ptr<FuncNode> node, Bytecode& bytecode)
    {
        Bytecode func;
        for(int64_t i = 1; i <= node->parameters.size(); i++)
        {
            uint64_t name = push_constant(node->parameters[i - 1], func);

            func.instructions.emplace_back(OP_STACK_GET, i);
            func.instructions.emplace_back(OP_SYM_CREATE, name);
        }

        visit_node(node->statements, func);
        if(error) return;

        if(node->statements->statements.size() == 0 
                || node->statements->statements[node->statements->statements.size() -1]->type != Node::Type::RETURN_NODE)
        {
            func.instructions.emplace_back(OP_PUSH_NULL);
            func.instructions.emplace_back(OP_RET);
        }
        SourDoFunction* value = new SourDoFunction(node->parameters.size(), func);
        uint64_t constant = push_constant(value, bytecode);
        bytecode.instructions.emplace_back(OP_PUSH_FUNC, constant);
    }

    void BytecodeGenerator::visit_return_node(std::shared_ptr<ReturnNode> node, Bytecode& bytecode)
    {
        visit_node(node->return_value, bytecode);
        if(error) return;
        bytecode.instructions.emplace_back(OP_RET);
    }

    void BytecodeGenerator::visit_break_node(std::shared_ptr<BreakNode> node, Bytecode& bytecode)
    {
        if(!in_loop)
        {
            std::stringstream ss;
            ss << node->position << "Cannot break outside of a loop";
            error = ss.str();
            return;
        }
        breaks.emplace_back(bytecode.instructions.size());
        bytecode.instructions.emplace_back(OP_JMP);
    }

    void BytecodeGenerator::visit_continue_node(std::shared_ptr<ContinueNode> node, Bytecode& bytecode)
    {
        if(!in_loop)
        {
            std::stringstream ss;
            ss << node->position << "Cannot continue outside of a loop";
            error = ss.str();
            return;
        }
        continues.emplace_back(bytecode.instructions.size());
        bytecode.instructions.emplace_back(OP_JMP);
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
            case Token::Type::EQUAL:
                bytecode.instructions.emplace_back(OP_EQ);
                break;
            case Token::Type::NOT_EQUAL:
                bytecode.instructions.emplace_back(OP_NE);
                break;
            case Token::Type::LESS_THAN:
                bytecode.instructions.emplace_back(OP_LT);
                break;
            case Token::Type::GREATER_THAN:
                bytecode.instructions.emplace_back(OP_GT);
                break;
            case Token::Type::GREATER_EQUAL:
                bytecode.instructions.emplace_back(OP_GE);
                break;
            case Token::Type::LESS_EQUAL:
                bytecode.instructions.emplace_back(OP_LE);
                break;
            case Token::Type::AND:
                bytecode.instructions.emplace_back(OP_AND);
                break;
            case Token::Type::OR:
                bytecode.instructions.emplace_back(OP_OR);
                break;
            default:
                break;
        }
    }

    void BytecodeGenerator::visit_unary_op_node(std::shared_ptr<UnaryOpNode> node, Bytecode& bytecode)
    {
        visit_node(node->operand, bytecode);
        if(error) return;

        switch(node->op_token.type)
        {
            case Token::Type::SUB:
                bytecode.instructions.emplace_back(OP_NEG);
                break;
            case Token::Type::NOT:
                bytecode.instructions.emplace_back(OP_NOT);
                break;
            default:
                break;
        }
    }

    void BytecodeGenerator::visit_index_node(std::shared_ptr<IndexNode> node, Bytecode& bytecode)
    {
        visit_node(node->base, bytecode);
        visit_node(node->attribute, bytecode);
        bytecode.instructions.emplace_back(OP_VAL_GET);
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

    void BytecodeGenerator::visit_table_node(std::shared_ptr<TableNode> node , Bytecode& bytecode)
    {
        bytecode.instructions.emplace_back(OP_AlLOC_TABLE);
        for(auto[k, v] : node->keys)
        {
            bytecode.instructions.emplace_back(OP_STACK_GET_TOP, 1);
            visit_node(k, bytecode);
            if(error) return;

            visit_node(v, bytecode);
            if(error) return;
            
            bytecode.instructions.emplace_back(OP_VAL_SET);
        }
    }

} // namespace sourdo

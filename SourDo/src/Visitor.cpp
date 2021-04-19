#include "Interpreter.hpp"

#include <cmath>
#include <sstream>
#include <iostream>

#include "SourDoData.hpp"

namespace sourdo
{
    static VisitorReturn visit_statement_list_node(sourdo_Data* data, std::shared_ptr<StatementListNode> node)
    {
        VisitorReturn return_value;
        for(auto& stmt : node->statements)
        {
            VisitorReturn stmt_return = visit_ast(data, stmt);
            if(stmt_return.error_message)
            {
                return stmt_return;
            }
        }
        return return_value;
    }

    static VisitorReturn visit_if_node(sourdo_Data* data, std::shared_ptr<IfNode> node)
    {
        VisitorReturn return_value;
        for(auto& if_case : node->cases)
        {
            VisitorReturn condition = visit_ast(data, if_case.condition);
            if(condition.error_message)
            {
                return condition;
            }
            if(condition.result.get_type() != Value::Type::BOOL)
            {
                std::stringstream ss;
                ss << if_case.condition->position << "condition does not result in a bool";
                return_value.error_message = ss.str();
                return return_value;
            }
            if(condition.result.to_bool())
            {
                sourdo_Data* if_scope = sourdo_data_create();
                if_scope->parent = data;
                VisitorReturn statements = visit_ast(if_scope, if_case.statements);
                sourdo_data_destroy(if_scope);
                return statements;
            }
        }
        if(node->else_case)
        {
            VisitorReturn statements = visit_ast(data, node->else_case);
        }
        return return_value;
    }

    static VisitorReturn visit_var_declaration_node(sourdo_Data* data, std::shared_ptr<VarDeclarationNode> node)
    {
        VisitorReturn return_value;
        VisitorReturn var_value;
        if(node->initializer)
        {
            var_value = visit_ast(data, node->initializer);
        }
        else
        {
            var_value.result = Null();
        }
        
        if(var_value.error_message)
        {
            return var_value;
        }

        if(sourdo_get_symbol(data, node->name_tok.value) != nullptr)
        {
            std::stringstream ss;
            ss << node->position << "Variable '" << node->name_tok.value << "' is already defined";
            return_value.error_message = ss.str();
        }
        else
        {
            data->symbol_table[node->name_tok.value] = var_value.result;
            return_value.result = var_value.result;
        }

        return return_value;
    }

    static VisitorReturn visit_var_assignment_node(sourdo_Data* data, std::shared_ptr<VarAssignmentNode> node)
    {
        VisitorReturn return_value;
        VisitorReturn new_value = visit_ast(data, node->new_value);
        if(new_value.error_message)
        {
            return new_value;
        }

        if(sourdo_get_symbol(data, node->name_tok.value) == nullptr)
        {
            std::stringstream ss;
            ss << node->position << "Variable '" << node->name_tok.value << "' is not defined";
            return_value.error_message = ss.str();
        }
        else
        {
            data->symbol_table[node->name_tok.value] = new_value.result;
            return_value.result = data->symbol_table[node->name_tok.value];
        }

        return return_value;
    }

    static VisitorReturn visit_var_access_node(sourdo_Data* data, std::shared_ptr<VarAccessNode> node)
    {
        VisitorReturn return_value;

        sourdo::Value* var_value = sourdo_get_symbol(data, node->name_tok.value);
        if(var_value == nullptr)
        {
            std::stringstream ss;
            ss << node->position << "Variable '" << node->name_tok.value << "' is not defined";
            return_value.error_message = ss.str();
        }
        else
        {
            return_value.result = *var_value;
        }
        return return_value;
    }

    static VisitorReturn visit_unary_op_node(sourdo_Data* data, std::shared_ptr<UnaryOpNode> node)
    {
        VisitorReturn return_value;
        VisitorReturn operand_value = visit_ast(data, node->operand);
        if(operand_value.error_message)
        {
            return return_value;
        }

        if(operand_value.result.get_type() == Value::Type::NUMBER)
        {
            switch(node->op_token.type)
            {
                case Token::Type::ADD:
                {
                    return_value.result = operand_value.result.to_number();
                    break;
                }
                case Token::Type::SUB:
                {
                    return_value.result = -(operand_value.result.to_number());
                    break;
                }
                case Token::Type::LOGIC_NOT:
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform a logical operation on numbers";
                    return_value.error_message = ss.str();
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        else if(operand_value.result.get_type() == Value::Type::BOOL)
        {
            switch(node->op_token.type)
            {
                case Token::Type::ADD:
                case Token::Type::SUB:
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform an arithmetic operation on bools";
                    return_value.error_message = ss.str();
                    break;
                }
                case Token::Type::LOGIC_NOT:
                {
                    return_value.result = !(operand_value.result.to_bool());
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        else if(operand_value.result.get_type() == Value::Type::_NULL)
        {
            switch(node->op_token.type)
            {
                case Token::Type::ADD:
                case Token::Type::SUB:
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform an arithmetic operation on null";
                    return_value.error_message = ss.str();
                    break;
                }
                case Token::Type::LOGIC_NOT:
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform a logical operation on null";
                    return_value.error_message = ss.str();
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        return return_value;
    }
    static VisitorReturn visit_binary_op_node(sourdo_Data* data, std::shared_ptr<BinaryOpNode> node)
    {
        VisitorReturn return_value;
        VisitorReturn left_value = visit_ast(data, node->left_operand);
        if(left_value.error_message)
        {
            return left_value;
        }

        VisitorReturn right_value = visit_ast(data, node->right_operand);
        if(right_value.error_message)
        {
            return right_value;
        }

        if(left_value.result.get_type() == Value::Type::NUMBER)
        {
            if(right_value.result.get_type() == Value::Type::NUMBER)
            {
                double left = left_value.result.to_number();
                double right = right_value.result.to_number();
                switch(node->op_token.type)
                {
                    case Token::Type::ADD:
                    {
                        return_value.result = left + right;
                        break;
                    }
                    case Token::Type::SUB:
                    {   
                        return_value.result = left - right;
                        break;
                    }
                    case Token::Type::MUL:
                    {
                        return_value.result = left * right;
                        break;
                    }
                    case Token::Type::DIV:
                    {
                        if(right == 0)
                        {
                            std::stringstream ss;
                            ss << node->position << "Cannot divide a number by zero";
                            return_value.error_message = ss.str();
                            break;
                        }
                        return_value.result = left / right;
                        break;
                    }
                    case Token::Type::POW:
                    {
                        return_value.result = std::pow(left, right);
                        break;
                    }
                    case Token::Type::LESS_THAN:
                    {
                        return_value.result = left < right;
                        break;
                    }
                    case Token::Type::GREATER_THAN:
                    {
                        return_value.result = left > right;
                        break;
                    }
                    case Token::Type::LESS_EQUAL:
                    {
                        return_value.result = left <= right;
                        break;
                    }
                    case Token::Type::GREATER_EQUAL:
                    {
                        return_value.result = left >= right;
                        break;
                    }
                    case Token::Type::EQUAL:
                    {
                        return_value.result = left == right;
                        break;
                    }
                    case Token::Type::NOT_EQUAL:
                    {
                        return_value.result = left != right;
                        break;
                    }
                    case Token::Type::LOGIC_OR:
                    case Token::Type::LOGIC_AND:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform a logical operation on numbers";
                        return_value.error_message = ss.str();
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            else if(right_value.result.get_type() == Value::Type::BOOL)
            {
                double left = left_value.result.to_number();
                bool right = right_value.result.to_bool();

                switch(node->op_token.type)
                {
                    case Token::Type::ADD:
                    case Token::Type::SUB:
                    case Token::Type::MUL:
                    case Token::Type::DIV:
                    case Token::Type::POW:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform an arithmetic operation with numbers and bools";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::LESS_THAN:
                    case Token::Type::GREATER_THAN:
                    case Token::Type::LESS_EQUAL:
                    case Token::Type::GREATER_EQUAL:
                    case Token::Type::EQUAL:
                    case Token::Type::NOT_EQUAL:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform a comparison operation with numbers and bools";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::LOGIC_OR:
                    case Token::Type::LOGIC_AND:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform a logical operation with numbers and bools";
                        return_value.error_message = ss.str();
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            else if(right_value.result.get_type() == Value::Type::_NULL)
            {
                switch(node->op_token.type)
                {
                    case Token::Type::ADD:
                    case Token::Type::SUB:
                    case Token::Type::MUL:
                    case Token::Type::DIV:
                    case Token::Type::POW:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform an arithmetic operation with number and null";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::LESS_THAN:
                    case Token::Type::GREATER_THAN:
                    case Token::Type::LESS_EQUAL:
                    case Token::Type::GREATER_EQUAL:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform this comparison operation with number and null";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::EQUAL:
                    {
                        return_value.result = false;
                        break;
                    }
                    case Token::Type::NOT_EQUAL:
                    {
                        return_value.result = true;
                        break;
                    }
                    case Token::Type::LOGIC_OR:
                    case Token::Type::LOGIC_AND:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform a logical operation with number and null";
                        return_value.error_message = ss.str();
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
        else if(left_value.result.get_type() == Value::Type::BOOL)
        {
            if(right_value.result.get_type() == Value::Type::NUMBER)
            {
                double left = left_value.result.to_bool();
                double right = right_value.result.to_number();
                switch(node->op_token.type)
                {
                    case Token::Type::ADD:
                    case Token::Type::SUB:
                    case Token::Type::MUL:
                    case Token::Type::DIV:
                    case Token::Type::POW:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform an arithmetic operation with bools and numbers";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::LESS_THAN:
                    case Token::Type::GREATER_THAN:
                    case Token::Type::LESS_EQUAL:
                    case Token::Type::GREATER_EQUAL:
                    case Token::Type::EQUAL:
                    case Token::Type::NOT_EQUAL:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform a comparison operation with bools and numbers";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::LOGIC_OR:
                    case Token::Type::LOGIC_AND:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform a logical operation with bools and numbers";
                        return_value.error_message = ss.str();
                        break;
                    } 
                    default:
                    {
                        break;
                    }
                }
            }
            else if(right_value.result.get_type() == Value::Type::BOOL)
            {
                bool left = left_value.result.to_bool();
                bool right = right_value.result.to_bool();

                switch(node->op_token.type)
                {
                    case Token::Type::ADD:
                    case Token::Type::SUB:
                    case Token::Type::MUL:
                    case Token::Type::DIV:
                    case Token::Type::POW:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform an arithmetic operation with bools and numbers";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::LESS_THAN:
                    case Token::Type::GREATER_THAN:
                    case Token::Type::LESS_EQUAL:
                    case Token::Type::GREATER_EQUAL:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform this comparison operation with bools and numbers";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::EQUAL:
                    {
                        return_value.result = left == right;
                        break;
                    }
                    case Token::Type::NOT_EQUAL:
                    {
                        return_value.result = left != right;
                        break;
                    }
                    case Token::Type::LOGIC_OR:
                    {
                        return_value.result = left || right;
                        break;
                    }
                    case Token::Type::LOGIC_AND:
                    {
                        return_value.result = left && right;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            else if(right_value.result.get_type() == Value::Type::_NULL)
            {
                switch(node->op_token.type)
                {
                    case Token::Type::ADD:
                    case Token::Type::SUB:
                    case Token::Type::MUL:
                    case Token::Type::DIV:
                    case Token::Type::POW:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform an arithmetic operation with bool and null";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::LESS_THAN:
                    case Token::Type::GREATER_THAN:
                    case Token::Type::LESS_EQUAL:
                    case Token::Type::GREATER_EQUAL:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform this comparison operation with bool and null";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::EQUAL:
                    {
                        return_value.result = false;
                        break;
                    }
                    case Token::Type::NOT_EQUAL:
                    {
                        return_value.result = true;
                        break;
                    }
                    case Token::Type::LOGIC_OR:
                    case Token::Type::LOGIC_AND:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform a logical operation with bool and null";
                        return_value.error_message = ss.str();
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
        else if(left_value.result.get_type() == Value::Type::_NULL)
        {
            if(right_value.result.get_type() == Value::Type::NUMBER)
            {
                switch(node->op_token.type)
                {
                    case Token::Type::ADD:
                    case Token::Type::SUB:
                    case Token::Type::MUL:
                    case Token::Type::DIV:
                    case Token::Type::POW:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform an arithmetic operation with null and numbers";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::LESS_THAN:
                    case Token::Type::GREATER_THAN:
                    case Token::Type::LESS_EQUAL:
                    case Token::Type::GREATER_EQUAL:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform this comparison operation with bools";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::EQUAL:
                    {
                        return_value.result = false;
                        break;
                    }
                    case Token::Type::NOT_EQUAL:
                    {
                        return_value.result = true;
                        break;
                    }
                    case Token::Type::LOGIC_OR:
                    case Token::Type::LOGIC_AND:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform a logical operation with null and numbers";
                        return_value.error_message = ss.str();
                        break;
                    } 
                    default:
                    {
                        break;
                    }
                }
            }
            else if(right_value.result.get_type() == Value::Type::BOOL)
            {
                switch(node->op_token.type)
                {
                    case Token::Type::ADD:
                    case Token::Type::SUB:
                    case Token::Type::MUL:
                    case Token::Type::DIV:
                    case Token::Type::POW:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform an arithmetic operation with bools";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::LESS_THAN:
                    case Token::Type::GREATER_THAN:
                    case Token::Type::LESS_EQUAL:
                    case Token::Type::GREATER_EQUAL:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform this comparison operation with null and bools";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::EQUAL:
                    {
                        return_value.result = false;
                        break;
                    }
                    case Token::Type::NOT_EQUAL:
                    {
                        return_value.result = true;
                        break;
                    }
                    case Token::Type::LOGIC_OR:
                    case Token::Type::LOGIC_AND:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform a logical operation with null and bools";
                        return_value.error_message = ss.str();
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }

                return return_value;
            }
            else if(right_value.result.get_type() == Value::Type::_NULL)
            {
                switch(node->op_token.type)
                {
                    case Token::Type::ADD:
                    case Token::Type::SUB:
                    case Token::Type::MUL:
                    case Token::Type::DIV:
                    case Token::Type::POW:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform an arithmetic operation with nulls";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::LESS_THAN:
                    case Token::Type::GREATER_THAN:
                    case Token::Type::LESS_EQUAL:
                    case Token::Type::GREATER_EQUAL:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform this comparison operation with nulls";
                        return_value.error_message = ss.str();
                        break;
                    }
                    case Token::Type::EQUAL:
                    {
                        return_value.result = true;
                        break;
                    }
                    case Token::Type::NOT_EQUAL:
                    {
                        return_value.result = false;
                        break;
                    }
                    case Token::Type::LOGIC_OR:
                    case Token::Type::LOGIC_AND:
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot perform a logical operation with nulls";
                        return_value.error_message = ss.str();
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }

                return return_value;
            }
        }

        return return_value;
    }

    VisitorReturn visit_ast(sourdo_Data* data, std::shared_ptr<Node> node)
    {
        VisitorReturn return_value;
        switch(node->type)
        {
            case Node::Type::STATEMENT_LIST_NODE:
            {
                return_value = visit_statement_list_node(data, std::static_pointer_cast<StatementListNode>(node));
                break;
            }
            case Node::Type::IF_NODE:
            {
                return_value = visit_if_node(data, std::static_pointer_cast<IfNode>(node));
                break;
            }
            case Node::Type::VAR_DECLARATION_NODE:
            {
                return_value = visit_var_declaration_node(data, std::static_pointer_cast<VarDeclarationNode>(node));
                break;
            }
            case Node::Type::VAR_ASSIGNMENT_NODE:
            {
                return_value = visit_var_assignment_node(data, std::static_pointer_cast<VarAssignmentNode>(node));
                break;
            }
            case Node::Type::VAR_ACCESS_NODE:
            {
                return_value = visit_var_access_node(data, std::static_pointer_cast<VarAccessNode>(node));
                break;
            }
            case Node::Type::NUMBER_VALUE_NODE:
            {
                auto number_node = std::static_pointer_cast<NumberValueNode>(node);
                return_value.result = std::stod(number_node->value.value);
                break;
            }
            case Node::Type::BOOL_VALUE_NODE:
            {
                auto bool_node = std::static_pointer_cast<BoolValueNode>(node);
                return_value.result = bool_node->value.value == "true";
                break;
            }
            case Node::Type::NULL_VALUE_NODE:
            {
                return_value.result = Null(); 
                break;
            }
            case Node::Type::UNARY_OP_NODE:
            {
                return_value = visit_unary_op_node(data, std::static_pointer_cast<UnaryOpNode>(node));
                break;
            }
            case Node::Type::BINARY_OP_NODE:
            {
                return_value = visit_binary_op_node(data, std::static_pointer_cast<BinaryOpNode>(node));
                break;
            }
            default:
            {
                // Just warns me that I missed something.
                throw std::logic_error("A node hasn't been implemented yet!");
                break;
            }
        }
        return return_value;
    }
} // namespace sourdo
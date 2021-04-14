#include "Interpreter.hpp"

#include <cmath>
#include <sstream>

#include "SourDoData.hpp"

namespace sourdo
{
    template<typename Left, typename Right>
    static VisitorReturn perform_binary_operation(Token op, Left left, Right right)
    {
        VisitorReturn return_value;
        switch(op.type)
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
                    return_value.error_message = "Cannot divide a number by zero";
                    break;
                }
                return_value.result = left / right;
                break;
            }
            case Token::Type::POW:
            {
                if(std::is_same<Left, int>::value && std::is_same<Right, int>::value)
                {
                    return_value.result = (int)(std::powf(left, right));
                    break;
                }
                return_value.result = std::powf(left, right);
                break;
            }
            default:
            {
                break;
            }
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

        if(operand_value.result.get_type() == Value::Type::INT)
        {
            if(node->op_token.type == Token::Type::SUB)
            {
                return_value.result = -(return_value.result.to_int());
            }
        }
        else if(operand_value.result.get_type() == Value::Type::FLOAT)
        {
            if(node->op_token.type == Token::Type::SUB)
            {
                return_value.result = -(operand_value.result.to_float());
            }
        }
        else
        {
            return_value.error_message = "Expected a numeric type";
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

        if(left_value.result.get_type() == Value::Type::INT)
        {
            if(right_value.result.get_type() == Value::Type::INT)
            {
                return_value = perform_binary_operation(node->op_token, left_value.result.to_int(), right_value.result.to_int());
                return return_value;
            }
            else if(right_value.result.get_type() == Value::Type::FLOAT)
            {
                return_value = perform_binary_operation(node->op_token, left_value.result.to_int(), right_value.result.to_float());
                return return_value;
            }
        }
        else if(left_value.result.get_type() == Value::Type::FLOAT)
        {
            if(right_value.result.get_type() == Value::Type::INT)
            {
                return_value = perform_binary_operation(node->op_token, left_value.result.to_float(), right_value.result.to_int());
                return return_value;
            }
            else if(right_value.result.get_type() == Value::Type::FLOAT)
            {
                return_value = perform_binary_operation(node->op_token, left_value.result.to_float(), right_value.result.to_float());
                return return_value;
            }
        }
        return_value.error_message = "Expected a numeric type";
        return return_value;
    }

    static VisitorReturn visit_var_declaration_node(sourdo_Data* data, std::shared_ptr<VarDeclarationNode> node)
    {
        VisitorReturn return_value;
        VisitorReturn var_value = visit_ast(data, node->initializer);
        if(var_value.error_message)
        {
            return var_value;
        }

        if(data->symbol_table.find(node->name_tok.value) != data->symbol_table.end())
        {
            std::stringstream ss;
            ss << "Variable '" << node->name_tok.value << "' is already defined";
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

        if(data->symbol_table.find(node->name_tok.value) == data->symbol_table.end())
        {
            std::stringstream ss;
            ss << "Variable '" << node->name_tok.value << "' is not defined";
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
        if(data->symbol_table.find(node->name_tok.value) == data->symbol_table.end())
        {
            std::stringstream ss;
            ss << "Variable '" << node->name_tok.value << "' is not defined";
            return_value.error_message = ss.str();
        }
        else
        {
            return_value.result = data->symbol_table[node->name_tok.value];
        }
        return return_value;
    }

    VisitorReturn visit_ast(sourdo_Data* data, std::shared_ptr<Node> node)
    {
        VisitorReturn return_value;
        switch(node->type)
        {
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
            case Node::Type::INT_VALUE_NODE:
            {
                auto int_node = std::static_pointer_cast<IntValueNode>(node);
                return_value.result = std::stoi(int_node->value.value);
                break;
            }
            case Node::Type::FLOAT_VALUE_NODE:
            {
                auto float_node = std::static_pointer_cast<IntValueNode>(node);
                return_value.result = std::stof(float_node->value.value);
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

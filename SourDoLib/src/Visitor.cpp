#include "Interpreter.hpp"

#include <cmath>

#include <iostream>

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
                std::cout << "LEFT: " << left << " RIGHT: " << right << std::endl;
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

    static VisitorReturn visit_literal_node(sourdo_Data* data, std::shared_ptr<LiteralNode> node)
    {
        VisitorReturn return_value;
        switch(node->token.type)
        {
            case Token::Type::INT_LITERAL:
            {
                return_value.result = std::stoi(node->token.value);
                break;
            }
            case Token::Type::FLOAT_LITERAL:
            {
                return_value.result = std::stof(node->token.value);
                break;
            }
            default:
            {
                return_value.error_message = "This should not happen! Token types INT_LITERAL or FLOAT_LITERAL were expected.";
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
            return_value.error_message = "Expected a numeric type.";
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
        return_value.error_message = "Expected a numeric type.";
        return return_value;
    }

    VisitorReturn visit_ast(sourdo_Data* data, std::shared_ptr<Node> node)
    {
        VisitorReturn return_value;
        switch(node->type)
        {
            case Node::Type::LITERAL_NODE:
            {
                return_value = visit_literal_node(data, std::static_pointer_cast<LiteralNode>(node));
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
                break;
        }
        return return_value;
    }
} // namespace sourdo

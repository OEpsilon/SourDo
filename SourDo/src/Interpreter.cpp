#include "Interpreter.hpp"

namespace SourDo {
    Variant Interpreter::travel_node(std::shared_ptr<Node> node)
    {
        Variant value;
        switch(node->type)
        {
            case Node::Type::BinaryOpNode:
                value = travel_binary_op_node(std::static_pointer_cast<BinaryOpNode>(node));
                break;
            case Node::Type::LiteralNode:
                value = travel_literal_node(std::static_pointer_cast<LiteralNode>(node));
                break;
            default:
                throw std::runtime_error("Unknown node type");
                break;
        }
        return value;
    }

    Variant Interpreter::travel_binary_op_node(std::shared_ptr<BinaryOpNode> node)
    {
        Variant left = travel_node(node->left_operand);
        Variant right = travel_node(node->right_operand);
        Variant result;

        switch(node->op_token.type)
        {
            case Token::Type::PLUS:
                result = left + right;
                break;
            case Token::Type::MINUS:
                result = left - right;
                break;
            default:
                break;
        }
        return result;
    }
    
    Variant Interpreter::travel_literal_node(std::shared_ptr<LiteralNode> node)
    {
        
        if(node->token.type == Token::Type::INT)
        {
            return std::stoi(node->token.value);
        }
        return std::stod(node->token.value);
    }
} // namespace SourDo

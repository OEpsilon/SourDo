#include "Interpreter.hpp"

namespace SourDo {
    InterpreterResult Interpreter::travel_ast(std::shared_ptr<Node> ast)
    {
        Variant result = travel_node(ast);
        return {result, error};
    }

    Variant Interpreter::travel_node(std::shared_ptr<Node> node)
    {
        Variant value;
        switch(node->type)
        {
            case Node::Type::BINARY_OP_NODE:
                value = travel_binary_op_node(std::static_pointer_cast<BinaryOpNode>(node));
                break;
            case Node::Type::LITERAL_NODE:
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
        if(error)
        {
            return NullType();
        }

        Variant right = travel_node(node->right_operand);
        if(error)
        {
            return NullType();
        }

        VariantResult result;

        switch(node->op_token.type)
        {
            case Token::Type::PLUS:
                result = left + right;
                error = result.error;
                break;
            case Token::Type::MINUS:
                result = left - right;
                error = result.error;
                break;
            case Token::Type::MULTI:
                result = left * right;
                error = result.error;
                break;
            case Token::Type::DIVIDE:
                result = left / right;
                error = result.error;
                break;
            case Token::Type::POWER:
                result = left.power(right);
                error = result.error;
                break;
            default:
                error = Error("Unknown operation. This error shouldn't happen", 
                        node->op_token.position);
                break;
        }
        if(error)
        {
            return NullType();
        }

        return result.result;
    }

    Variant Interpreter::travel_unary_op_node(std::shared_ptr<UnaryOpNode> node)
    {
        Variant operand = travel_node(node->operand);
        if(error)
        {
            return NullType();
        }

        if(node->op_token.type == Token::Type::MINUS)
        {
            VariantResult result = operand * -1;
            error = result.error;
            operand = result.result;
        }
        if(error)
        {
            return NullType();
        }

        return operand;
    }
    
    Variant Interpreter::travel_literal_node(std::shared_ptr<LiteralNode> node)
    {
        Variant value;
        if(node->token.type == Token::Type::INT)
        {
            value = std::stoi(node->token.value);
        }
        else
        {
            value = std::stod(node->token.value);
        }
        std::cout << node->token.position.line 
                << ", " << node->token.position.column << "\n";
        value.set_position(node->token.position);
        return value;
    }
} // namespace SourDo

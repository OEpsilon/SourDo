#pragma once

#include "DataTypes/Nodes.hpp"
#include "DataTypes/Variant.hpp"

namespace SourDo {
    struct InterpreterResult 
    {
        Variant result;
        std::optional<Error> error;
    };

    class Interpreter 
    {
    public:
        InterpreterResult travel_ast(std::shared_ptr<Node> ast);
    private:
        std::optional<Error> error;

        Variant travel_node(std::shared_ptr<Node> node);
        Variant travel_binary_op_node(std::shared_ptr<BinaryOpNode> node);
        Variant travel_unary_op_node(std::shared_ptr<UnaryOpNode> node);
        Variant travel_literal_node(std::shared_ptr<LiteralNode> node);
    };
} // namespace SourDo


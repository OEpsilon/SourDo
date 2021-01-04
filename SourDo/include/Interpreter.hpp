#pragma once

#include "DataTypes/Nodes.hpp"
#include "DataTypes/Variant.hpp"

namespace SourDo {
    class Interpreter 
    {
    public:
        Variant travel_node(std::shared_ptr<Node> node);
    private:
        Variant travel_binary_op_node(std::shared_ptr<BinaryOpNode> node);
        Variant travel_literal_node(std::shared_ptr<LiteralNode> node);
    };
} // namespace SourDo


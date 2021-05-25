#pragma once

#include "Bytecode.hpp"
#include "../Datatypes/Nodes.hpp"

#include <optional>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace sourdo
{ 
    class BytecodeGenerator
    {
    public:
        struct Result
        {
            Bytecode bytecode;
            std::optional<std::string> error;
        };

        Result generate_bytecode(std::shared_ptr<Node> ast);
    private:
        std::optional<std::string> error;

        uint64_t push_constant(const Value& val, Bytecode& bytecode);

        void visit_node(std::shared_ptr<Node> node, Bytecode& bytecode);
        void visit_statement_list_node(std::shared_ptr<StatementListNode> node, Bytecode& bytecode);
        void visit_if_node(std::shared_ptr<IfNode> node, Bytecode& bytecode);

        void visit_var_declaration_node(std::shared_ptr<VarDeclarationNode> node, Bytecode& bytecode);
        void visit_assignment_node(std::shared_ptr<AssignmentNode> node, Bytecode& bytecode);
        void visit_func_node(std::shared_ptr<FuncNode> node, Bytecode& bytecode);
        void visit_return_node(std::shared_ptr<ReturnNode> node, Bytecode& bytecode);

        void visit_call_node(std::shared_ptr<CallNode> node, Bytecode& bytecode);
        void visit_binary_op_node(std::shared_ptr<BinaryOpNode> node, Bytecode& bytecode);
        void visit_number_node(std::shared_ptr<NumberNode> node, Bytecode& bytecode);
        void visit_string_node(std::shared_ptr<StringNode> node, Bytecode& bytecode);
        void visit_bool_node(std::shared_ptr<BoolNode> node, Bytecode& bytecode);
        void visit_null_node(std::shared_ptr<NullNode> node, Bytecode& bytecode);
        void visit_identifier_node(std::shared_ptr<IdentifierNode> node , Bytecode& bytecode);
    };
} // namespace sourdo

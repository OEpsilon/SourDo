#pragma once

#include "../Datatypes/Value.hpp"

#include <cstdint>
#include <optional>
#include <vector>
#include <ostream>

namespace sourdo
{
    enum Opcode : uint64_t
    {
        OP_PUSH_NUMBER,
        OP_PUSH_STRING,
        OP_PUSH_BOOL,
        OP_PUSH_NULL,
        OP_SYM_CREATE,
        OP_SYM_GET,
        OP_SYM_SET,
        OP_POP,
        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OP_MOD,
        OP_POW,
        OP_CALL,
    };

    struct Instruction
    {
        Instruction(Opcode op, std::optional<uint64_t> operand = {})
            : op(op), operand(operand)
        {
        }

        Opcode op;
        std::optional<uint64_t> operand;
    };
    
    struct Bytecode
    {
        std::string file_name;
        std::string scope_name;
        std::vector<Instruction> instructions;
        std::vector<Value> constants;
    };

    std::ostream& operator<<(std::ostream& os, Opcode op);
    std::ostream& operator<<(std::ostream& os, const Bytecode& bytecode);
} // namespace sourdo

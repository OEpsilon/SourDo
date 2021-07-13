#pragma once

#include "../Datatypes/Value.hpp"

#include <cstdint>
#include <optional>
#include <vector>
#include <ostream>

namespace sourdo
{
    enum Opcode : uint8_t
    {
        OP_PUSH_NUMBER,
        OP_PUSH_STRING,
        OP_PUSH_BOOL,
        OP_PUSH_NULL,
        OP_PUSH_FUNC,

        OP_CREATE_SUBTYPE,
        OP_CREATE_TYPE,
        OP_SET_SETTER,
        OP_SET_GETTER,
        OP_SET_METHOD,
        OP_SET_CLASS_PROP,
        OP_SET_INITIALIZER,
        OP_GET_INITIALIZER,
        OP_FINISH_TYPE,

        OP_ALLOC_OBJECT,
        OP_ADD_PROPERTY,
        OP_ADD_CONST_PROPERTY,

        OP_STACK_GET,
        OP_STACK_GET_TOP,
        OP_SYM_CREATE,
        OP_SYM_CONST,
        OP_SYM_GET,
        OP_SYM_SET,

        OP_AlLOC_TABLE,
        OP_VAL_SET,
        OP_VAL_GET,

        OP_JMP,
        OP_NJMP,

        OP_PUSH_SCOPE,
        OP_POP_SCOPE,
        OP_POP,
        OP_REMOVE_TOP,

        OP_TYPE_CHECK,
        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OP_MOD,
        OP_POW,
        OP_NEG,

        OP_EQ,
        OP_NE,
        OP_LT,
        OP_LE,
        OP_GT,
        OP_GE,

        OP_OR,
        OP_AND,
        OP_NOT,

        OP_CALL,
        OP_RET,
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

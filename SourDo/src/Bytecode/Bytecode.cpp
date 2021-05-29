#include "Bytecode.hpp"

#include "../Datatypes/Function.hpp"

#include <string>
#include <functional>

namespace sourdo
{
    std::ostream& operator<<(std::ostream& os, Opcode op)
    {
        switch(op)
        {
            case OP_PUSH_NUMBER: 
                os << "push_num"; 
                break;
            case OP_PUSH_STRING: 
                os << "push_str"; 
                break;
            case OP_PUSH_BOOL: 
                os << "push_bool"; 
                break;
            case OP_PUSH_NULL: 
                os << "push_null"; 
                break;
            case OP_PUSH_FUNC: 
                os << "push_func"; 
                break;
            case OP_STACK_GET: 
                os << "stack_get"; 
                break;
            case OP_STACK_GET_TOP:
                os << "stack_get_top";
                break;
            case OP_SYM_CREATE:
                os << "sym_create"; 
                break;
            case OP_SYM_CONST:
                os << "sym_const"; 
                break;
            case OP_SYM_GET:
                os << "sym_get"; 
                break;
            case OP_SYM_SET:
                os << "sym_set"; 
                break;
            case OP_AlLOC_TABLE:
                os << "alloc_table"; 
                break;
            case OP_VAL_SET:
                os << "val_set"; 
                break;
            case OP_VAL_GET:
                os << "val_get"; 
                break;
            case OP_JMP:
                os << "jmp"; 
                break;
            case OP_NJMP:
                os << "njmp"; 
                break;
            case OP_PUSH_SCOPE:
                os << "push_scope";
                break;
            case OP_POP_SCOPE:
                os << "pop_scope";
                break;
            case OP_POP: 
                os << "pop"; 
                break;
            case OP_TYPE_CHECK:
                os << "type_check";
                break;
            case OP_ADD: 
                os << "add"; 
                break;
            case OP_SUB: 
                os << "sub"; 
                break;
            case OP_MUL: 
                os << "mul"; 
                break;
            case OP_DIV: 
                os << "div"; 
                break;
            case OP_MOD: 
                os << "mod"; 
                break;
            case OP_POW: 
                os << "pow";
                break;
            case OP_NEG: 
                os << "neg";
                break;
            case OP_EQ:
                os << "eq";
                break;
            case OP_NE:
                os << "ne";
                break;
            case OP_LT:
                os << "lt";
                break;
            case OP_LE:
                os << "le";
                break;
            case OP_GT:
                os << "gt";
                break;
            case OP_GE:
                os << "ge";
                break;
            case OP_OR:
                os << "or";
                break;
            case OP_AND:
                os << "and";
                break;
            case OP_NOT:
                os << "not";
                break;
            case OP_CALL: 
                os << "call"; 
                break;
            case OP_RET: 
                os << "ret"; 
                break;
        }
        return os;
    }

    static void print_bytecode(std::ostream& os, std::function<void(std::ostream&)> print_name, const Bytecode& bytecode, uint32_t indent)
    {
        print_name(os);
        os << ":\n";
        for(int i = 0; i < indent; i++) os << "\t";
        os << "INSTRUCTIONS for ";
        print_name(os);
        os << ":\n";
        for(int i = 0; i < bytecode.instructions.size(); i++)
        {
            for(int j = 0; j < indent + 1; j++) os << "\t";

            os << "[" << i << "]" << "\t" << bytecode.instructions[i].op;
            if(bytecode.instructions[i].operand)
            {
                static const std::array<Opcode, 17> multi_tab = 
                {
                    OP_JMP,
                    OP_NJMP,
                    OP_POP,
                    OP_ADD,
                    OP_SUB,
                    OP_MUL,
                    OP_DIV,
                    OP_MOD,
                    OP_POW,
                    OP_EQ,
                    OP_LT,
                    OP_LE,
                    OP_OR,
                    OP_AND,
                    OP_NOT,
                    OP_CALL,
                    OP_RET,
                };
                if(std::find(multi_tab.begin(), multi_tab.end(), 
                        bytecode.instructions[i].op) != multi_tab.end())
                {
                    os << ",\t\t";
                }
                else
                {
                    os << ",\t";
                }
                os << bytecode.instructions[i].operand.value();
            }
            os << "\n";
        }
        for(int i = 0; i < indent; i++) os << "\t";
        os << "CONSTANTS for ";
        print_name(os);
        os << ":\n";
        for(int i = 0; i < bytecode.constants.size(); i++)
        {
            for(int j = 0; j < indent + 1; j++) os << "\t";
            os << "[" << i << "]\t";

            if(bytecode.constants[i].get_type() == ValueType::SOURDO_FUNCTION)
            {
                print_bytecode(os, [bytecode, i](std::ostream& os) -> void
                    {
                        os << bytecode.constants[i].to_sourdo_function();
                    },
                    bytecode.constants[i].to_sourdo_function()->bytecode, indent + 1
                );
                continue;
            }
            os << bytecode.constants[i] << "\n";
        }
        for(int i = 0; i < indent; i++) os << "\t";
        os << "END ";
        print_name(os);
        os << "\n";
    }

    std::ostream& operator<<(std::ostream& os, const Bytecode& bytecode)
    {
        os << "Bytecode from file: " << bytecode.file_name << "\n";
        print_bytecode(os, [](std::ostream& os) -> void
            {
                os << "MAIN";
            },
            bytecode, 0
        );
        return os;
    }
} // namespace sourdo

#include "Bytecode.hpp"

#include <string>

namespace sourdo
{
    std::ostream& operator<<(std::ostream& os, Opcode op)
    {
        switch(op)
        {
            case OP_PUSH_NUMBER: 
                os << "push_number"; 
                break;
            case OP_PUSH_STRING: 
                os << "push_string"; 
                break;
            case OP_PUSH_BOOL: 
                os << "push_bool"; 
                break;
            case OP_PUSH_NULL: 
                os << "push_null"; 
                break;
            case OP_SYM_CREATE:
                os << "sym_create"; 
                break;
            case OP_SYM_GET:
                os << "sym_get"; 
                break;
            case OP_SYM_SET:
                os << "sym_set"; 
                break;
            case OP_POP: 
                os << "pop"; 
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
            case OP_CALL: 
                os << "call"; 
                break;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const Bytecode& bytecode)
    {
        os << "Bytecode from file: " << bytecode.file_name << "\n";
        os << "MAIN:\n";
        for(int i = 0; i < bytecode.instructions.size(); i++)
        {
            os << "\t[" << i << "]" << "\t\t" << bytecode.instructions[i].op;
            if(bytecode.instructions[i].operand)
            {
                os << ",  " << bytecode.instructions[i].operand.value();
            }
            os << "\n";
        }
        os << "CONSTANTS for MAIN:\n";
        for(int i = 0; i < bytecode.constants.size(); i++)
        {
            os << "\t[" << i << "]\t\t" << bytecode.constants[i] << "\n";
        }
        return os;
    }
} // namespace sourdo

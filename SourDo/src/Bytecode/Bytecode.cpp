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
            case OP_SYM_GET:
                os << "sym_get"; 
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

    std::ostream& operator<<(std::ostream& os, const std::vector<Instruction>& ir)
    {
        for(auto& instruction : ir)
        {
            os << instruction.op << " ";
            if(instruction.operand)
            {
                os << instruction.operand.value();
            }
            os << "\n";
        }
        return os;
    }
} // namespace sourdo

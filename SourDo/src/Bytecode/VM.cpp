#include "VM.hpp"

#include "../Datatypes/Position.hpp"

#include <cmath>

namespace sourdo
{
    std::optional<std::string> VirtualMachine::run_bytecode(const Bytecode& bytecode, Data::Impl* data)
    {
        /* Currently, file positions are not logged in runtime error messages.
         * This should be fixed using some extra debug information in the bytecode.
         */
        for(auto& instruction : bytecode.instructions)
        {
            switch(instruction.op)
            {
                case OP_PUSH_NUMBER:
                {
                    data->stack.emplace_back(bytecode.constants[instruction.operand.value()]);
                    break;
                }
                case OP_PUSH_STRING:
                {
                    data->stack.emplace_back(bytecode.constants[instruction.operand.value()]);
                    break;
                }
                case OP_PUSH_BOOL:
                {
                    data->stack.emplace_back(bytecode.constants[instruction.operand.value()]);
                    break;
                }
                case OP_PUSH_NULL:
                {
                    data->stack.emplace_back(Null());
                    break;
                }
                case OP_POP:
                {
                    data->stack.pop_back();
                    break;
                }
                case OP_ADD:
                {
                    double right = data->index_stack(-1).to_number();
                    data->stack.pop_back();
                    double left = data->index_stack(-1).to_number();
                    data->stack.pop_back();
                    data->stack.emplace_back(left + right);
                    break;
                }
                case OP_SUB:
                {
                    double right = data->index_stack(-1).to_number();
                    data->stack.pop_back();
                    double left = data->index_stack(-1).to_number();
                    data->stack.pop_back();
                    data->stack.emplace_back(left - right);
                    break;
                }
                case OP_MUL:
                {
                    double right = data->index_stack(-1).to_number();
                    data->stack.pop_back();
                    double left = data->index_stack(-1).to_number();
                    data->stack.pop_back();
                    data->stack.emplace_back(left * right);
                    break;
                }
                case OP_DIV:
                {
                    double right = data->index_stack(-1).to_number();
                    if(right == 0)
                    {
                        std::stringstream ss;
                        ss << bytecode.file_name << "(Runtime Error): Cannot divide a number by zero";
                        return ss.str();
                    }
                    data->stack.pop_back();
                    double left = data->index_stack(-1).to_number();
                    data->stack.pop_back();
                    data->stack.emplace_back(left / right);
                    break;
                }
                case OP_MOD:
                {
                    double right = data->index_stack(-1).to_number();
                    data->stack.pop_back();
                    double left = data->index_stack(-1).to_number();
                    data->stack.pop_back();
                    data->stack.emplace_back(std::fmod(left, right));
                    break;
                }
                case OP_POW:
                {
                    double right = data->index_stack(-1).to_number();
                    data->stack.pop_back();
                    double left = data->index_stack(-1).to_number();
                    data->stack.pop_back();
                    data->stack.emplace_back(std::pow(left, right));
                    break;
                }
                case OP_SYM_GET:
                {
                    uint64_t sym_name = instruction.operand.value();
                    if(data->symbol_table.find(bytecode.constants[sym_name].to_string()) == data->symbol_table.end())
                    {
                        std::stringstream ss;
                        ss << bytecode.file_name << "(Runtime Error): '" << bytecode.constants[sym_name].to_string() << "' is undefined";
                        return ss.str();
                    }
                    data->stack.emplace_back(data->symbol_table[bytecode.constants[sym_name].to_string()]);
                    break;
                }
                case OP_CALL:
                {
                    uint64_t param_count = instruction.operand.value();
                    Value func = data->index_stack(-(param_count + 1) );
                    data->stack.erase(data->stack.begin() + (data->stack.size() - (param_count + 1) ) );

                    if(func.get_type() == ValueType::SOURDO_FUNCTION)
                    {
                        // Implement later when I get around to bytecode functions.
                    }
                    else if(func.get_type() == ValueType::CPP_FUNCTION)
                    {
                        Data scope;
                        scope.get_impl()->parent = data->parent;
                        for(int i = -param_count; i < 0; i++)
                        {
                            scope.get_impl()->stack.emplace_back(data->index_stack(i));
                            data->stack.erase(data->stack.begin() + (data->stack.size() + i));
                        }

                        try
                        {
                            bool does_return = func.to_cpp_function()(scope);
                            if(does_return)
                            {
                                data->stack.emplace_back(scope.get_impl()->index_stack(-1));
                            }
                            else
                            {
                                data->stack.emplace_back(Null());
                            }
                        }
                        catch(SourDoError err)
                        {
                            using namespace std::string_literals;
                            return bytecode.file_name + "(Runtime Error): " + err.what();
                        }
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << bytecode.file_name << "(Runtime Error): Value being called is not a function.";
                        return ss.str();
                    }
                    
                    break;
                }
            }

        }
        return {};
    }
} // namespace sourdo
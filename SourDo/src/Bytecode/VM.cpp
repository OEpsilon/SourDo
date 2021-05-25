#include "VM.hpp"

#include "../Datatypes/Position.hpp"
#include "../Datatypes/Function.hpp"

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
                case OP_PUSH_FUNC:
                {
                    data->stack.emplace_back(bytecode.constants[instruction.operand.value()]);
                    break;
                }
                case OP_STACK_GET:
                {
                    data->stack.emplace_back(data->index_stack(instruction.operand.value()));
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
                case OP_SYM_CREATE:
                {
                    Value initializer = data->index_stack(-1);
                    data->stack.pop_back();
                    uint64_t sym_name = instruction.operand.value();
                    if(data->symbol_table.find(bytecode.constants[sym_name].to_string()) != data->symbol_table.end())
                    {
                        std::stringstream ss;
                        ss << bytecode.file_name << "(Runtime Error): '" << bytecode.constants[sym_name].to_string() << "' is already defined";
                        return ss.str();
                    }
                    data->symbol_table[bytecode.constants[sym_name].to_string()] = initializer;
                    break;
                }
                case OP_SYM_GET:
                {
                    uint64_t sym_name = instruction.operand.value();
                    Value* value = data->get_symbol(bytecode.constants[sym_name].to_string());
                    if(value == nullptr)
                    {
                        std::stringstream ss;
                        ss << bytecode.file_name << "(Runtime Error): '" << bytecode.constants[sym_name].to_string() << "' is undefined";
                        return ss.str();
                    }
                    data->stack.emplace_back(*value);
                    break;
                }
                case OP_SYM_SET:
                {
                    uint64_t sym_name = instruction.operand.value();
                    if(data->symbol_table.find(bytecode.constants[sym_name].to_string()) == data->symbol_table.end())
                    {
                        std::stringstream ss;
                        ss << bytecode.file_name << "(Runtime Error): '" << bytecode.constants[sym_name].to_string() << "' is undefined";
                        return ss.str();
                    }
                    std::string str_name = bytecode.constants[sym_name].to_string();
                    Value new_value = data->index_stack(-1);
                    data->stack.pop_back();
                    data->symbol_table[str_name] = new_value;
                    break;
                }
                case OP_CALL:
                {
                    uint64_t param_count = instruction.operand.value();
                    Data scope;
                    scope.get_impl()->parent = data;
                    for(int i = -param_count; i < 0; i++)
                    {
                        scope.get_impl()->stack.emplace_back(data->index_stack(i));
                        data->stack.erase(data->stack.begin() + (data->stack.size() + i));
                    }

                    Value func = data->index_stack(-1);
                    data->stack.pop_back();

                    if(func.get_type() == ValueType::SOURDO_FUNCTION)
                    {
                        bool saved_state = is_function;
                        is_function = true;
                        std::optional<std::string> error = run_bytecode(func.to_sourdo_function()->bytecode, scope.get_impl());
                        if(error)
                        {
                            return error;
                        }
                        is_function = saved_state;
                        data->stack.emplace_back(scope.get_impl()->index_stack(-1));
                    }
                    else if(func.get_type() == ValueType::CPP_FUNCTION)
                    {
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
                            return bytecode.file_name + "(Runtime Error): " + err.what();
                        }
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << bytecode.file_name << "(Runtime Error): Value being called is not a function";
                        return ss.str();
                    }
                    break;
                }
                case OP_RET:
                {
                    if(!is_function)
                    {
                        std::stringstream ss;
                        ss << bytecode.file_name << "(Runtime Error): Cannot return when outside of a function";
                        return ss.str();
                    }
                    return {};
                    break;
                }
            }

        }
        return {};
    }
} // namespace sourdo

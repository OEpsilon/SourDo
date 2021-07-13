#include "VM.hpp"

#include "../Datatypes/Position.hpp"
#include "../Datatypes/Function.hpp"
#include "../VisitorTypeFunctions/StringFunctions.hpp"
#include "../VisitorTypeFunctions/TableFunctions.hpp"

#include "../GlobalData.hpp"
#include "../GarbageCollector.hpp"

#include <cmath>

namespace sourdo
{
    static std::ostream& operator<<(std::ostream& os, const ValueType& type)
    {
        switch(type)
        {
            case ValueType::NUMBER:
                os << "number";
                break;
            case ValueType::BOOL:
                os << "bool";
                break;
            case ValueType::STRING:
                os << "string";
                break;
            case ValueType::SOURDO_FUNCTION:
            case ValueType::CPP_FUNCTION:
                os << "function";
                break;
            case ValueType::_NULL:
                os << "null";
                break;
            // Note: this case should never occour as value_ref's are dereferenced when using on them
            case ValueType::VALUE_REF:
                os << "value_ref";
                break;
            case ValueType::OBJECT:
                os << "object";
                break;
            case ValueType::TABLE:
                os << "table";
                break;
            case ValueType::CLASS_TYPE:
                os << "class_type";
                break;
            case ValueType::CPP_OBJECT:
                os << "CppObject";
                break;
        }
        return os;
    }

    std::optional<std::string> VirtualMachine::run_bytecode(const Bytecode& bytecode, Data::Impl* data)
    {
        #define UNPACK_REF(var_name) if(var_name.get_type() == ValueType::VALUE_REF) var_name = *(var_name.to_value_ref())

        /* Currently, file positions are not logged in runtime error messages.
         * This should be fixed by adding some extra debug information in the bytecode.
         */
        while(ipointer < bytecode.instructions.size())
        {
            auto& instruction = bytecode.instructions[ipointer];
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
                case OP_CREATE_SUBTYPE:
                {
                    Value& super_type = data->index_stack(-1);
                    if(super_type.get_type() != ValueType::CLASS_TYPE)
                    {
                        std::stringstream ss;
                        ss << bytecode.file_name << "(Runtime Error): Super value is not a class";
                        return ss.str();
                    }

                    data->stack.emplace_back(new ClassType(
                        bytecode.constants[instruction.operand.value()].to_string(),
                        super_type.to_class()
                    ));
                    break;
                }
                case OP_CREATE_TYPE:
                {
                    data->stack.emplace_back(new ClassType(bytecode.constants[instruction.operand.value()].to_string(), nullptr) );
                    break;
                }
                case OP_SET_SETTER:
                {
                    Value& class_type = data->index_stack(-3);
                    if(class_type.to_class()->complete)
                    {
                        break;
                    }
                    Value name = data->index_stack(-2);
                    Value value = data->index_stack(-1);
                    data->stack.pop_back();
                    data->stack.pop_back();
                    
                    class_type.to_class()->setters[name.to_string()] = ClassType::Property(value, class_type.to_class()->name, instruction.operand.value(), true);
                    break;
                }
                case OP_SET_GETTER:
                {
                    Value& class_type = data->index_stack(-3);
                    if(class_type.to_class()->complete)
                    {
                        break;
                    }
                    Value name = data->index_stack(-2);
                    Value value = data->index_stack(-1);
                    data->stack.pop_back();
                    data->stack.pop_back();

                    class_type.to_class()->getters[name.to_string()] = ClassType::Property(value, class_type.to_class()->name, instruction.operand.value(), true);
                    break;
                }
                case OP_SET_METHOD:
                {
                    Value& class_type = data->index_stack(-3);
                    if(class_type.to_class()->complete)
                    {
                        break;
                    }
                    Value name = data->index_stack(-2);
                    Value value = data->index_stack(-1);
                    data->stack.pop_back();
                    data->stack.pop_back();

                    class_type.to_class()->methods[name.to_string()] = ClassType::Property(value, class_type.to_class()->name, instruction.operand.value(), true);
                    break;
                }
                case OP_SET_CLASS_PROP:
                {
                    Value& class_type = data->index_stack(-3);
                    if(class_type.to_class()->complete)
                    {
                        break;
                    }
                    Value name = data->index_stack(-2);
                    Value value = data->index_stack(-1);
                    data->stack.pop_back();
                    data->stack.pop_back();
                    
                    class_type.to_class()->class_methods[name.to_string()] = ClassType::Property(value, class_type.to_class()->name, false, true);
                    break;
                }
                case OP_SET_INITIALIZER:
                {
                    Value& class_type = data->index_stack(-2);
                    if(class_type.to_class()->complete)
                    {
                        break;
                    }
                    Value value = data->index_stack(-1);
                    data->stack.pop_back();

                    class_type.to_class()->initializer = value.to_sourdo_function();
                    break;
                }
                case OP_GET_INITIALIZER:
                {
                    Value& class_type = data->index_stack(-1);
                    data->stack.emplace_back(class_type.to_class()->initializer);
                    break;
                }
                case OP_FINISH_TYPE:
                {
                    data->index_stack(-1).to_class()->complete = true;
                    break;
                }
                case OP_ALLOC_OBJECT:
                {
                    Value& type = data->index_stack(-1);

                    data->stack.emplace_back(new Object(type.to_class()));
                    break;
                }
                case OP_ADD_PROPERTY:
                {
                    Value& object = data->index_stack(-4);
                    std::string class_context = data->index_stack(-3).to_string();
                    Value name = data->index_stack(-2);
                    Value value = data->index_stack(-1);
                    data->stack.pop_back();
                    data->stack.pop_back();
                    data->stack.pop_back();
                    object.to_object()->props[name.to_string()] = ClassType::Property(value, class_context, instruction.operand.value(), false);
                    break;
                }
                case OP_ADD_CONST_PROPERTY:
                {
                    Value& object = data->index_stack(-4);
                    std::string class_context = data->index_stack(-3).to_string();
                    Value name = data->index_stack(-2);
                    Value value = data->index_stack(-1);
                    data->stack.pop_back();
                    data->stack.pop_back();
                    data->stack.pop_back();
                    object.to_object()->props[name.to_string()] = ClassType::Property(value, class_context, instruction.operand.value(), true);
                    break;
                }
                case OP_STACK_GET:
                {
                    data->stack.emplace_back(data->index_stack(instruction.operand.value()));
                    break;
                }
                case OP_STACK_GET_TOP:
                {
                    data->stack.emplace_back(data->index_stack(-instruction.operand.value()));
                    break;
                }
                case OP_SYM_CREATE:
                case OP_SYM_CONST:
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
                    data->symbol_table[bytecode.constants[sym_name].to_string()] = {instruction.op == OP_SYM_CONST, 
                            initializer.get_type() == ValueType::VALUE_REF ? *(initializer.to_value_ref()) : initializer };
                    break;
                }
                case OP_SYM_GET:
                {
                    uint64_t sym_name = instruction.operand.value();
                    std::optional<Value> value = data->get_symbol(bytecode.constants[sym_name].to_string());
                    if(!value)
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
                    std::string str_name = bytecode.constants[sym_name].to_string();
                    Value new_value = data->index_stack(-1);
                    data->stack.pop_back();

                    SetSymbolResult res = data->set_symbol(bytecode.constants[sym_name].to_string(), 
                            new_value.get_type() == ValueType::VALUE_REF ? *(new_value.to_value_ref()) : new_value );
                    if(res == SetSymbolResult::SYM_NOT_FOUND)
                    {
                        std::stringstream ss;
                        ss << bytecode.file_name << "(Runtime Error): '" << bytecode.constants[sym_name].to_string() << "' is undefined";
                        return ss.str();
                    }
                    else if(res == SetSymbolResult::SYM_READONLY)
                    {
                        std::stringstream ss;
                        ss << bytecode.file_name << "(Runtime Error): '" << bytecode.constants[sym_name].to_string() << "' is a constant";
                        return ss.str();
                    }
                    break;
                }
                case OP_AlLOC_TABLE:
                {
                    data->stack.emplace_back(new Table());
                    break;
                }
                case OP_VAL_SET:
                {
                    Value val = data->index_stack(-1);
                    UNPACK_REF(val);
                    data->stack.pop_back();
                    Value key = data->index_stack(-1);
                    UNPACK_REF(key);
                    data->stack.pop_back();
                    Value stack_object = data->index_stack(-1);
                    data->stack.pop_back();
                    Value* object = stack_object.get_type() == ValueType::VALUE_REF? stack_object.to_value_ref(): &stack_object;
                    switch(object->get_type())
                    {
                        case ValueType::CLASS_TYPE:
                        {
                            if(key.get_type() == ValueType::STRING)
                            {
                                ClassType* class_type = object->to_class();
                                if(class_type->class_methods.find(key.to_string()) != class_type->class_methods.end())
                                {
                                    if(class_type->class_methods[key.to_string()].readonly)
                                    {
                                        std::stringstream ss;
                                        ss << "(Runtime Error): Cannot alter the const property '" << key.to_string() << "' of class '" << class_type->name << "'";
                                        return ss.str();
                                    }
                                    class_type->class_methods[key.to_string()].val = val.get_type() == ValueType::VALUE_REF? *(val.to_value_ref()) : val;;
                                    break;
                                }
                                std::stringstream ss;
                                ss << "(Runtime Error): '" << key.to_string() << "' does not exist in class '" << class_type->name << "'";
                                return ss.str();
                                break;
                            }

                            std::stringstream ss;
                            ss << "(Runtime Error): Cannot index a class with a value of type " << key.get_type();
                            return ss.str();
                            break;
                        }
                        case ValueType::OBJECT:
                        {
                            Object* obj = object->to_object();
                            if(key.get_type() == ValueType::STRING)
                            {
                                std::string name = key.to_string();
                                auto it = obj->props.find(name);
                                if(it != obj->props.end())
                                {
                                    if(obj->props[it->first].is_private && current_class_context != it->second.class_context)
                                    {
                                        std::stringstream ss;
                                        ss << "(Runtime Error): Cannot access the private property '" << name << "' outside of the class it is defined in";
                                        return ss.str();
                                    }

                                    obj->props[it->first].val = val;
                                    break;
                                }

                                ClassType* current_type = obj->type;
                                bool value_is_found = false;
                                while(current_type != nullptr)
                                {
                                    it = current_type->setters.find(name);
                                    if(it != current_type->setters.end())
                                    {
                                        if(current_type->setters[it->first].is_private && current_class_context != it->second.class_context)
                                        {
                                            std::stringstream ss;
                                            ss << "(Runtime Error): Cannot access the private setter '" << name << "' outside of the class it is defined in";
                                            return ss.str();
                                        }
                                        data->stack.emplace_back(current_type->setters[it->first].val);
                                        data->stack.emplace_back(obj);
                                        data->stack.emplace_back(val);
                                        std::optional<std::string> error = call_function(bytecode, data, 2);
                                        if(error)
                                        {
                                            return error;
                                        }
                                        value_is_found = true;
                                        break;
                                    }

                                    it = current_type->methods.find(name);
                                    if(it != current_type->methods.end())
                                    {
                                        std::stringstream ss;
                                        if(current_type->methods[it->first].is_private && current_class_context != it->second.class_context)
                                        {
                                            ss << "(Runtime Error): Cannot access the private method '" << name << "' outside of the class it is defined in";
                                            return ss.str();
                                        }

                                        ss << "(Runtime Error): Cannot set the value of '" << name << "' as it is defined as a method'";
                                        return ss.str();
                                    }
                                    current_type = current_type->super;
                                    break;
                                }

                                if(value_is_found)
                                {
                                    break;
                                }

                                std::stringstream ss;
                                ss << "(Runtime Error): '" << name << "' does not exist in object of type '" << obj->type->name << "'";
                                return ss.str();
                            }

                            std::stringstream ss;
                            ss << "(Runtime Error): Cannot index an object with a value of type " << key.get_type();
                            return ss.str();
                            break;
                        }
                        case ValueType::TABLE:
                        {
                            if(key.get_type() == ValueType::STRING && key.to_string() == "has")
                            {
                                std::stringstream ss;
                                ss << "(Runtime Error): 'has' is a built-in method for tables and cannot be changed";
                                return ss.str();
                            }
                            object->to_table()->keys[key] = val.get_type() == ValueType::VALUE_REF? *(val.to_value_ref()) : val;;
                            break;
                        }
                        case ValueType::STRING:
                        {
                            if(key.get_type() == ValueType::NUMBER)
                            {
                                std::stringstream ss;
                                ss << "(Runtime Error): The result from indexing a string cannot be assigned to";
                                return ss.str();
                            }
                            if(key.get_type() == ValueType::STRING)
                            {
                                std::stringstream ss;
                                ss << "(Runtime Error): '" << key.to_string() << "' does not exist in string";
                                return ss.str();
                            }
                            std::stringstream ss;
                            ss << "(Runtime Error): Expected a number";
                            return ss.str();
                            break;
                        }
                        default:
                        {
                            std::stringstream ss;
                            ss << "(Runtime Error): Cannot index value of type " << object->get_type(); 
                            return ss.str();
                            break;
                        }
                    }
                    break;
                }
                case OP_VAL_GET:
                {
                    Value key = data->index_stack(-1);
                    UNPACK_REF(key);
                    data->stack.pop_back();
                    Value stack_object = data->index_stack(-1);
                    data->stack.pop_back();
                    Value* object = stack_object.get_type() == ValueType::VALUE_REF? stack_object.to_value_ref(): &stack_object;
                    switch(object->get_type())
                    {
                        case ValueType::CLASS_TYPE:
                        {
                            if(key.get_type() == ValueType::STRING)
                            {
                                ClassType* class_type = object->to_class();
                                if(class_type->class_methods.find(key.to_string()) != class_type->class_methods.end())
                                {
                                    data->stack.emplace_back( &(class_type->class_methods[key.to_string()].val) );
                                    break;
                                }
                                std::stringstream ss;
                                ss << "(Runtime Error): '" << key.to_string() << "' does not exist in class '" << class_type->name << "'";
                                return ss.str();
                                break;
                            }

                            std::stringstream ss;
                            ss << "(Runtime Error): Cannot index a class with a value of type " << key.get_type();
                            return ss.str();
                            break;
                        }
                        case ValueType::OBJECT:
                        {
                            Object* obj = object->to_object();
                            if(key.get_type() == ValueType::STRING)
                            {
                                std::string name = key.to_string();
                                auto it = obj->props.find(name);
                                if(it != obj->props.end())
                                {
                                    if(obj->props[it->first].is_private && current_class_context != it->second.class_context)
                                    {
                                        std::stringstream ss;
                                        ss << "(Runtime Error): Cannot access the private property '" << name << "' outside of the class it is defined in";
                                        return ss.str();
                                    }

                                    data->stack.emplace_back(&(obj->props[it->first].val));
                                    break;
                                }

                                ClassType* current_type = obj->type;
                                bool value_is_found = false;
                                while(current_type != nullptr)
                                {
                                    it = current_type->getters.find(name);
                                    if(it != current_type->getters.end())
                                    {
                                        if(current_type->getters[it->first].is_private && current_class_context != it->second.class_context)
                                        {
                                            std::stringstream ss;
                                            ss << "(Runtime Error): Cannot access the private getter '" << name << "' outside of the class it is defined in";
                                            return ss.str();
                                        }
                                        data->stack.emplace_back(current_type->getters[it->first].val);
                                        data->stack.emplace_back(obj);
                                        std::optional<std::string> error = call_function(bytecode, data, 1);
                                        if(error)
                                        {
                                            return error;
                                        }
                                        value_is_found = true;
                                        break;
                                    }

                                    it = current_type->methods.find(name);
                                    if(it != current_type->methods.end())
                                    {
                                        if(current_type->methods[it->first].is_private && current_class_context != it->second.class_context)
                                        {
                                            std::stringstream ss;
                                            ss << "(Runtime Error): Cannot access the private method '" << name << "' outside of the class it is defined in";
                                            return ss.str();
                                        }
                                        data->stack.emplace_back(&(current_type->methods[it->first].val));
                                        value_is_found = true;
                                        break;
                                    }
                                    current_type = current_type->super;
                                }
                                if(value_is_found)
                                {
                                    break;
                                }

                                std::stringstream ss;
                                ss << "(Runtime Error): '" << name << "' does not exist in object of type '" << obj->type->name << "'";
                                return ss.str();
                            }

                            std::stringstream ss;
                            ss << "(Runtime Error): Cannot index an object with a value of type " << key.get_type();
                            return ss.str();
                            break;
                        }
                        case ValueType::TABLE:
                        {
                            if(key.get_type() == ValueType::STRING && key.to_string() == "has")
                            {
                                data->stack.emplace_back(table_has);
                                break;
                            }
                            data->stack.emplace_back( &(object->to_table()->keys[key]) );
                            break;
                        }
                        case ValueType::STRING:
                        {
                            if(key.get_type() == ValueType::STRING)
                            {
                                if(key.to_string() == "length")
                                {
                                    data->stack.emplace_back(string_length);
                                }
                                else
                                {
                                    std::stringstream ss;
                                    ss << "(Runtime Error): '" << key.to_string() << "' does not exist in string";
                                    return ss.str();
                                }
                            }
                            else if(key.get_type() == ValueType::NUMBER)
                            {
                                int num = key.to_number();
                                if(num < 0)
                                {
                                    std::stringstream ss;
                                    ss << "(Runtime Error): Index is less than 0";
                                    return ss.str();
                                }
                                else if(num > object->to_string().size())
                                {
                                    std::stringstream ss;
                                    ss << "(Runtime Error): Index is greater than the string length";
                                    return ss.str();
                                }
                                else
                                {
                                    data->stack.emplace_back(std::string(1, object->to_string()[num]));
                                }
                            }
                            std::stringstream ss;
                            ss << "(Runtime Error): Expected a number";
                            return ss.str();
                            break;
                        }
                        default:
                        {
                            std::stringstream ss;
                            ss << "(Runtime Error): Cannot index value of type " << object->get_type(); 
                            return ss.str();
                            break;
                        }
                    }
                    break;
                }
                case OP_JMP:
                {
                    ipointer = instruction.operand.value();
                    continue;
                }
                case OP_NJMP:
                {
                    if(data->index_stack(-1).get_type() != ValueType::BOOL)
                    {
                        std::stringstream ss;
                        ss << bytecode.file_name << "(Runtime Error): Expression does not evaluate to true";
                        return ss.str();
                    }

                    if(!data->index_stack(-1).to_bool())
                    {
                        ipointer = instruction.operand.value();
                        data->stack.pop_back();
                        continue;
                    }
                    data->stack.pop_back();
                    break;
                }
                case OP_PUSH_SCOPE:
                {
                    Data scope;
                    scope.get_impl()->parent = data;
                    ipointer++;
                    std::optional<std::string> error = run_bytecode(bytecode, scope.get_impl());
                    if(error)
                    {
                        return error;
                    }
                    if(returning)
                    {
                        data->stack.emplace_back(scope.get_impl()->index_stack(-1));
                    }
                    break;
                }
                case OP_POP_SCOPE:
                {
                    GarbageCollector::collect_garbage(data);
                    return {};
                    break;
                }
                case OP_TYPE_CHECK:
                {
                    Value val = data->index_stack(-1);
                    data->stack.pop_back();
                    std::string type = bytecode.constants[instruction.operand.value()].to_string();
                    data->stack.emplace_back(check_value_type(val, type));
                    break;
                }
                case OP_POP:
                {
                    data->stack.pop_back();
                    break;
                }
                case OP_REMOVE_TOP:
                {
                    data->stack.erase(data->stack.begin() + (data->stack.size() - instruction.operand.value() ) );
                    break;
                }
                case OP_ADD:
                {
                    Value right = data->index_stack(-1);
                    UNPACK_REF(right);
                    data->stack.pop_back();
                    Value left = data->index_stack(-1);
                    UNPACK_REF(left);
                    data->stack.pop_back();
                    if(left.get_type() == ValueType::NUMBER && 
                            right.get_type() == ValueType::NUMBER)
                    {
                        data->stack.emplace_back(left.to_number() + right.to_number());
                    }
                    else if(left.get_type() == ValueType::STRING && 
                            right.get_type() == ValueType::STRING)
                    {
                        data->stack.emplace_back(left.to_string() + right.to_string());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform addition with types " 
                                << left.get_type() << " and " << right.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_SUB:
                {
                    Value right = data->index_stack(-1);
                    UNPACK_REF(right);
                    data->stack.pop_back();
                    Value left = data->index_stack(-1);
                    UNPACK_REF(left);
                    data->stack.pop_back();
                    if(left.get_type() == ValueType::NUMBER && 
                            right.get_type() == ValueType::NUMBER)
                    {
                        data->stack.emplace_back(left.to_number() - right.to_number());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform substraction with types " 
                                << left.get_type() << " and " << right.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_MUL:
                {
                    Value right = data->index_stack(-1);
                    UNPACK_REF(right);
                    data->stack.pop_back();
                    Value left = data->index_stack(-1);
                    UNPACK_REF(left);
                    data->stack.pop_back();
                    if(left.get_type() == ValueType::NUMBER && 
                            right.get_type() == ValueType::NUMBER)
                    {
                        data->stack.emplace_back(left.to_number() * right.to_number());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform multiplication with types " 
                                << left.get_type() << " and " << right.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_DIV:
                {
                    Value right = data->index_stack(-1);
                    UNPACK_REF(right);
                    data->stack.pop_back();
                    Value left = data->index_stack(-1);
                    UNPACK_REF(left);
                    data->stack.pop_back();
                    if(left.get_type() == ValueType::NUMBER && 
                            right.get_type() == ValueType::NUMBER)
                    {
                        if(right.to_number() == 0)
                        {
                            std::stringstream ss;
                            ss << bytecode.file_name << "(Runtime Error): Cannot divide a number by zero";
                            return ss.str();
                        }
                        data->stack.emplace_back(left.to_number() / right.to_number());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform modulo with types " 
                                << left.get_type() << " and " << right.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_MOD:
                {
                    Value right = data->index_stack(-1);
                    UNPACK_REF(right);
                    data->stack.pop_back();
                    Value left = data->index_stack(-1);
                    UNPACK_REF(left);
                    data->stack.pop_back();
                    if(left.get_type() == ValueType::NUMBER && 
                            right.get_type() == ValueType::NUMBER)
                    {
                        data->stack.emplace_back(std::fmod(left.to_number(), right.to_number()) );
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform modulo with types " 
                                << left.get_type() << " and " << right.get_type();
                        return ss.str();
                    }
                    
                    break;
                }
                case OP_POW:
                {
                    Value right = data->index_stack(-1).to_number();
                    UNPACK_REF(right);
                    data->stack.pop_back();
                    Value left = data->index_stack(-1).to_number();
                    UNPACK_REF(left);
                    data->stack.pop_back();
                    if(left.get_type() == ValueType::NUMBER && 
                            right.get_type() == ValueType::NUMBER)
                    {
                        data->stack.emplace_back(std::pow(left.to_number(), right.to_number()));
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform exponentiation with types " 
                                << left.get_type() << " and " << right.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_NEG:
                {
                    Value operand = data->index_stack(-1);
                    UNPACK_REF(operand);
                    data->stack.pop_back();
                    if(operand.get_type() == ValueType::NUMBER)
                    {
                        data->stack.emplace_back(-operand.to_number());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform negation with value of type " 
                                << operand.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_EQ:
                {
                    Value right = data->index_stack(-1);
                    UNPACK_REF(right);
                    data->stack.pop_back();
                    Value left = data->index_stack(-1);
                    UNPACK_REF(left);
                    data->stack.pop_back();
                    if(left.get_type() == ValueType::NUMBER && 
                            right.get_type() == ValueType::NUMBER)
                    {
                        data->stack.emplace_back(left.to_number() == right.to_number());
                    }
                    else if(left.get_type() == ValueType::_NULL || 
                            right.get_type() == ValueType::_NULL)
                    {
                        data->stack.emplace_back(left.get_type() == right.get_type());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform equality comparison with types " 
                                << left.get_type() << " and " << right.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_NE:
                {
                    Value right = data->index_stack(-1);
                    UNPACK_REF(right);
                    data->stack.pop_back();
                    Value left = data->index_stack(-1);
                    UNPACK_REF(left);
                    data->stack.pop_back();
                    if(left.get_type() == ValueType::NUMBER && 
                            right.get_type() == ValueType::NUMBER)
                    {
                        data->stack.emplace_back(left.to_number() == right.to_number());
                    }
                    else if(left.get_type() == ValueType::_NULL || 
                            right.get_type() == ValueType::_NULL)
                    {
                        data->stack.emplace_back(left.get_type() != right.get_type());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform inequality comparison with types " 
                                << left.get_type() << " and " << right.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_LT:
                {
                    Value right = data->index_stack(-1);
                    UNPACK_REF(right);
                    data->stack.pop_back();
                    Value left = data->index_stack(-1);
                    UNPACK_REF(left);
                    data->stack.pop_back();
                    if(left.get_type() == ValueType::NUMBER && 
                            right.get_type() == ValueType::NUMBER)
                    {
                        data->stack.emplace_back(left.to_number() < right.to_number());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform less than comparison with types " 
                                << left.get_type() << " and " << right.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_LE:
                {
                    Value right = data->index_stack(-1);
                    UNPACK_REF(right);
                    data->stack.pop_back();
                    Value left = data->index_stack(-1);
                    UNPACK_REF(left);
                    data->stack.pop_back();
                    if(left.get_type() == ValueType::NUMBER && 
                            right.get_type() == ValueType::NUMBER)
                    {
                        data->stack.emplace_back(left.to_number() <= right.to_number());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform less than or equal comparison with types " 
                                << left.get_type() << " and " << right.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_GT:
                {
                    Value right = data->index_stack(-1);
                    UNPACK_REF(right);
                    data->stack.pop_back();
                    Value left = data->index_stack(-1);
                    UNPACK_REF(left);
                    data->stack.pop_back();
                    if(left.get_type() == ValueType::NUMBER && 
                            right.get_type() == ValueType::NUMBER)
                    {
                        data->stack.emplace_back(left.to_number() > right.to_number());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform greater than comparison with types " 
                                << left.get_type() << " and " << right.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_GE:
                {
                    Value right = data->index_stack(-1);
                    UNPACK_REF(right);
                    data->stack.pop_back();
                    Value left = data->index_stack(-1);
                    UNPACK_REF(left);
                    data->stack.pop_back();
                    if(left.get_type() == ValueType::NUMBER && 
                            right.get_type() == ValueType::NUMBER)
                    {
                        data->stack.emplace_back(left.to_number() >= right.to_number());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform greater than or equal comparison with types " 
                                << left.get_type() << " and " << right.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_OR:
                {
                    Value right = data->index_stack(-1);
                    UNPACK_REF(right);
                    data->stack.pop_back();
                    Value left = data->index_stack(-1);
                    UNPACK_REF(left);
                    data->stack.pop_back();
                    if(left.get_type() == ValueType::BOOL && 
                            right.get_type() == ValueType::BOOL)
                    {
                        data->stack.emplace_back(left.to_bool() || right.to_bool());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform logical operation (or) with types " 
                                << left.get_type() << " and " << right.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_AND:
                {
                    Value right = data->index_stack(-1);
                    UNPACK_REF(right);
                    data->stack.pop_back();
                    Value left = data->index_stack(-1);
                    UNPACK_REF(left);
                    data->stack.pop_back();
                    if(left.get_type() == ValueType::BOOL && 
                            right.get_type() == ValueType::BOOL)
                    {
                        data->stack.emplace_back(left.to_bool() && right.to_bool());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform logical operation (and) with types " 
                                << left.get_type() << " and " << right.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_NOT:
                {
                    Value operand = data->index_stack(-1);
                    UNPACK_REF(operand);
                    data->stack.pop_back();
                    if(operand.get_type() == ValueType::BOOL)
                    {
                        data->stack.emplace_back(!operand.to_bool());
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "(Runtime Error): Cannot perform logical operation (not) with value of type " 
                                << operand.get_type();
                        return ss.str();
                    }
                    break;
                }
                case OP_CALL:
                {
                    uint64_t arg_count = instruction.operand.value();
                    std::optional<std::string> error = call_function(bytecode, data, arg_count);
                    if(error)
                    {
                        return error;
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
                    returning = true;
                    return {};
                    break;
                }
            }
            ipointer++;
        }
        return {};
    }

    std::optional<std::string> VirtualMachine::call_function(const Bytecode& bytecode, Data::Impl* data, uint64_t arg_count)
    {
        Data scope;
        scope.get_impl()->parent = data;
        for(int i = -arg_count; i < 0; i++)
        {
            Value val = data->index_stack(i);
            UNPACK_REF(val);
            scope.get_impl()->stack.emplace_back(val);
            data->stack.erase(data->stack.begin() + (data->stack.size() + i));
        }

        Value func = data->index_stack(-1);
        UNPACK_REF(func);
        data->stack.pop_back();

        if(func.get_type() == ValueType::SOURDO_FUNCTION)
        {
            if(func.to_sourdo_function()->parameter_count != arg_count)
            {
                std::stringstream ss;
                ss << "Function being called expected " << func.to_sourdo_function()->parameter_count; 
                
                if(func.to_sourdo_function()->parameter_count == 1)
                {
                    ss << " argument but ";
                }
                else
                {
                    ss << " arguments but ";
                }

                ss << arg_count;

                if(arg_count == 1)
                {
                    ss << " was given";
                }
                else
                {
                    ss << " were given";
                }
                return ss.str();
            }
            std::optional<std::string> saved_class_context = current_class_context;
            current_class_context = func.to_sourdo_function()->class_context;
            bool saved_state = is_function;
            is_function = true;
            uint64_t saved_ipointer = ipointer;
            ipointer = 0;
            std::optional<std::string> error = run_bytecode(func.to_sourdo_function()->bytecode, scope.get_impl());
            if(error)
            {
                return error;
            }
            returning = false;
            ipointer = saved_ipointer;
            is_function = saved_state;
            current_class_context = saved_class_context;
            data->stack.emplace_back(scope.get_impl()->index_stack(-1));

            GarbageCollector::collect_garbage(scope.get_impl());
            return {};
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
                return {};
            }
            catch(SourDoError err)
            {
                return bytecode.file_name + "(Runtime Error): " + err.what();
            }
        }
        std::stringstream ss;
        ss << bytecode.file_name << "(Runtime Error): Value being called is not a function";
        return ss.str();
    }
} // namespace sourdo

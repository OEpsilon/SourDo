#include "SourDo/SourDo.hpp"
#include "SourDo/ConsoleColors.hpp"

#include "SourDoData.hpp"
#include "Interpreter.hpp"
#include "GarbageCollector.hpp"
#include "GlobalData.hpp"
#include "Bytecode/BytecodeGen.hpp"
#include "Bytecode/VM.hpp"
#include "Datatypes/Function.hpp"

#include <fstream>
#include <iostream>

namespace sourdo
{
    Data::Data()
    {
        impl = new Impl();
    }

    Data::~Data()
    {
        impl->symbol_table.clear();
        if(impl->parent == nullptr)
        {
            GarbageCollector::collect_garbage(impl);
        }
        delete impl;
    }

    Result Data::do_string(const std::string& string)
    {
        auto[tokens, tok_error] = tokenize_string(string, string);
        if(tok_error)
        {
            std::stringstream ss;
            ss << COLOR_RED << tok_error.value() << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }

        Parser parser;
        auto[ast, parse_error] = parser.parse_tokens(tokens);
        if(parse_error)
        {
            std::stringstream ss;
            ss << COLOR_RED << parse_error.value() << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }

        BytecodeGenerator byte_gen;
        auto bytecode = byte_gen.generate_bytecode(ast);
        if(bytecode.error)
        {
            std::stringstream ss;
            ss << COLOR_RED << bytecode.error.value() << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }
        std::cout << bytecode.bytecode;
        VirtualMachine vm;
        std::optional<std::string> error = vm.run_bytecode(bytecode.bytecode, impl);
        if(error)
        {
            std::stringstream ss;
            ss << COLOR_RED << error.value() << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }

        return Result::SUCCESS;
    }
    
    Result Data::do_file(const std::string& file_path)
    {
        std::ifstream file;
        file.open(file_path);
        if(!file.is_open())
        {
            std::stringstream ss;
            ss << "Could not open file '" << file_path << "'";
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }

        std::stringstream file_text;
        file_text << file.rdbuf();

        file.close();

        auto[tokens, tok_error] = tokenize_string(file_text.str(), file_path);
        if(tok_error)
        {
            std::stringstream ss;
            ss << COLOR_RED << tok_error.value() << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }

        Parser parser;
        auto[ast, parse_error] = parser.parse_tokens(tokens);
        if(parse_error)
        {
            std::stringstream ss;
            ss << COLOR_RED << parse_error.value() << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }

        BytecodeGenerator byte_gen;
        auto bytecode = byte_gen.generate_bytecode(ast);
        if(bytecode.error)
        {
            std::stringstream ss;
            ss << COLOR_RED << bytecode.error.value() << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }
        VirtualMachine vm;
        std::optional<std::string> error = vm.run_bytecode(bytecode.bytecode, impl);
        if(error)
        {
            std::stringstream ss;
            ss << COLOR_RED << error.value() << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }

        return Result::SUCCESS;
    }
    
    void* Data::create_cpp_object(size_t size)
    {
        //CppObject* cpp_object = new CppObject(size);
        //impl->stack.push_back(cpp_object);
        //return cpp_object->block;
        return nullptr;
    }
    
    void* Data::check_cpp_object(int index, const std::string& name)
    {
        Value& value = impl->index_stack(index);
        if(value.get_type() == ValueType::CPP_OBJECT)
        {
            if(check_value_type(value, name))
            {
                return value.to_cpp_object()->block;
            }
        }
        std::stringstream ss;
        ss << "Argument #" << index << ": Expected a CppObject that inherits type '" << name << "'";
        throw SourDoError(ss.str());
    }
    
    void* Data::test_cpp_object(int index, const std::string& name)
    {
        Value& value = impl->index_stack(index);
        if(value.get_type() == ValueType::CPP_OBJECT)
        {
            if(check_value_type(value, name))
            {
                return value.to_cpp_object()->block;
            }
        }
        return nullptr;
    }

    void Data::set_cpp_object_type(int index, const std::string& name)
    {
        pop();
    }

    GCRef Data::create_ref(int index)
    {
        Value& value = impl->index_stack(index);

        if(value.get_type() == ValueType::OBJECT || value.get_type() == ValueType::SOURDO_FUNCTION)
        {
            GCRef ref;
            auto it = std::find(GlobalData::references.begin(), GlobalData::references.end(), Null());
            if(it == GlobalData::references.end())
            {
                ref = GlobalData::references.size();
                GlobalData::references.emplace_back(value);
                return ref;
            }
            *it = value;
            return it - GlobalData::references.begin();
        }
        // Invalid reference instead of error
        return -1;
    }

    void Data::remove_ref(GCRef ref)
    {
        assert(ref >= 0);
        assert(ref < GlobalData::references.size());
    }

    void Data::push_ref(GCRef ref)
    {
        assert(ref >= 0);
        assert(ref < GlobalData::references.size());

        impl->stack.emplace_back(GlobalData::references[ref]);
    }

    bool Data::is_ref_valid(GCRef ref)
    {
        return ref >= 0 && ref < GlobalData::references.size();
    }

    void Data::create_table()
    {
        impl->stack.emplace_back(new Table());
        GarbageCollector::collect_garbage(impl);
    }

    Result Data::table_set(int object_index, bool protected_mode_enabled)
    {
        sourdo::Value& obj = impl->index_stack(object_index);
        if(obj.get_type() != ValueType::TABLE)
        {
            std::stringstream ss;
            ss << COLOR_RED << "'" << "Value at the given index is not an object" << COLOR_DEFAULT << std::flush;
            if(protected_mode_enabled)
            {
                push_string(ss.str());
                return Result::RUNTIME_ERROR;
            }
            throw SourDoError(ss.str());
        }
        Value new_value = impl->index_stack(-1);
        impl->stack.pop_back();
        Value key = impl->index_stack(-1);
        impl->stack.pop_back();

        if(key.get_type() == ValueType::STRING && key.to_string() == "__prototype"
                && (new_value.get_type() != ValueType::OBJECT || new_value.get_type() != ValueType::_NULL))
        {
            std::stringstream ss;
            ss << COLOR_RED << "'__prototype' must be set to an object or null" << COLOR_DEFAULT << std::flush;
            if(protected_mode_enabled)
            {
                push_string(ss.str());
                return Result::RUNTIME_ERROR;
            }
            throw SourDoError(ss.str());
        }
        
        obj.to_table()->keys[key] = new_value;
        return Result::SUCCESS;
    }

    Result Data::table_get(int object_index, bool protected_mode_enabled)
    {
        sourdo::Value& obj = impl->index_stack(object_index);
        if(obj.get_type() != ValueType::OBJECT)
        {
            std::stringstream ss;
            ss << COLOR_RED << "'" << "Value at the given index is not an object" << COLOR_DEFAULT << std::flush;
            if(protected_mode_enabled)
            {
                push_string(ss.str());
                return Result::RUNTIME_ERROR;
            }
            throw SourDoError(ss.str());
        }
        Value key = impl->index_stack(-1);
        impl->stack.pop_back();

        if(obj.to_table()->keys.find(key) == obj.to_table()->keys.end())
        {
            std::stringstream ss;
            ss << COLOR_RED << "Key does not exist in object" << COLOR_DEFAULT << std::flush;
            if(protected_mode_enabled)
            {
                push_string(ss.str());
                return Result::RUNTIME_ERROR;
            }
            throw SourDoError(ss.str());
        }
        
        impl->stack.emplace_back(obj.to_table()->keys[key]);
        return Result::SUCCESS;
    }

    ValueType Data::get_value_type(int index)
    {
        return impl->index_stack(index).get_type();
    }

    Number Data::value_to_number(int index)
    {
        Value& value = impl->index_stack(index);
        if(value.get_type() != ValueType::NUMBER)
        {
            return 0;
        }
        return value.to_number();
    }

    bool Data::value_to_bool(int index)
    {
        Value& value = impl->index_stack(index);
        if(value.get_type() != ValueType::BOOL)
        {
            return false;
        }
        return value.to_bool();
    }
    
    std::string Data::value_to_string(int index)
    {
        Value& value = impl->index_stack(index);
        if(value.get_type() != ValueType::STRING)
        {
            return "";
        }
        return value.to_string();
    }

    void Data::push_number(Number value)
    {
        impl->stack.emplace_back(value);
        GarbageCollector::collect_garbage(impl);
    }

    void Data::push_bool(bool value)
    {
        impl->stack.emplace_back(bool(value));
        GarbageCollector::collect_garbage(impl);
    }

    void Data::push_string(const std::string& value)
    {
        impl->stack.emplace_back(value);
        GarbageCollector::collect_garbage(impl);
    }

    void Data::push_cppfunction(const CppFunction& value)
    {
        impl->stack.emplace_back(value);
        GarbageCollector::collect_garbage(impl);
    }

    void Data::push_null()
    {
        impl->stack.emplace_back(Null());
        GarbageCollector::collect_garbage(impl);
    }

    Result Data::call_function(uint32_t arg_count, bool protected_mode_enabled)
    {
        Value func = impl->index_stack((-arg_count) - 1);
        if(func.get_type() != ValueType::SOURDO_FUNCTION && func.get_type() != ValueType::CPP_FUNCTION)
        {
            std::stringstream ss;
            ss << COLOR_RED << "Value is not a function" << COLOR_DEFAULT << std::flush;
            if(protected_mode_enabled)
            {
                push_string(ss.str());
                return Result::RUNTIME_ERROR;
            }
            throw SourDoError(ss.str());
        }
        remove(-arg_count - 1);
        std::vector<Value> args;
        args.reserve(arg_count);

        for(int i = -arg_count; i < 0; i++)
        {
            args.emplace_back(impl->index_stack(i));
            remove(i);
        }
        
        if(func.get_type() == ValueType::SOURDO_FUNCTION)
        {
            
            SourDoFunction* func_value = func.to_sourdo_function();
            if(args.size() != func_value->parameter_count)
            {
                std::stringstream ss;
                ss << COLOR_RED << "Function being called expected " << func_value->parameter_count; 
                
                if(func_value->parameter_count == 1)
                {
                    ss << " argument but ";
                }
                else
                {
                    ss << " arguments but ";
                }

                ss << args.size();

                if(args.size() == 1)
                {
                    ss << " was given";
                }
                else
                {
                    ss << " were given";
                }
                ss << COLOR_DEFAULT << std::flush;
                if(protected_mode_enabled)
                {
                    push_string(ss.str());
                    return Result::RUNTIME_ERROR;
                }
                throw SourDoError(ss.str());
            }

            Data func_scope;
            func_scope.get_impl()->parent = impl;
            for(int i = 0; i < func_value->parameter_count; i++)
            {
                func_scope.get_impl()->stack.emplace_back(args[i]);
            }
            VirtualMachine vm;
            vm.run_bytecode(func_value->bytecode, func_scope.get_impl());
        }
        else
        {
            Data func_scope;
            func_scope.get_impl()->parent = impl;
            for(int i = 0; i < args.size(); i++)
            {
                func_scope.get_impl()->stack.emplace_back(args[i]);
            }
            CppFunction func_value = func.to_cpp_function();
            try
            {
                if(func_value(func_scope))
                {
                    impl->stack.emplace_back(func_scope.impl->index_stack(-1));
                }
                else
                {
                    push_null();
                }
            }
            catch(const SourDoError& error)
            {
                if(protected_mode_enabled)
                {
                    push_string(error.what());
                    return Result::RUNTIME_ERROR;
                }
                throw;
            }
        }
        return Result::SUCCESS;
    }

    uint32_t Data::get_sourdo_func_param_count(int index)
    {
        Value& value = impl->index_stack(index);
        if(value.get_type() == ValueType::SOURDO_FUNCTION)
        {
            return value.to_sourdo_function()->parameter_count;
        }
        return 0;
    }

    void Data::create_value(const std::string& name)
    {
        impl->symbol_table[name].val = Null();
    }

    void Data::create_constant(const std::string& name)
    {
        impl->symbol_table[name] = {true, Null()};
    }

    Result Data::get_value(const std::string& name, bool protected_mode_enabled)
    {
        std::optional<Value> value = impl->get_symbol(name);
        if(!value)
        {
            std::stringstream ss;
            ss << COLOR_RED << "'" << name << "' is undefined" << COLOR_DEFAULT << std::flush;
            if(protected_mode_enabled)
            {
                push_string(ss.str());
                return Result::RUNTIME_ERROR;
            }
            throw SourDoError(ss.str());
        }

        impl->stack.emplace_back(*value);
        return Result::SUCCESS;
    }

    Result Data::set_value(const std::string& name, bool protected_mode_enabled)
    {
        SetSymbolResult result = impl->set_symbol(name, impl->index_stack(-1));
        pop();
        GarbageCollector::collect_garbage(impl);
        switch(result)
        {
            case SetSymbolResult::SYM_NOT_FOUND:
            {
                std::stringstream ss;
                ss << COLOR_RED << "'" << name << "' is undefined" << COLOR_DEFAULT << std::flush;
                if(protected_mode_enabled)
                {
                    push_string(ss.str());
                    return Result::RUNTIME_ERROR;
                }
                throw SourDoError(ss.str());
                break;
            }
            case SetSymbolResult::SYM_READONLY:
            {
                std::stringstream ss;
                ss << COLOR_RED << "'" << name << "' is a constant" << COLOR_DEFAULT << std::flush;
                if(protected_mode_enabled)
                {
                    push_string(ss.str());
                    return Result::RUNTIME_ERROR;
                }
                throw SourDoError(ss.str());
                break;
            }
            default:
                break;
        }
        return Result::SUCCESS;
    }

    uint32_t Data::get_size()
    {
        return impl->stack.size();
    }

    void Data::remove(int index)
    {
        assert(index != 0);
        assert(std::abs(index) <= impl->stack.size());

        if(index > 0)
        {
            impl->stack.erase(impl->stack.begin() + (index - 1));
        }
        else
        {
            impl->stack.erase(impl->stack.begin() + (impl->stack.size() + index ) );
        }
    }

    void Data::pop()
    {
        impl->stack.pop_back();
    }

    void Data::error(const std::string& message)
    {
        throw SourDoError(message);
    }
} // namespace sourdo

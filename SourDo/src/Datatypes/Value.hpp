#pragma once

#include "SourDo/SourDo.hpp"

#include <vector>
#include <memory>
#include <variant>
#include <string>
#include <unordered_map>

namespace sourdo 
{
    struct StatementListNode;
    
    struct Null
    {
    };

    struct SourDoFunction
    {
        SourDoFunction(std::vector<std::string> parameters, std::shared_ptr<StatementListNode> statements)
            : parameters(parameters), statements(statements)
        {
        }
        std::vector<std::string> parameters;
        std::shared_ptr<StatementListNode> statements;
    };

    class Value
    {
    public:
        Value();
        Value(Null new_value);
        Value(double new_value);
        Value(bool new_value);
        Value(const std::string& new_value);
        Value(std::shared_ptr<SourDoFunction> new_value);
        Value(const CppFunction& new_value);
        Value(std::shared_ptr<Object> new_value);
        
        Value(const Value& new_value);
        Value(Value&& new_value);

        Value& operator=(const Value& new_value);
        Value& operator=(Value&& new_value);

        Value& operator=(Null new_value);
        Value& operator=(double new_value);
        Value& operator=(bool new_value);
        Value& operator=(const std::string& new_value);
        Value& operator=(std::shared_ptr<SourDoFunction> new_value);
        Value& operator=(const CppFunction& new_value);
        Value& operator=(std::shared_ptr<Object> new_value);

        bool operator==(const Value& other) const;

        ValueType get_type();

        double to_number() const
        { 
            return std::get<double>(value);
        }
        
        bool to_bool() const
        {
            return std::get<bool>(value); 
        }

        std::string to_string() const
        { 
            return std::get<std::string>(value); 
        }

        std::shared_ptr<SourDoFunction> to_sourdo_function() const
        {
            return std::get<std::shared_ptr<SourDoFunction>>(value);
        }

        CppFunction to_cpp_function() const
        {
            return std::get<CppFunction>(value);
        }

        std::shared_ptr<Object> to_object() const
        {
            return std::get<std::shared_ptr<Object>>(value);
        }
    private:
        friend struct std::hash<Value>;
        ValueType type;

        std::variant<
                Null, 
                double, 
                bool, 
                std::string, 
                std::shared_ptr<SourDoFunction>, 
                CppFunction, 
                std::shared_ptr<Object>
            > value;
    };
} // namespace SourDo

namespace std
{
    template <>
    struct hash<sourdo::Null>
    {
        std::size_t operator()(const sourdo::Value& k) const
        {
            return std::hash<nullptr_t>()(nullptr);
        }
    };

    template <>
    struct hash<sourdo::Value>
    {
        std::size_t operator()(const sourdo::Value& k) const
        {
            return std::hash<std::variant<
                    sourdo::Null, 
                    double, 
                    bool, 
                    std::string, 
                    std::shared_ptr<sourdo::SourDoFunction>, 
                    sourdo::CppFunction, 
                    std::shared_ptr<sourdo::Object>
                >>()(k.value);
        }
    };
} // namespace std

namespace sourdo
{
    struct Object
    {
        Object(const std::unordered_map<Value, Value>& keys)
            : keys(keys)
        {
        }

        std::unordered_map<Value, Value> keys;
    };

} // namespace SourDo
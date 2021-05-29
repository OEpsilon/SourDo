#pragma once

#include "SourDo/SourDo.hpp"
#include "GCObject.hpp"

#include <vector>
#include <variant>
#include <string>
#include <unordered_map>

namespace sourdo 
{
    struct Null
    {
    };
    // Can't put this in the Null struct declaration as I get errors.

    constexpr bool operator==(const Null& first, const Null& second)
    {
        return true;
    }

    constexpr bool operator!=(const Null& first, const Null& second)
    {
        return false;
    }

    struct SourDoFunction;
    struct Table;
    struct Object;
    struct CppObject;

    class Value
    {
    public:
        Value();
        Value(Null new_value);
        Value(double new_value);
        Value(bool new_value);
        Value(const std::string& new_value);
        Value(const char* new_value);
        Value(SourDoFunction* new_value);
        Value(const CppFunction& new_value);
        Value(Object* new_value);
        Value(Table* new_value);
        Value(CppObject* new_value);
        
        Value(const Value& new_value);
        Value(Value&& new_value);

        Value& operator=(const Value& new_value);
        Value& operator=(Value&& new_value);

        Value& operator=(Null new_value);
        Value& operator=(double new_value);
        Value& operator=(bool new_value);
        Value& operator=(const std::string& new_value);
        Value& operator=(const char* new_value);
        Value& operator=(SourDoFunction* new_value);
        Value& operator=(const CppFunction& new_value);
        Value& operator=(Object* new_value);
        Value& operator=(Table* new_value);
        Value& operator=(CppObject* new_value);

        bool operator==(const Value& other) const;
        bool operator!=(const Value& other) const;

        ValueType get_type() const;

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

        SourDoFunction* to_sourdo_function() const
        {
            return std::get<SourDoFunction*>(value);
        }

        CppFunction to_cpp_function() const
        {
            return std::get<CppFunction>(value);
        }

        Table* to_table() const
        {
            return std::get<Table*>(value);
        }

        Object* to_object() const
        {
            return std::get<Object*>(value);
        }

        CppObject* to_cpp_object() const
        {
            return std::get<CppObject*>(value);
        }
    private:
        friend struct std::hash<Value>;
        friend std::ostream& operator<<(std::ostream& os, const Value& val);
        ValueType type;

        std::variant<
                Null, 
                double, 
                bool, 
                std::string, 
                SourDoFunction*, 
                CppFunction, 
                Object*,
                Table*,
                CppObject*
            > value;
    };

    std::ostream& operator<<(std::ostream& os, const Value& val);
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
                    sourdo::SourDoFunction*, 
                    sourdo::CppFunction, 
                    sourdo::Object*,
                    sourdo::Table*,
                    sourdo::CppObject*
                >>()(k.value);
        }
    };
} // namespace std

namespace sourdo
{
    struct Table : public GCObject
    {
        Table() = default;
        
        Table(const std::unordered_map<Value, Value>& keys)
            : keys(keys)
        {
        }
        
        virtual ~Table() = default;

        std::unordered_map<Value, Value> keys;
        bool readonly = false;

        void on_garbage_collected() final
        {
        }
    };

    struct ClassType : public GCObject
    {
        ClassType(const std::unordered_map<std::string, Value>& methods,
                const std::vector<std::string>& property_names)
            : methods(methods), property_names(property_names)
        {
        }

        virtual ~ClassType() = default;
        ClassType* super = nullptr;

        std::unordered_map<std::string, Value> methods;
        std::vector<std::string> property_names;
        std::string name;
    };

    struct Object : public GCObject
    {
        Object() = default;

        Object(const std::unordered_map<std::string, Value>& props, ClassType* type)
            : props(props), type(type)
        {
        }

        virtual ~Object() = default;
        ClassType* type = nullptr;

        std::unordered_map<std::string, Value> props;

        std::optional<Value> find_symbol(std::string name)
        {
            if(props.find(name) != props.end())
            {
                return props[name];
            }
            ClassType* current_type = type;
            while(current_type != nullptr)
            {
                if(current_type->methods.find(name) != current_type->methods.end())
                {
                    return current_type->methods[name];
                }
                current_type = current_type->super;
            }
            return {};
        }

        void on_garbage_collected() final
        {
        }
    };

    struct CppObject : public GCObject
    {
        CppObject(size_t size)
            : block(new uint8_t[size])
        {
        }

        void on_garbage_collected() final
        {
            delete[] block;
        }

        ClassType* type = nullptr;
        uint8_t* block;
    };

} // namespace SourDo
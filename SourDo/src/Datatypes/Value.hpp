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
    struct ClassType;
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
        Value(Value* new_value);
        Value(Object* new_value);
        Value(Table* new_value);
        Value(ClassType* new_value);
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
        Value& operator=(Value* new_value);
        Value& operator=(Object* new_value);
        Value& operator=(Table* new_value);
        Value& operator=(ClassType* new_value);
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

        Value* to_value_ref() const
        {
            return std::get<Value*>(value);
        }

        Table* to_table() const
        {
            return std::get<Table*>(value);
        }

        ClassType* to_class() const
        {
            return std::get<ClassType*>(value);
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
                Value*,
                Object*,
                Table*,
                ClassType*,
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
                    sourdo::Value*,
                    sourdo::Object*,
                    sourdo::Table*,
                    sourdo::ClassType*,
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

        void on_garbage_collected(Data::Impl* data) final
        {
        }
    };

    struct ClassType : public GCObject
    {
        struct Property
        {
            Property() = default;

            Property(const Value& val, const std::string& class_context, bool is_private, bool readonly)
                : val(val), class_context(class_context), is_private(is_private), readonly(readonly)
            {
            }

            Value val;
            std::string class_context;
            bool is_private = false;
            bool readonly = false;
        };

        ClassType(const std::string& name, ClassType* super)
            : name(name), super(super)
        {
        }

        virtual ~ClassType() = default;
        ClassType* super = nullptr;

        SourDoFunction* initializer = nullptr;
        std::unordered_map<std::string, Property> methods;
        std::unordered_map<std::string, Property> setters;
        std::unordered_map<std::string, Property> getters;

        std::unordered_map<std::string, Property> class_methods;
        
        std::string name;
        bool complete = false;

        void on_garbage_collected(Data::Impl* data) override {}
    };

    struct Object : public GCObject
    {
        Object() = default;

        Object(ClassType* type)
            : type(type)
        {
        }

        virtual ~Object() = default;
        ClassType* type = nullptr;

        std::unordered_map<std::string, ClassType::Property> props;

        Value* find_method(std::string name)
        {
            ClassType* current_type = type;
            while(current_type != nullptr)
            {
                if(current_type->methods.find(name) != current_type->methods.end())
                {
                    return &current_type->methods[name].val;
                }
                current_type = current_type->super;
            }
            return nullptr;
        }

        ClassType::Property* find_symbol(std::string name)
        {
            if(props.find(name) != props.end())
            {
                return &props[name];
            }
            ClassType* current_type = type;
            while(current_type != nullptr)
            {
                if(current_type->methods.find(name) != current_type->methods.end())
                {
                    return &current_type->methods[name];
                }
                current_type = current_type->super;
            }
            return nullptr;
        }

        void on_garbage_collected(Data::Impl* data) override;
    };

    struct CppObject : public Object
    {
        CppObject(ClassType* type, size_t size)
            : Object(type), block(new uint8_t[size])
        {
        }

        void on_garbage_collected(Data::Impl* data) final
        {
            Object::on_garbage_collected(data);
            delete[] block;
        }

        ClassType* type = nullptr;
        uint8_t* block;
    };

} // namespace SourDo
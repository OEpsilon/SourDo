#include "Value.hpp"
#include "Function.hpp"

#include <ostream>

namespace sourdo
{
    Value::Value()
    {
        type = ValueType::_NULL;
        value = Null();
    }

    Value::Value(Null new_value)
    {
        type = ValueType::_NULL;
        value = new_value;
    }

    Value::Value(double new_value)
    {
        type = ValueType::NUMBER;
        value = new_value;
    }

    Value::Value(bool new_value)
    {
        type = ValueType::BOOL;
        value = new_value;
    }
    
    Value::Value(const std::string& new_value)
    {
        type = ValueType::STRING;
        value = new_value;
    }

    Value::Value(const char* new_value)
    {
        type = ValueType::STRING;
        value = new_value;
    }

    Value::Value(SourDoFunction* new_value)
    {
        type = ValueType::SOURDO_FUNCTION;
        value = new_value;
    }

    Value::Value(const CppFunction& new_value)
    {
        type = ValueType::CPP_FUNCTION;
        value = new_value;
    }

    Value::Value(Value* new_value)
    {
        type = ValueType::VALUE_REF;
        value = new_value;
    }

    Value::Value(Object* new_value)
    {
        type = ValueType::OBJECT;
        value = new_value;
    }

    Value::Value(Table* new_value)
    {
        type = ValueType::TABLE;
        value = new_value;
    }

    Value::Value(ClassType* new_value)
    {
        type = ValueType::CLASS_TYPE;
        value = new_value;
    }
    
    Value::Value(CppObject* new_value)
    {
        type = ValueType::CPP_OBJECT;
        value = new_value;
    }

    Value::Value(const Value& new_value)
    {
        type = new_value.type;
        value = new_value.value;
    }

    Value::Value(Value&& new_value)
    {
        type = new_value.type;
        value = new_value.value;

        new_value.type = ValueType::_NULL;
        new_value.value = Null();
    }

    Value& Value::operator=(const Value& new_value)
    {
        type = new_value.type;
        value = new_value.value;
        return *this;
    }
    
    Value& Value::operator=(Value&& new_value)
    {
        type = new_value.type;
        value = new_value.value;

        new_value.type = ValueType::_NULL;
        new_value.value = Null();
        return *this;
    }

    Value& Value::operator=(Null new_value)
    {
        type = ValueType::_NULL;
        value = new_value;
        return *this;
    }
    
    Value& Value::operator=(double new_value)
    {
        type = ValueType::NUMBER;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(bool new_value)
    {
        type = ValueType::BOOL;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(const std::string& new_value)
    {
        type = ValueType::STRING;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(const char* new_value)
    {
        type = ValueType::STRING;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(SourDoFunction* new_value)
    {
        type = ValueType::SOURDO_FUNCTION;
        value = new_value;
        return *this;
    }
    
    Value& Value::operator=(const CppFunction& new_value)
    {
        type = ValueType::CPP_FUNCTION;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(Value* new_value)
    {
        type = ValueType::VALUE_REF;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(Object* new_value)
    {
        type = ValueType::OBJECT;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(Table* new_value)
    {
        type = ValueType::TABLE;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(ClassType* new_value)
    {
        type = ValueType::CLASS_TYPE;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(CppObject* new_value)
    {
        type = ValueType::CPP_OBJECT;
        value = new_value;
        return *this;
    }

    bool Value::operator==(const Value& other) const
    {
        return value == other.value;
    }

    bool Value::operator!=(const Value& other) const
    {
        return value != other.value;
    }

    ValueType Value::get_type() const
    {
        return type;
    }

    std::ostream& operator<<(std::ostream& os, const Value& val)
    {
        switch(val.type)
        {
            case ValueType::_NULL: 
                os << "null";
                break;
            case ValueType::NUMBER: 
            {
                os << val.to_number();
                break;
            }
            case ValueType::BOOL: 
                os << val.to_bool();
                break;
            case ValueType::STRING: 
                os << "\"" << val.to_string() << "\"";
                break;
            case ValueType::SOURDO_FUNCTION: 
                os << "[SourdoFunc: " << val.to_sourdo_function() << "]";
                break;
            case ValueType::CPP_FUNCTION: 
                os << "[CppFunc: " << val.to_cpp_function() << "]";
                break;
            case ValueType::VALUE_REF:
                os << *(val.to_value_ref());
                break;
            case ValueType::TABLE: 
            {
                os << "{";
                int i = 0;
                for(auto&[k, v] : val.to_table()->keys)
                {
                    os << k << "=" << v;
                    if(i < val.to_table()->keys.size() - 1)
                    {
                        os << ", ";
                    }
                    i++;
                }
                os << "}";
                break;
            }
            case ValueType::CLASS_TYPE: 
                os << "[ClassType: " << val.to_class()->name << "]";
                break;
            case ValueType::OBJECT: 
                os << "[Object: " << val.to_object() << "]";
                break;
            case ValueType::CPP_OBJECT:  
                os << "[CppObject: " << val.to_cpp_object() << "]";
                break;
            default:
                break;
        }
        return os;
    }
} // namespace sourdo

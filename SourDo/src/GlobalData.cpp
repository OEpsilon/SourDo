#include "GlobalData.hpp"

namespace sourdo
{
    std::vector<Value> GlobalData::references;

    bool check_value_type(const Value& value, const std::string& name)
    {
        auto find_class_name = [name](ClassType* type) -> bool
        {
            while(type != nullptr)
            {
                if(type->name == name)
                {
                    return true;
                }
                type = type->super;
            }
            return name == "Object";
        };

        switch(value.get_type())
        {
            case ValueType::_NULL:
            {
                return name == "null_type";
                break;
            }
            case ValueType::NUMBER:
            {
                return name == "number";
                break;
            }
            case ValueType::BOOL:
            {
                return name == "bool";
                break;
            }
            case ValueType::STRING:
            {
                return name == "string";
                break;
            }
            case ValueType::SOURDO_FUNCTION:
            {
                return name == "sourdo_function" || name == "function";
                break;
            }
            case ValueType::CPP_FUNCTION:
            {
                return name == "cpp_function" ||  name == "function";
                break;
            }
            case ValueType::OBJECT:
            {
                return find_class_name(value.to_object()->type);
                break;
            }
            case ValueType::CPP_OBJECT:
            {
                return find_class_name(value.to_cpp_object()->type);
                break;
            }
        }
        return false;
    }
} // namespace sourdo

#include "GlobalData.hpp"

namespace sourdo
{
    std::vector<Value> GlobalData::references;

    bool check_value_type(const Value& value, const std::string& name)
    {
        auto find_object_name = [name](Object* prototype) -> bool
        {
            while(prototype != nullptr)
            {
                if(prototype->keys.find("__name") != prototype->keys.end())
                {
                    return name == prototype->keys["__name"].to_string();
                }
                prototype = prototype->keys["__prototype"].to_object();
            }
            return name == "Object";
        };

        switch(value.get_type())
        {
            case ValueType::_NULL:
            {
                return name == "null";
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
                return name == "sourdo_function";
                break;
            }
            case ValueType::CPP_FUNCTION:
            {
                return name == "cpp_function";
                break;
            }
            case ValueType::OBJECT:
            {
                return find_object_name(value.to_object());
                break;
            }
            case ValueType::CPP_OBJECT:
            {
                return find_object_name(value.to_cpp_object()->prototype);
                break;
            }
        }
        return false;
    }
} // namespace sourdo

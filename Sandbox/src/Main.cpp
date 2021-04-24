#include <string>
#include <iostream>

#include <sstream>

#include <SourDo/SourDo.hpp>

int main()
{
    sourdo::Data test;

    test.push_cppfunction([](sourdo::Data& data) -> bool
    {
        uint32_t arg_count = data.get_size();
        std::stringstream ss;
        ss << std::boolalpha << std::fixed;
        for(int i = 1; i <= arg_count; i++)
        {
            switch(data.get_value_type(i))
            {
                case sourdo::ValueType::_NULL:
                    ss << "null";
                    break;
                case sourdo::ValueType::NUMBER:
                    ss << data.value_to_number(1);
                    break;
                case sourdo::ValueType::BOOL:
                    ss << data.value_to_bool(1);
                    break;
                case sourdo::ValueType::STRING:
                    ss << data.value_to_string(1);
                    break;
                case sourdo::ValueType::SOURDO_FUNCTION:
                    ss << "[SourdoFunction]";
                    break;
                case sourdo::ValueType::CPP_FUNCTION:
                    ss << "[CppFunction]";
                    break;
            }
            if(i < arg_count)
            {
                ss << " ";
            }
        }
        ss << std::endl;
        std::cout << ss.str();
        return false;
    });
    test.set_value("print");

    sourdo::Result result = test.do_file("Examples/Scripts/Main.sourdo");
    if(result != sourdo::Result::SUCCESS)
    {
        std::string error = test.value_to_string(-1);
        std::cout << error << std::endl;
    }
    
    return 0;
}

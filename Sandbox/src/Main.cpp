#include <string>
#include <iostream>

#include <sstream>

#include <chrono>

#include <SourDo/SourDo.hpp>
#include <SourDo/ConsoleColors.hpp>

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
                    ss << data.value_to_number(i);
                    break;
                case sourdo::ValueType::BOOL:
                    ss << data.value_to_bool(i);
                    break;
                case sourdo::ValueType::STRING:
                    ss << data.value_to_string(i);
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
    test.create_value("print");
    test.set_value("print");

    auto begin = std::chrono::high_resolution_clock::now();

    sourdo::Result result = test.do_file("Examples/Scripts/Main.sourdo");

    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - begin);
    if(result != sourdo::Result::SUCCESS)
    {
        std::string error = test.value_to_string(-1);
        std::cout << error << std::endl;
    }
    std::cout << "SourDo script took " << duration.count() << " milliseconds" << std::endl;
    
    return 0;
}

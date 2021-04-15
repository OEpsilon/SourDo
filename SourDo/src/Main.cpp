#include <string>
#include <iostream>

#include "SourDo/SourDo.hpp"

int main()
{
    sourdo_Data* module = sourdo_data_create();

    bool running = true;
    while(running)
    {
        std::string text;
        std::cout << std::boolalpha << "SourDo> ";
        std::getline(std::cin, text);

        SourDoBool result = sourdo_do_string(module, text.c_str());
        if(!result)
        {
            std::string error = sourdo_to_string(module, -1);
            std::cout << error << std::endl;
        }
        else
        {
            if(sourdo_is_number(module, -1))
            {
                double result = sourdo_to_number(module, -1);
                std::cout << result << std::endl;
            }
            else if(sourdo_is_bool(module, -1))
            {
                bool result = sourdo_to_bool(module, -1);
                std::cout << result << std::endl;
            }
            else if(sourdo_is_null(module, -1))
            {
                std::cout << "null" << std::endl;
            }
            
        }
    }

    sourdo_data_destroy(module);
    return 0;
}

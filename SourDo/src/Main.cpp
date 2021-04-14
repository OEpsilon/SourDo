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
        std::cout << "SourDo> ";
        std::getline(std::cin, text);

        SourDoBool result = sourdo_do_string(module, text.c_str());
        if(!result)
        {
            std::string error = sourdo_to_string(module, -1);
            std::cout << error << std::endl;
        }
        else
        {
            float result = sourdo_to_float(module, -1);
            std::cout << result << std::endl;
        }
    }

    sourdo_data_destroy(module);
    return 0;
}

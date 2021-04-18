#include <string>
#include <iostream>

#include "SourDo/SourDo.hpp"

int main()
{
    sourdo_Data* module = sourdo_data_create();

    sourdo_push_number(module, 40);
    sourdo_set_global(module, "apple_count");

    SourDoBool result = sourdo_do_file(module, "Examples/Scripts/Main.sourdo");
    if(result)
    {
        sourdo_get_global(module, "apple_count");
        if(sourdo_is_number(module, -1))
        {
            std::cout << "There are " << sourdo_to_number(module, -1) << " apples.\n";
        }
    }
    else
    {
        std::string error = sourdo_to_string(module, -1);
        std::cout << error << std::endl;
    }

    sourdo_data_destroy(module);
    return 0;
}

#include <string>
#include <iostream>

#include <SourDo/SourDo.hpp>

int main()
{
    sourdo::Data test;

    test.push_number(40);
    test.set_value("apple_count");

    sourdo::Result result = test.do_file("Examples/Scripts/Main.sourdo");
    if(result == sourdo::Result::SUCCESS)
    {
        test.get_value("apple_count");
        if(test.value_is_number(-1))
        {
            std::cout << "There are " << test.value_to_number(-1) << " apples." << std::endl;
        }
    }
    else
    {
        std::string error = test.value_to_string(-1);
        std::cout << error << std::endl;
    }
    return 0;
}

#include <string>
#include <iostream>

#include <sstream>

#include <chrono>

#include <SourDo/SourDo.hpp>
#include <SourDo/StandardLibs/Basic.hpp>
#include <Sourdo/StandardLibs/Math.hpp>

int main()
{
    sourdo::Data test;

    sourdo::load_lib_basic(test);
    sourdo::load_lib_math(test);

    test.create_value("strlen");
    test.push_cppfunction([](sourdo::Data& data) -> bool
    {
        if(data.get_size() != 1)
        {
            std::stringstream ss;
            ss << "Expected 1 argument but got " << data.get_size();
            data.error(ss.str());
        }
        if(data.get_value_type(1) != sourdo::ValueType::STRING)
        {
            std::stringstream ss;
            ss << "Argument #1: Expected a string";
            data.error(ss.str());
        }
        data.push_number(data.value_to_string(1).size());
        return true;
    });
    test.set_value("strlen");

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

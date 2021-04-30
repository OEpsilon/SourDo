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
    sourdo::load_lib_math(test);;

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

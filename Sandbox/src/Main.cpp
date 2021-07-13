#include <string>
#include <iostream>

#include <sstream>
#include <vector>
#include <chrono>


#include <SourDo/SourDo.hpp>
#include <SourDo/Errors.hpp>
#include <SourDo/StandardLibs/Basic.hpp>
#include <Sourdo/StandardLibs/Math.hpp>


void check_result(sourdo::Data& data, sourdo::Result res)
{
    if(res != sourdo::Result::SUCCESS)
    {
        std::string error = data.value_to_string(-1);
        std::cout << error << std::endl;
    }
}

int main()
{
    sourdo::Data test;

    sourdo::load_lib_basic(test);
    sourdo::load_lib_math(test);

    auto begin = std::chrono::high_resolution_clock::now();

    sourdo::Result res = test.do_file("Scripts/Main.sourdo");

    auto end = std::chrono::high_resolution_clock::now();
    check_result(test, res);

    auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - begin);
    std::cout << "SourDo script took " << duration.count() << " milliseconds" << std::endl;
    
    return 0;
}

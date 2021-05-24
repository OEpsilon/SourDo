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

    /*
    auto callback_signal = (std::vector<sourdo::GCRef>*)(test.create_cpp_object(sizeof(std::vector<sourdo::GCRef>)));
    new(callback_signal) std::vector<sourdo::GCRef>();
    
    test.create_object();
    test.push_string("__name");
    test.push_string("CallbackSignal");
    test.object_set(-3);
    
    test.push_string("listen");
    test.push_cppfunction([](sourdo::Data& data) -> bool
    {
        sourdo::check_arg_count(data, 2);
        auto self = (std::vector<sourdo::GCRef>*)(data.check_cpp_object(1, "CallbackSignal"));
        sourdo::check_is_function(data, 2);
        sourdo::check_sourdo_func_param_count(data, 2, 1);

        sourdo::GCRef func_ref = data.create_ref(2);
        if(data.is_ref_valid(func_ref))
        {
            self->emplace_back(func_ref);
        }
        return false;
    });
    test.object_set(-3);
    test.set_cpp_object_prototype(-2);
    test.create_value("callback_signal");
    check_result(test, test.set_value("callback_signal", true));
    */

    auto begin = std::chrono::high_resolution_clock::now();

    //check_result(test, test.do_file("Examples/Scripts/Main.sourdo"));
    sourdo::Result res = test.do_file("Examples/Scripts/BytecodeTest.sourdo");

    auto end = std::chrono::high_resolution_clock::now();
    check_result(test, res);

    auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - begin);
    std::cout << "SourDo script took " << duration.count() << " milliseconds" << std::endl;
    /*
    int call_count = 1;
    for(auto& callback_func : *callback_signal)
    {
        test.push_ref(callback_func);
        test.push_number(call_count);
        check_result(test, test.call_function(1, true));
    }
    call_count++;

    for(auto& callback_func : *callback_signal)
    {
        test.push_ref(callback_func);
        test.push_number(call_count);
        check_result(test, test.call_function(1, true));
    }
    call_count++;

    callback_signal->~vector();

    */
    
    return 0;
}

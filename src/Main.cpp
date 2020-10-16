#include "SourDo/SourDo.h"

int main()
{
    // TODO: Allow input files to be from command line args
    // For now, the file is hard-coded into the executable.
    std::string sandbox = R"(
let<int> myVariable <= 20;
print(myVariable - 5.5);
/)";
    srdo::compile({"Sandbox", sandbox});
}
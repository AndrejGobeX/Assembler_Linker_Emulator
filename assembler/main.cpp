#include <iostream>
#include "driver.hpp"
#include "compiler.hpp"
#include "symbol_table.hpp"
#include "line.hpp"
#include "arg.hpp"

int main (int argc, char *argv[])
{
    int res = 0;
    driver drv;
    for (int i = 1; i < argc; ++i)
        if (argv[i] == std::string ("-p"))
            drv.trace_parsing = true;
        else if (argv[i] == std::string ("-s"))
            drv.trace_scanning = true;
        else if (!drv.parse (argv[i]))
            //std::cout << drv.result << '\n';
            res = 0;
        else
        {
            res = 1;
            break;
        }

    if(!res)
    {
        compiler::get_compiler()->get_symbol_table().print();
        compiler::get_compiler()->print();
    }
    compiler::del_compiler();
    return res;
}

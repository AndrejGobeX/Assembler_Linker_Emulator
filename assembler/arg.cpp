#include "arg.hpp"

arg arg::make_arg(int _arg, unsigned char type)
{
    arg temp(_arg);
    temp.set_type(type);
    return temp;
}

arg arg::make_arg(std::string _arg, unsigned char type)
{
    arg temp(_arg);
    temp.set_type(type);
    return temp;
}
#include"line.hpp"
#include"arg.hpp"

line::line(std::string _name)
    : name(_name)
{}

void line::add_arg(arg _arg)
{
    args.push_back(_arg);
}
#include"compiler.hpp"
#include"arg.hpp"

compiler * compiler::ptr;

compiler::compiler()
{
    lc = 0;
    section = "default";
    compiler::ptr = nullptr;
}

compiler * compiler::get_compiler()
{
    if(compiler::ptr == nullptr)
    {
        compiler::ptr = new compiler();
    }
    return compiler::ptr;
}

void compiler::add_line(line _line)
{
    if(_line.get_jmp())
    {
        unsigned char type = _line.get_args()[0].get_type();
        if(type == 1 || type == 2)
            lc += 3;
        else
            lc += 5;
    }
    else if(_line.get_name() == "ldr" || _line.get_name() == "str")
    {
        unsigned char type = _line.get_args()[0].get_type();
        if(type == 1 || type == 2)
            lc += 3;
        else
            lc += 5;
    }
    else if(_line.get_name() == ".word")
    {
        lc += 2 * ( _line.get_args().size() );
    }
    else if(_line.get_name() == ".skip")
    {
        lc += ( _line.get_args()[0].get_argl() );
    }
    else if(_line.get_args().size() == 0)
    {
        lc += 1;
    }
    else if(_line.get_args().size() == 1)
    {
        lc += 2;
    }
    else if(_line.get_args().size() == 2)
    {
        lc += 2;
    }
    parsed_lines.push_back(_line);
}
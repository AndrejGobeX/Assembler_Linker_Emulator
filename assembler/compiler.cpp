#include"compiler.hpp"
#include"arg.hpp"

const int EXCEPTION_SYMBOL_NOT_DEFINED = 2;

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

void compiler::pass_two()
{
    section = "default";
    lc = 0;
    for(line & l : parsed_lines)
    {
        interpret(l);
    }
}

void compiler::interpret(line & l)
{
    std::string name = l.get_name();
    if(name == ".section")
    {
        lc = 0;
        section = l.get_args()[0].get_argi();
        std::vector<unsigned char> mini;
        std::vector<relocation> maxi;
        bytes.push_back({section, mini});
        relocations.push_back({section, maxi});
    }
    else if(name == ".word")
    {
        for(arg & _arg : l.get_args())
        {
            if(_arg.get_literal())
            {
                add_word(_arg.get_argl());
            }
            else
            {
                add_byte_symbol(_arg.get_argi());
            }
            lc += 2;
        }
    }
    else if(name == ".skip")
    {
        int n = l.get_args()[0].get_argl();
        lc += n;
        while(n--)
        {
            add_byte(0);
        }
    }
    //STAO
}

void compiler::add_byte_symbol(std::string name)
{
    if(!sym_tab.find(name))
        throw EXCEPTION_SYMBOL_NOT_DEFINED;
                
    entry e = sym_tab[name];
    if(e.abs)
    {
        add_word(e.val);
    }
    else if(e.glob)
    {
        add_relocation(name, false, lc);
    }
    else
    {
        add_word(e.val);
        add_relocation(e.section, false, lc);
    }

}

void compiler::add_relocation(std::string name, bool pc_rel, int location)
{
    relocation r;
    r.name = name;
    r.pc_rel = pc_rel;
    r.location = location;
    relocations[relocations.size() - 1].second.push_back(r);
}

void compiler::print()
{
    for(line l : parsed_lines)
    {
        std::cout<<l.get_name()<<"\n";
    }
}
#include"compiler.hpp"
#include"arg.hpp"
#include<iomanip>
#include<fstream>

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
    if(_line.get_name() == ".section")
    {
        lc += 0;
    }
    else if(_line.get_name() == ".word")
    {
        lc += 2 * ( _line.get_args().size() );
    }
    else if(_line.get_name() == ".skip")
    {
        lc += ( _line.get_args()[0].get_argl() );
    }
    else if(_line.get_jmp())
    {
        short type = _line.get_args()[0].get_type();
        if(type == 1 || type == 2)
            lc += 3;
        else
            lc += 5;
    }
    else if(_line.get_name() == "ldr" || _line.get_name() == "str")
    {
        short type = _line.get_args()[1].get_type();
        if(type == 1 || type == 2)
            lc += 3;
        else
            lc += 5;
    }
    else if(_line.get_name() == "push" || _line.get_name() == "pop")
    {
        lc += 3;
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
    check_symbols();
    section = "default";
    lc = 0;
    for(line & l : parsed_lines)
    {
        interpret(l);
    }
}

void compiler::check_symbols()
{
    for(std::pair<std::string, entry> symbol : sym_tab.get_entries())
    {
        if(symbol.second.abs == false && symbol.second.val == -2)
        {
            throw EXCEPTION_SYMBOL_NOT_DEFINED;
        }
    }
}

void compiler::add_arg(arg & _arg, short off)
{
    if(_arg.get_literal())
    {
        if(_arg.has_offset())
        {
            add_arg(_arg.get_offset(), _arg.get_argl());
        }
        else
        {
            add_word(_arg.get_argl());
        }
    }
    else
    {
        add_word_symbol(_arg.get_argi(), off);
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
            add_arg(_arg);
            lc += 2;
        }
    }
    else if(name == ".skip")
    {
        short n = l.get_args()[0].get_argl();
        lc += n;
        while(n--)
        {
            add_byte(0);
        }
    }
    else
    {
        if(name == "halt")
        {
            add_byte(0);
            lc += 1;
        }
        else if(name == "int")
        {
            add_byte(0x10);
            unsigned char reg = parse_reg(l.get_args()[0].get_argi());
            add_byte(reg, 0xF);
            lc += 2;
        }
        else if(name == "push")
        {
            add_byte(0xB0);
            unsigned char reg = parse_reg(l.get_args()[0].get_argi());
            add_byte(reg, 0x6);
            add_byte(0x1, 0x2);
            lc += 3;
        }
        else if(name == "pop")
        {
            add_byte(0xA0);
            unsigned char reg = parse_reg(l.get_args()[0].get_argi());
            add_byte(reg, 0x6);
            add_byte(0x4, 0x2);
            lc += 3;
        }
        else if(name == "iret")
        {
            add_byte(0x20);
            lc += 1;
        }
        else if(name == "ret")
        {
            add_byte(0x40);
            lc += 1;
        }
        else if(name == "call")
        {
            add_byte(0x30);
            lc += 1;
            short type = l.get_args()[0].get_type();
            if(type == 0 || type == 4)
            {
                add_byte(0xF, 0x0);
                add_byte(0x0, type);
                lc += 2;
                add_arg(l.get_args()[0]);
                lc += 2;
            }
            else if(type == 1 || type == 2 || type == 3 || type == 5)
            {
                unsigned char reg = parse_reg(l.get_args()[0].get_argi());
                add_byte(0xF, reg);
                add_byte(0x0, type);
                lc += 2;
                if(type == 3 || type == 5)
                {
                    add_arg(l.get_args()[0].get_offset());
                    lc += 2;
                }
            }
        }
        else if(name[0] == 'j')
        {
            unsigned char jtype = 0;
            if(name == "jeq")jtype = 1;
            else if(name == "jne")jtype = 2;
            else if(name == "jgt")jtype = 3;

            add_byte(0x5, jtype);
            lc += 1;
            short type = l.get_args()[0].get_type();
            if(type == 0 || type == 4)
            {
                add_byte(0xF, 0x0);
                add_byte(0x0, type);
                lc += 2;
                add_arg(l.get_args()[0]);
                lc += 2;
            }
            else if(type == 1 || type == 2 || type == 3 || type == 5)
            {
                unsigned char reg = parse_reg(l.get_args()[0].get_argi());
                add_byte(0xF, reg);
                add_byte(0x0, type);
                lc += 2;
                if(type == 3 || type == 5)
                {
                    add_arg(l.get_args()[0].get_offset());
                    lc += 2;
                }
            }
        }
        else if(name == "ldr")
        {
            add_byte(0xA0);
            lc += 1;
            short type = l.get_args()[1].get_type();
            if(type == 0 || type == 4)
            {
                unsigned char reg = parse_reg(l.get_args()[0].get_argi());
                add_byte(reg, 0x0);
                add_byte(0x0, type);
                lc += 2;
                add_arg(l.get_args()[1]);
                lc += 2;
            }
            else if(type == 1 || type == 2 || type == 3 || type == 5)
            {
                unsigned char reg = parse_reg(l.get_args()[0].get_argi());
                unsigned char reg1 = parse_reg(l.get_args()[1].get_argi());
                add_byte(reg, reg1);
                add_byte(0x0, type);
                lc += 2;
                if(type == 3 || type == 5)
                {
                    add_arg(l.get_args()[1].get_offset());
                    lc += 2;
                }
            }
        }
        else if(name == "str")
        {
            add_byte(0xB0);
            lc += 1;
            short type = l.get_args()[1].get_type();
            if(type == 0 || type == 4)
            {
                unsigned char reg = parse_reg(l.get_args()[0].get_argi());
                add_byte(reg, 0x0);
                add_byte(0x0, type);
                lc += 2;
                add_arg(l.get_args()[1]);
                lc += 2;
            }
            else if(type == 1 || type == 2 || type == 3 || type == 5)
            {
                unsigned char reg = parse_reg(l.get_args()[0].get_argi());
                unsigned char reg1 = parse_reg(l.get_args()[1].get_argi());
                add_byte(reg, reg1);
                add_byte(0x0, type);
                lc += 2;
                if(type == 3 || type == 5)
                {
                    add_arg(l.get_args()[1].get_offset());
                    lc += 2;
                }
            }
        }
        else
        {
            if(name == "xchg")
            {
                add_byte(0x60);
            }
            else if(name == "add")
            {
                add_byte(0x70);
            }
            else if(name == "sub")
            {
                add_byte(0x71);
            }
            else if(name == "mul")
            {
                add_byte(0x72);
            }
            else if(name == "div")
            {
                add_byte(0x73);
            }
            else if(name == "cmp")
            {
                add_byte(0x74);
            }
            else if(name == "not")
            {
                add_byte(0x80);
                lc += 1;
                unsigned char reg = parse_reg(l.get_args()[0].get_argi());
                add_byte(reg, 0x0);
                lc += 1;
                return;
            }
            else if(name == "and")
            {
                add_byte(0x81);
            }
            else if(name == "or")
            {
                add_byte(0x82);
            }
            else if(name == "xor")
            {
                add_byte(0x83);
            }
            else if(name == "test")
            {
                add_byte(0x84);
            }
            else if(name == "shl")
            {
                add_byte(0x90);
            }
            else if(name == "shr")
            {
                add_byte(0x91);
            }
            else
                throw EXCEPTION_SYMBOL_NOT_DEFINED;
            lc += 1;
            unsigned char reg = parse_reg(l.get_args()[0].get_argi());
            unsigned char reg1 = parse_reg(l.get_args()[1].get_argi());
            add_byte(reg, reg1);
            lc += 1;
        }
    }
}

unsigned char compiler::parse_reg(std::string name)
{
    if(name[0] == 'r')
    {
        unsigned char n = name[1] - '0';
        return n;
    }
    else if(name == "sp")
    {
        return 6;
    }
    else if(name == "pc")
    {
        return 7;
    }
    else if(name == "psw")
    {
        return 8;
    }
    else
    {
        throw EXCEPTION_SYMBOL_NOT_DEFINED;
    }
}

void compiler::add_word_symbol(std::string name, short off)
{
    if(!sym_tab.find(name))
        throw EXCEPTION_SYMBOL_NOT_DEFINED;

    entry e = sym_tab[name];
    if(off && section == e.section)
    {
        add_word(e.val+off-lc);
    }
    else if(e.abs)
    {
        add_word(e.val+off);
    }
    else if(e.glob)
    {
        add_word(off);
        add_relocation(name, off, lc);
    }
    else
    {
        add_word(e.val+off);
        add_relocation(e.section, off, lc);
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

void compiler::generate(std::string file_name)
{
    std::ofstream out(file_name, std::ofstream::trunc);
    std::ofstream out_hr(file_name + ".hr", std::ofstream::trunc);
    out_hr<<"=============================\nSYMBOL TABLE\n=============================\n";
    out_hr<<std::left<<std::setw(10)<<"Symbol";
    out_hr<<std::left<<std::setw(10)<<"Value";
    out_hr<<std::left<<std::setw(10)<<"Section"<<"\n";
    for(std::pair<std::string, entry> e : sym_tab.get_entries())
    {
        if(!e.second.glob && e.second.section != e.first)
            continue;
        out<<std::hex;
        out<<e.first<<" ";
        out<<e.second.val<<" ";
        out<<(e.second.section == "" ? "UND" : e.second.section)<<"\n";

        out_hr<<std::hex;
        out_hr<<std::left<<std::setw(10)<<e.first<<"";
        out_hr<<std::left<<std::setw(10)<<e.second.val<<"";
        out_hr<<std::left<<std::setw(10)<<(e.second.section == "" ? "UND" : e.second.section)<<"\n";
    }
    out<<"\n";
    out_hr<<"\n=============================\nRELOCATIONS\n=============================\n";
    out_hr<<std::left<<std::setw(10)<<"Location";
    out_hr<<std::left<<std::setw(10)<<"Relative";
    out_hr<<std::left<<std::setw(10)<<"Symbol"<<"\n";
    for(std::pair<std::string, std::vector<relocation>> & rel_list : relocations)
    {
        out<<rel_list.first<<" "<<rel_list.second.size()<<"\n";
        out_hr<<rel_list.first<<"\n";
        for(relocation & rel : rel_list.second)
        {
            out<<rel.location<<" "<<rel.pc_rel<<" "<<rel.little<<" "<<rel.name<<"\n";
            out_hr<<std::left<<std::setw(10)<<rel.location;
            out_hr<<std::left<<std::setw(10)<<rel.pc_rel;
            out_hr<<std::left<<std::setw(10)<<rel.name<<"\n";
        }
    }
    out<<"\n";
    out_hr<<"=============================\nBYTES\n=============================\n";
    for(std::pair<std::string, std::vector<unsigned char>> & byte_list : bytes)
    {
        out<<byte_list.first<<"\n";
        out_hr<<byte_list.first<<"\n";
        for(unsigned char & b : byte_list.second)
        {
            out<<(unsigned int)b<<" ";
            out_hr<<(unsigned int)b<<" ";
        }
        out<<"\n";
        out_hr<<"\n";
    }
    out_hr.close();
    out.close();
}
#include"symbol_table.hpp"
#include<iostream>

const int EXCEPTION_DUPLICATE_SYMBOL = 1;

entry symbol_table::make_entry(unsigned int _index, std::string _section,
        bool _abs, bool _glob, short _val)
{
    entry e;
    e.i = _index;
    e.section = _section;
    e.abs = _abs;
    e.glob = _glob;
    e.val = _val;
    return e;
}

void symbol_table::add(std::string _name, entry e)
{
    if(entries.find(_name) != entries.end())
    {
        if(entries[_name].val > -2)
        {
            std::cerr<<"error: duplicate symbol "<<_name<<"\n";
            throw EXCEPTION_DUPLICATE_SYMBOL;
        }
        else
        {
            entries[_name].abs = e.abs;
            entries[_name].section = e.section;
            entries[_name].val = e.val;
        }
    }
    else
    {
        e.i = index++;
        entries[_name] = e;
    }
}

bool symbol_table::find(std::string _name)
{
    if(entries.find(_name) == entries.end())
    {
        return false;
    }
    return true;
}

void symbol_table::print()
{
    std::cout<<std::hex<<"SYMBOL TABLE\n";
    std::cout<<"i\tname\tval\tg/l\tA\tsection\n";
    std::cout<<"----------------------------------------------------------------------------\n";
    for(std::pair<std::string, entry> e : entries)
    {
        std::cout<<e.second.i<<"\t";
        std::cout<<e.first<<"\t";
        std::cout<<e.second.val<<"\t";
        std::cout<<(e.second.glob ? "g" : "l")<<"\t";
        std::cout<<e.second.abs<<"\t";
        std::cout<<(e.second.section == "" ? "UND" : e.second.section)<<"\n";
    }
}
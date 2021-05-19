#ifndef COMPILER_HPP
#define COMPILER_HPP

#include<string>
#include<vector>
#include"line.hpp"
#include"symbol_table.hpp"

class compiler
{
public:
    compiler();
    static compiler * get_compiler();
    static void del_compiler()
    {
        if(compiler::ptr!=nullptr)delete compiler::ptr;
        compiler::ptr = nullptr;
    }
    void add_line(line _line);
    symbol_table & get_symbol_table(){return sym_tab;}
    std::string get_section(){return section;}
    void set_section(std::string _section){section = _section; lc = 0;}
    int get_lc(){return lc;}
private:
    static compiler * ptr;
    std::string section;
    int lc;
    std::vector<line> parsed_lines;
    symbol_table sym_tab;
};


#endif // ! COMPILER_HPP
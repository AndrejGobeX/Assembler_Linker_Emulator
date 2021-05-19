#ifndef COMPILER_HPP
#define COMPILER_HPP

#include<string>
#include<vector>
#include"line.hpp"
#include"symbol_table.hpp"

struct relocation
{
    std::string name;
    bool pc_rel;
    int location;
};

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
    void print();
    symbol_table & get_symbol_table(){return sym_tab;}
    std::string get_section(){return section;}
    void set_section(std::string _section){section = _section; lc = 0;}
    int get_lc(){return lc;}
    void pass_two();
    void interpret(line & l);
    void add_byte(unsigned char b){ bytes[bytes.size() - 1].second.push_back(b); }
    void add_word(int word)
    {
        add_byte(word);
        word >>= 8;
        add_byte(word);
    }
    void add_byte_symbol(std::string name);
    void add_relocation(std::string name, bool pc_rel, int location);
private:
    static compiler * ptr;
    std::string section;
    int lc;
    std::vector<line> parsed_lines;
    std::vector<std::pair<std::string, std::vector<unsigned char>>> bytes;
    std::vector<std::pair<std::string, std::vector<relocation>>> relocations;
    symbol_table sym_tab;
};


#endif // ! COMPILER_HPP
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
    bool little = false;
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
    void generate(std::string file_name);
    symbol_table & get_symbol_table(){return sym_tab;}
    std::string get_section(){return section;}
    void set_section(std::string _section){section = _section; lc = 0;}
    int get_lc(){return lc;}
    void pass_two();
    void interpret(line & l);
    void add_byte(unsigned char b){ bytes[bytes.size() - 1].second.push_back(b); }
    void add_word(short word)
    {
        add_byte(word);
        add_byte(word>>8);
    }
    void add_word_symbol(std::string name, short off=0);
    void add_arg(arg & _arg, short off=0);
    unsigned char parse_reg(std::string name);
    void add_byte(unsigned char a, unsigned char b)
    {
        a<<=4;
        a+=(b & 0xF);
        add_byte(a);
    };
    void add_relocation(std::string name, bool pc_rel, int location);
    void check_symbols();
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
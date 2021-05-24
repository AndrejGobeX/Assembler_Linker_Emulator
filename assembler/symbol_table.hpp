#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include<string>
#include<unordered_map>

struct entry
{
    unsigned int i;
    std::string section;
    bool abs;
    bool glob; // l - false, g - true
    short val; // -2 - global, -1 - extern
};

class symbol_table
{
    friend class compiler;
public:
    symbol_table(){}
    static entry make_entry(unsigned int _index, std::string _section,
        bool _abs, bool _glob, short _val);
    int size(){return entries.size();}
    entry & operator[](std::string _name){return entries[_name];}
    bool find(std::string _name);
    void add(std::string _name, entry e);
    void print();
    std::unordered_map<std::string, entry> & get_entries(){return entries;}
private:
    std::unordered_map<std::string, entry> entries;
    unsigned int index = 1;
};

#endif
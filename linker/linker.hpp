#ifndef LINKER_HPP
#define LINKER_HPP

#include<string>
#include<vector>
#include<unordered_map>
#include<iostream>

class linker
{
public:

    struct symbol
    {
        std::string name;
        short val;
        std::string section_abs;
        bool is_section()
        {
            return name == section_abs;
        }
    };

    struct relocation
    {
        short location;
        bool pc_rel, big_endian;
        std::string symbol;
        short addend = 0;
    };

    static linker * get_linker(){ if(instance == nullptr)instance = new linker(); return instance; }
    static void delete_linker(){ if(instance != nullptr){delete instance; instance = nullptr;} }

    void link_file(std::string);
    void add_symbol(linker::symbol);
    void add_relocation(linker::relocation, std::string);
    unsigned short get_offset(std::string section)
    {
        if(section == "UND" || section == "ABS")
            return 0;
        unsigned short offset = 0;
        if(bytes.find(section) != bytes.end())
            offset = bytes[section].size();
        return offset;
    }
    void fix_relocations();
    bool symbol_defined(std::string name)
    {
        if(symbols.find(name) != symbols.end())
            if(symbols[name].section_abs != "UND")
                return true;
        return false;
    }

private:
    static linker * instance;

    std::unordered_map<std::string, std::vector<unsigned char>> bytes;
    std::unordered_map<std::string, std::vector<relocation>> relocations;
    std::unordered_map<std::string, symbol> symbols;
};

#endif
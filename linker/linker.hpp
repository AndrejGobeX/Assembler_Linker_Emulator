#ifndef LINKER_HPP
#define LINKER_HPP

#include<string>
#include<vector>
#include<unordered_map>

class linker
{
public:

    struct symbol
    {
        std::string name;
        short val;
        std::string section_abs;
    };

    struct relocation
    {
        short location;
        bool pc_rel, big_endian;
        std::string symbol;
    };

    static linker * get_linker(){ if(instance == nullptr)instance = new linker(); return instance; }
    static void delete_linker(){ if(instance != nullptr){delete instance; instance = nullptr;} }

    void link_file(std::string);
    void add_symbol(linker::symbol);
    void add_relocation(linker::relocation, std::string);

private:
    static linker * instance;

    std::unordered_map<std::string, std::vector<unsigned char>> bytes;
    std::unordered_map<std::string, std::vector<relocation>> relocations;
    std::unordered_map<std::string, symbol> symbols;
};

#endif
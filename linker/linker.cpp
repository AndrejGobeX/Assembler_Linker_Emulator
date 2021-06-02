#include"linker.hpp"
#include<fstream>
#include<sstream>
#include"linker_exceptions.hpp"

linker * linker::instance = nullptr;

void linker::link_file(std::string file_name)
{
    std::ifstream file(file_name);

    std::string line;

    //SYMBOLS
    while(std::getline(file, line))
    {
        if(line == "")
            break;
        
        std::stringstream ss(line);
        linker::symbol sym;
        ss >> sym.name >> sym.val >> sym.section_abs >> sym.section_abs; //Deliberate
        add_symbol(sym);
    }

    //RELOCATIONS
    while(std::getline(file, line))
    {
        if(line == "")
            break;
        
        std::stringstream ss(line);
        std::string section;
        short size;
        ss >> section >> size;
        while(size--)
        {
            std::getline(file, line);
            std::stringstream ss(line);
            linker::relocation rel;
            ss >> rel.location >> rel.pc_rel >> rel.big_endian >> rel.symbol;
            add_relocation(rel, section);
        }
    }

    //BYTES
    while(std::getline(file, line))
    {
        if(line == "")
            break;
        
        std::stringstream ss(line);
        std::string section;
        ss >> section;
        std::getline(file, line);
        std::stringstream file_bytes(line);
        unsigned short byte;
        while(file_bytes >> std::hex >> byte)
        {
            bytes[section].push_back(byte);
        }
    }

    file.close();
}

void linker::fix_relocations()
{
    for(std::pair<std::string, std::vector<relocation>> rels : relocations)
    {
        std::string section = rels.first;
        for(relocation rel : rels.second)
        {
            if(!symbol_defined(rel.symbol))
            {
                throw linker_exception("Undefined symbol " + rel.symbol);
            }
            //TODO:
        }
    }
}

void linker::add_symbol(linker::symbol sym)
{
    if(symbol_defined(sym.name))
    {
        if((sym.section_abs == "UND") || (sym.is_section()))return;
        throw linker_exception("Duplicate symbol " + sym.name);
    }
    sym.val += get_offset(sym.section_abs);
    symbols[sym.name] = sym;
}

void linker::add_relocation(linker::relocation rel, std::string section)
{
    rel.location += get_offset(section);
    if(symbols[rel.symbol].is_section())
    {
        rel.addend = get_offset(symbols[rel.symbol].section_abs);
    }
    relocations[section].push_back(rel);
}
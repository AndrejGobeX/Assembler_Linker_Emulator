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
        short offset = 0;
        if(relocations.find(section) != relocations.end())
            offset = relocations[section].size();
        while(size--)
        {
            std::getline(file, line);
            std::stringstream ss(line);
            linker::relocation rel;
            ss >> rel.location >> rel.pc_rel >> rel.big_endian >> rel.symbol;
            rel.location += offset;
            add_relocation(rel, section);
        }
    }


    file.close();
}

void linker::add_symbol(linker::symbol sym)
{
    if(symbols.find(sym.name) != symbols.end())
    {
        if((sym.section_abs == "UND") || (sym.name == sym.section_abs))return;
        if(symbols[sym.name].section_abs != "UND")
        {
            throw linker_exception("Duplicate symbol " + sym.name);
        }
    }
    symbols[sym.name] = sym;
}

void linker::add_relocation(linker::relocation rel, std::string section)
{
    relocations[section].push_back(rel);
}
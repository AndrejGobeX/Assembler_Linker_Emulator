#include<iostream>
#include<string>
#include<regex>
#include<unordered_map>
#include"linker.hpp"

int main (int argc, char *argv[])
{
    bool hex = false, link = false;
    std::string file_name = "output.hex";
    std::regex section_pattern ("-place=([a-zA-Z][a-zA-Z_0-9]*)@(0x[0-9A-Fa-f]+)");
    std::smatch sm;
    std::string str;
    std::unordered_map<std::string, short> sections;
    std::stringstream ss;
    std::vector<std::string> files;

    for(int i = 1; i < argc; ++i)
    {
        if(argv[i] == "-o")
            file_name = argv[++i];
        else if(argv[i] == std::string("-hex"))
            hex = true;
        else if(argv[i] == std::string("-linkable"))
            link = true;
        else if(argv[i][0] == '-')
        {
            str = argv[i];
            if(std::regex_match(str, sm, section_pattern))
            {
                ss<<std::hex<<sm[2];
                short start;
                ss>>start;
                sections[sm[1]] = start;
                ss.clear();
            }
            else
            {
                std::cout<<"Invalid argument "<<i<<".\n";
                return 1;
            }
        }
        else
        {
            files.push_back(argv[i]);
        }
    }

    if(!(hex ^ link))
    {
        std::cout<<"Expected exactly one of the following arguments: -hex, -linkable\n";
        return 1;
    }

    if(files.size() == 0)
        return 0;

    linker * _linker = linker::get_linker();

    for(std::string file : files)
    {
        _linker->link_file(file);
    }

    
    _linker->fix_relocations();

    linker::delete_linker();

    return 0;
}
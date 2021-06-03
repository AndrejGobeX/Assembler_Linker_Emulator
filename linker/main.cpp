#include<iostream>
#include<fstream>
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
    std::vector<std::pair<unsigned short, std::string>> sections;
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
                sections.push_back({start, sm[1]});
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

    for(std::string filename : files)
    {
        std::ifstream file(filename);
        _linker->link_file(file);
        file.close();
    }

    std::ofstream out(file_name, std::ofstream::trunc);
    if(link)
    {
        _linker->generate_linkable(out);
    }
    else
    {
        _linker->check_symbols();
        _linker->set_section_starts(sections);
        _linker->fix_sections();
        _linker->fix_relocations();
        _linker->generate_executable(out);
    }
    out.close();

    linker::delete_linker();

    return 0;
}